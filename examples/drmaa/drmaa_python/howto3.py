# --------------------------------------------------------------------------
# Copyright 2002-2010, GridWay Project Leads (GridWay.org)          
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
# --------------------------------------------------------------------------

from DRMAA import *
import sys, os, time

def print_job_status(id):
	
	# ----- Check job state -----
	
	(result, status, error)=drmaa_job_ps(id)
	
	if result != DRMAA_ERRNO_SUCCESS:
		print >> sys.stderr, "drmaa_job_ps() failed: %s" % (error)
		sys.exit(-1)

	##################################################
	# drmaa_gw_strstatus is not a DRMAA 1.0 function #
	# it is only provided by Gridway                 #
	##################################################

	print "Job state is: " + drmaa_gw_strstatus(status)	

def setup_job_template():
	
	args=["-l", "-a"]
	
	(result, jt, error)=drmaa_allocate_job_template()
	
	if result != DRMAA_ERRNO_SUCCESS:
		print >> sys.stderr, "drmaa_allocate_job_template() failed: %s" % (error)
		sys.exit(-1)

	cwd=os.getcwd()

	# SHOULD CHECK result, REMOVED FOR THE SAKE OF CLARITY

	(result, error)=drmaa_set_attribute(jt, DRMAA_WD, cwd)
	(result, error)=drmaa_set_attribute(jt, DRMAA_JOB_NAME, "ht3")
	(result, error)=drmaa_set_attribute(jt, DRMAA_REMOTE_COMMAND, "/bin/ls")
	(result, error)=drmaa_set_vector_attribute(jt, DRMAA_V_ARGV, args)
	
	(result, error)=drmaa_set_attribute(jt, DRMAA_OUTPUT_PATH, 
					"stdout."+DRMAA_GW_JOB_ID)

	(result, error)=drmaa_set_attribute(jt, DRMAA_ERROR_PATH,
					"stderr."+DRMAA_GW_JOB_ID)

	return jt

########
# MAIN #
########

job_ids=[DRMAA_JOB_IDS_SESSION_ALL]

# ------ INIT DRMAA Library ------
(result, error)=drmaa_init(None)

if result != DRMAA_ERRNO_SUCCESS:
	print >> sys.stderr, "drmaa_init() failed: %s" % (error)
	sys.exit(-1)

# ------ Set Up job template ------
jt=setup_job_template()

# ------ Set Up job submission state ------
(result, error)=drmaa_set_attribute(jt, 
				DRMAA_JS_STATE, 
				DRMAA_SUBMISSION_STATE_HOLD)

# ------ Submit the job ------
(result, job_id, error)=drmaa_run_job(jt)

if result != DRMAA_ERRNO_SUCCESS:
	print >> sys.stderr, "drmaa_run_job() failed: %s" % (error)
	sys.exit(-1)

print "Your job has been submitted with id: %s" % (job_id)

# ------ Check job state ------
time.sleep(5)

print_job_status(job_id)

time.sleep(1)

# ------ Control op. ------
print "Releasing the Job"

(result, error)=drmaa_control(job_id, DRMAA_CONTROL_RELEASE)

if result != DRMAA_ERRNO_SUCCESS:
	print >> sys.stderr, "drmaa_control() failed: %s" % (error)
	sys.exit(-1)

# ------ Check job state again ------
print_job_status(job_id)

# ------ synchronize() ------
print "Synchronizing with job..."

(result, error)=drmaa_synchronize(job_ids, DRMAA_TIMEOUT_WAIT_FOREVER, 0)

if result != DRMAA_ERRNO_SUCCESS:
	print >> sys.stderr, "drmaa_synchronize failed: %s" % (error)
	sys.exit(-1)

# ------ Control op. --------------------------------------- #
# You can kill the job using dispose=1 in the previous call  #
# You could also use drmaa_wait to get rusage and remove job #
# ---------------------------------------------------------- #

print "Killing the Job"

(result, error)=drmaa_control(job_id, DRMAA_CONTROL_TERMINATE)

if result != DRMAA_ERRNO_SUCCESS:
	print >> sys.stderr, "drmaa_control() failed: %s" % (error)
	sys.exit(-1)

print "Your job has been deleted"

# ------ Finalize ------
(result, error)=drmaa_delete_job_template(jt)

(result, error)=drmaa_exit()

if result != DRMAA_ERRNO_SUCCESS:
	print >> sys.stderr, "drmaa_exit() failed: %s" % (error)
	sys.exit(-1)
