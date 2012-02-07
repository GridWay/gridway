# ------------------------------------------------------------------------------
# Copyright 2002-2012, GridWay Project Leads (GridWay.org)          
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may
# not use this file except in compliance with the License. You may obtain
# a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ------------------------------------------------------------------------------


import sys
import os
import shutil
import logging
import thread
import time
from arclib import *

logger=None
log_handler=None
job_list={}

# NorduGrid Stuff	

nordugrid_states = {
	'ACCEPTING':	'PENDING',
	'ACCEPTED':		'PENDING',
	'PREPARING':	'PENDING',
	'PREPARED':		'PENDING',
	'SUBMITING':	'PENDING',
	'INLRMS:R':		'ACTIVE',
	'INLRMS:Q':		'ACTIVE',
	'KILLING':		'ACTIVE',
	'EXECUTED FINISHING': 'ACTIVE',
	'FINISHED':		'DONE',
	'FAILED':		'FAILED',
	'KILLED':		'DONE',
	'DELETED':		'DONE'
}

def nordugrid_to_gw_states(state):
	if nordugrid_states.has_key(state):
		return nordugrid_states[state]
	else:
		return 'UNKNOWN'

def get_target_list(rsl, contact, name="fork"):
	# get information about the cluster (with queues)
	ldap_url=URL(contact)
	queues_list=GetQueueInfo(ldap_url)

	# search for the queue
	queue=False
	for i in range(len(queues_list)):
		if queues_list[i].name == name:
			queue=queues_list[i]

	if not queue:
		return None

	# convert the queue into a target and insert it into the target_list	
	target=Target(queue, rsl)
	tl=target_list()
	tl.push_back(target)

	return tl
	
def ldap_contact(host):
	return "ldap://%s:2135/o=grid/mds-vo-name=local" % host

def submit_rsl(rsl, contact, queue_name):
	target_list=get_target_list(rsl, contact, queue_name)
	if target_list==None:
		return None
	job_id=SubmitJob(rsl, target_list)
	return job_id

# Job Class
class GwJob(object):
	def __init__(self, jid):
		self.jid=jid
		self.submited=False
		self.properties_lock=thread.allocate_lock()
		self.downloading=False
		
	def lock(self):
		self.properties_lock.acquire()
		
	def unlock(self):
		self.properties_lock.release()
		
	def submit(self, contact, rsl_file):
		(host, queue) = contact.split("/")
		self.lock()
		self.host=host
		self.queue=queue
		jid=self.jid
		self.unlock()
		
		file_handler=open(rsl_file, 'r')
		rsl_text=file_handler.read()
		file_handler.close()
		rsl=Xrsl(rsl_text)
		
		logger.info("RSL --8<------")
		logger.info(rsl_text)
		logger.info("------>8-- RSL")		
		
		try:
			job_id=submit_rsl(rsl, ldap_contact(host), queue)
			status="PENDING"
			print "SUBMIT %s SUCCESS %s" % (jid, job_id)
			logger.info("Job submited: job_id=%s" % job_id)
			
			self.lock()
			self.job_id=job_id
			self.status=status
			self.unlock()
			
			thread.start_new_thread(self.status_updater, ())
		except Exception, e:
			self.lock()
			self.status="FAILED"
			self.unlock()
			
			print "SUBMIT %s FAILED Error sending job" % jid
			print e
		
		self.lock()
		self.submited=True
		self.unlock()
		
		sys.stdout.flush()
		
	def poll(self):
		if not self.submited:
			print "POLL %s FAILURE Job not yet submited" % self.jid
		else:
			print "POLL %s SUCCESS %s" % (self.jid, self.status)
			
		sys.stdout.flush()
		
	def standalone_poll(self):
		self.lock()
		submited=self.submited
		downloading=self.downloading
		self.unlock()
		
		if not submited:
			print "POLL %s FAILURE Job not yet submited" % self.jid
		elif not downloading:
			self.lock()
			job_id=self.job_id
			self.unlock()
			
			job=GetJobInfo(job_id)
			
			self.lock()
			self.status=nordugrid_to_gw_states(job.status)
			print "POLL %s SUCCESS %s" % (self.jid, self.status)
			self.unlock()
		else:
			print "POLL %s SUCCESS %s" % (self.jid, "ACTIVE")
			
		sys.stdout.flush()
		
	def status_updater(self):
		self.lock()
		submited=self.submited
		job_id=self.job_id
		jid=self.jid
		self.unlock()
		
		if self.submited==True:
			self.lock()
			self.monitor=True
			monitor=self.monitor
			self.unlock()
			
			while monitor:
				
				job=GetJobInfo(job_id)
				new_status=nordugrid_to_gw_states(job.status)
				
				self.lock()
				old_status=self.status
				self.unlock()
				
				if new_status=="DONE" or new_status=="FAILED":
					self.lock()
					self.monitor=False
					self.unlock()
					self.get_output()
					GW_LOCATION=os.environ['GW_LOCATION']
					logger.info(job.exitcode.__class__)
					self.create_wrapper_files(
							job.exitcode, 
							"%s/var/%s/nordugrid" % (GW_LOCATION, jid))
				
				if not new_status==old_status:
					self.lock()
					self.status=new_status
					self.unlock()
					print "CALLBACK %s SUCCESS %s" % (jid, new_status)
					sys.stdout.flush()					
					
				# Change here status polling time
				time.sleep(10)
				
				self.lock()
				monitor=self.monitor
				self.unlock()
				
	def cancel(self):
		self.lock()
		submited=self.submited
		jid=self.jid
		self.unlock()
		if submited==True:
			# Stop it right now or let the monitor die by 
			# itself (after DONE status)?
			self.lock()
			self.monitor=False
			job_id=self.job_id
			self.unlock()
			CancelJob(job_id)
			print "CANCEL %s SUCCESS -" % jid
		else:
			print "CANCEL %s FAILURE Job not submited" % jid
		
		sys.stdout.flush()
		
	def get_output(self):
		self.lock()
		jid=self.jid
		job_id=self.job_id
		self.downloading=True
		self.unlock()
		GW_LOCATION=os.environ['GW_LOCATION']
		destination="%s/var/%s/nordugrid" % (GW_LOCATION, jid)
		logger.info("Downloading dir %s into %s" % (job_id, destination))
		os.mkdir(destination)
		j=JobFTPControl()
		j.DownloadDirectory(URL(job_id), destination)
		
		# copy wrapper stdout & stderr
		# execution -> out
		#shutil.copy(destination+"/stdout.execution", destination+"/stdout.wrapper")
		#shutil.copy(destination+"/stderr.execution", destination+"/stderr.wrapper")
		
		
		
		self.lock()
		self.downloading=False
		self.unlock()
		
	def create_wrapper_files(self, status, directory):
		out_file_name="%s/stdout.wrapper" % directory
		logger.info("Writting: %s" % out_file_name)
		logger.info("Status: %d" % status)
		out_file=open(out_file_name, "w")
		out_file.write("EXIT_STATUS=%d\n" % status)
		out_file.write("REAL_TIME=%d\n" % 10)
		out_file.close()

		err_file_name="%s/stderr.wrapper" % directory
		logger.info("Writting: %s" % err_file_name)
		err_file=open(err_file_name, "w")
		err_file.write("\n\n")
		err_file.close()

		
		
