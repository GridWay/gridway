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
import re
import logging
import arclib

logger=None
log_handler=None

re_src=re.compile(r"^gsiftp://.*?/~/\.gw_.*?_.*?/(.*)$")
re_dst=re.compile(r"^file://(/.*)$")

def run():
	global logger, log_handler
	logger=logging.getLogger("tm")
	log_handler=logging.FileHandler("/tmp/nordugrid.log")
	log_formatter = logging.Formatter('%(asctime)s %(name)s %(levelname)s %(message)s')
	log_handler.setFormatter(log_formatter)
	logger.addHandler(log_handler)
	logger.setLevel(logging.INFO)
	
	process_line()
	
def stop():
	if logger:
		logger.removeHandler(log_handler)
		log_handler.close()

def process_line():
	line=sys.stdin.readline()
	
	tmp_line=line.split()
	action=""
	xfr_ids="-"
	cp_xfr_ids="-"
	mode="-"
	src_url="-"
	dst_url="-"
	
	if len(tmp_line)>0:
		action=tmp_line[0]
		
	if len(tmp_line)>1:
		xfr_ids=tmp_line[1]
		
	if len(tmp_line)>2:
		cp_xfr_ids=tmp_line[2]
		
	if len(tmp_line)>3:
		mode=tmp_line[3]

	if len(tmp_line)>4:
		src_url=tmp_line[4]
		
	if len(tmp_line)>5:
		dst_url=tmp_line[5]
	
	process_action(action, xfr_ids, cp_xfr_ids, mode, src_url, dst_url)
	
def action_init(xfr_ids, cp_xfr_ids, mode, src_url, dst_url):
	print "INIT - - SUCCESS -"

def action_start(xfr_ids, cp_xfr_ids, mode, src_url, dst_url):
	print "START %s - SUCCESS -" % xfr_ids
	
def action_end(xfr_ids, cp_xfr_ids, mode, src_url, dst_url):
	# DELETE TEMPORAL DOWNLOAD DIRECTORY
	print "END %s - SUCCESS -" % xfr_ids

def action_rmdir(xfr_ids, cp_xfr_ids, mode, src_url, dst_url):
	print "RMDIR %s - SUCCESS -" % xfr_ids

def action_exists(xfr_ids, cp_xfr_ids, mode, src_url, dst_url):
	print "EXISTS %s - SUCCESS -" % xfr_ids

def action_mkdir(xfr_ids, cp_xfr_ids, mode, src_url, dst_url):
	print "MKDIR %s - SUCCESS %s" % (xfr_ids, src_url)

def action_cp(xfr_ids, cp_xfr_ids, mode, src_url, dst_url):
	res_src=re_src.match(src_url)
	res_dst=re_dst.match(dst_url)
	
	if res_src and res_dst:
		s=res_src.group(1)
		dst=res_dst.group(1)
		GW_LOCATION=os.environ['GW_LOCATION']
		src="%s/var/%s/nordugrid/%s" % (GW_LOCATION, xfr_ids, s)
		try:
			shutil.copy(src, dst)
			print "CP %s %s SUCCESS (%s->%s)" % (xfr_ids, cp_xfr_ids, src, dst)
		except Exception, e:
			print "CP %s %s FAILURE (%s->%s)" % (xfr_ids, cp_xfr_ids, src, dst)
	else:
		print "CP %s %s SUCCESS (%s->%s)" % (xfr_ids, cp_xfr_ids, src_url, dst_url)

def action_finalize(xfr_ids, cp_xfr_ids, mode, src_url, dst_url):
	print "FINALIZE - - SUCCESS -"
	sys.exit(0);

action_functions = {
	'INIT': 	action_init,
	'START': 	action_start,
	'END':		action_end,
	'RMDIR':	action_rmdir,
	'EXISTS':	action_exists,
	'MKDIR':	action_mkdir,
	'CP':		action_cp,
	'FINALIZE':	action_finalize
}

def process_action(action, xfr_ids, cp_xfr_ids, mode, src_url, dst_url):
	logger.info("GOT MESSAGE: \"%s\"" % " ".join([action, xfr_ids, cp_xfr_ids, mode, src_url, dst_url]))
	logger.info("  Action: %s" % action)
	logger.info("  xfr_ids: %s" % xfr_ids)
	logger.info("  Cp_xfr_ids: %s" % cp_xfr_ids)
	logger.info("  Mode: %s" % mode)
	logger.info("  Src_url: %s" % src_url)
	logger.info("  Dst_url: %s" % dst_url)
	
	tmp_action=action.upper()
	if action_functions.has_key(tmp_action):
		action_functions[tmp_action](xfr_ids, cp_xfr_ids, mode, src_url, dst_url)
		
	sys.stdout.flush()