# MAD STUFF

def run(jl=None):
	global logger, log_handler, job_list
	logger=logging.getLogger("em")
	log_handler=logging.FileHandler("/tmp/nordugrid.log")
	log_formatter = logging.Formatter('%(asctime)s %(name)s %(levelname)s %(message)s')
	log_handler.setFormatter(log_formatter)
	logger.addHandler(log_handler)
	logger.setLevel(logging.INFO)
	
	job_list=jl
	#print job_list
	
	process_line()
	
def stop():
	if logger:
		logger.removeHandler(log_handler)
		log_handler.close()

def process_line():
	line=sys.stdin.readline()
	
	tmp_line=line.split()
	action=""
	jid=""
	contact=""
	rsl_file=""
	
	if len(tmp_line)>0:
		action=tmp_line[0]
		
	if len(tmp_line)>1:
		jid=tmp_line[1]
		
	if len(tmp_line)>2:
		contact=tmp_line[2]
		
	if len(tmp_line)>3:
		rsl_file=" ".join(tmp_line[3:])
	
	process_action(action, jid, contact, rsl_file)
	
def action_init(jid, contact, rsl_file):
	logger.info("INITIALIZATION SUCCESS")
	sys.stdout.write("INIT - SUCCESS -\n")
	sys.stdout.flush()
	
def action_submit(jid, contact, rsl_file):
	global job_list
	if not job_list.has_key(jid):
		job_list[jid]=GwJob(jid)
	logger.info(job_list.__str__())
	
	thread.start_new_thread(job_list[jid].submit, (contact, rsl_file))
	
def action_recover(jid, contact, rsl_file):
	print "RECOVER %s FAILURE Recover not implemented (404)" % jid
	
def action_cancel(jid, contact, rsl_file):
	# print "CANCEL %s FAILURE Cancel not implemented (404)" % jid
	if job_list.has_key(jid):
		thread.start_new_thread(job_list[jid].cancel, ())
	else:
		print "CANCEL %s FAILURE Job not initialized" % jid
		sys.stdout.flush()
	
	
def action_poll(jid, contact, rsl_file):
	# print "POLL %s FAILURE Poll not implemented (404)" % jid
	if job_list.has_key(jid):
		thread.start_new_thread(job_list[jid].standalone_poll, ())
		#thread.start_new_thread(job_list[jid].poll, ())
	else:
		print "POLL %s FAILURE Job not initialized" % jid
		sys.stdout.flush()
		
def action_updater(jid, contact, rsl_file):
	if job_list.has_key(jid):
		thread.start_new_thread(job_list[jid].status_updater, ())
	else:
		print "UPDATER %s FAILURE Job not initialized" % jid
		sys.stdout.flush()
	
		
def action_finalize(jid, contact, rsl_file):
	print "FINALIZE - SUCCESS -"
	sys.exit(0);

action_functions = {
	'INIT': 	action_init,
	'SUBMIT': 	action_submit,
	'RECOVER':	action_recover,
	'CANCEL':	action_cancel,
	'POLL':		action_poll,
	'FINALIZE':	action_finalize,
	'UPDATER':	action_updater
}

def process_action(action, jid, contact, rsl_file):
	logger.info("GOT MESSAGE: \"%s\"" % " ".join([action, jid, contact, rsl_file]))
	logger.info("  Action: %s" % action)
	logger.info("  Jid: %s" % jid)
	logger.info("  Contact: %s" % contact)
	logger.info("  RSL File: %s" % rsl_file)

	tmp_action=action.upper()
	if action_functions.has_key(tmp_action):
		action_functions[tmp_action](jid, contact, rsl_file)
	sys.stdout.flush()
