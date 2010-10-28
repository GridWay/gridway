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

require 'pp'

require 'DRMAA'
include DRMAA

def print_job_status(id)
  
  # ----- Check job state -----
  
  (result, status, error)=drmaa_job_ps(id)
  
  if result != DRMAA_ERRNO_SUCCESS
    STDERR.puts "drmaa_job_ps() failed: #{error}"
    exit -1
  end
  
  ##################################################
  # drmaa_gw_strstatus us bir a DRMAA 1.0 function #
  # it is only provided by Gridway                 #
  ##################################################

  puts "Job state is: " + drmaa_gw_strstatus(status)  
end

def setup_job_template
  
  args=["-l", "-a"]
  
  (result, jt, error)=drmaa_allocate_job_template
  
  if result != DRMAA_ERRNO_SUCCESS
    STDERR.puts "drmaa_allocate_job_template() failed: #{error}"
    exit -1
  end
  
  cwd=Dir.getwd

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
end

########
# MAIN #
########

job_ids=[DRMAA_JOB_IDS_SESSION_ALL]

# ------ INIT DRMAA Library ------
(result, error)=drmaa_init(nil)

if result != DRMAA_ERRNO_SUCCESS
  STDERR.puts "drmaa_init() failed: #{error}"
  exit -1
end

# ------ Set Up job template ------
jt=setup_job_template

# ------ Set Up job submission state ------
(result, error)=drmaa_set_attribute(jt, 
                                    DRMAA_JS_STATE, 
                                    DRMAA_SUBMISSION_STATE_HOLD)

# ------ Submit the job ------
(result, job_id, error)=drmaa_run_job(jt)

if result != DRMAA_ERRNO_SUCCESS
  STDERR.puts "drmaa_run_job() failed: #{error}"
  exit -1
end

puts "Your job has been submitted with id: #{job_id}"

# ------ Check job state ------
sleep(5)

print_job_status(job_id)

sleep(1)

# ------ Control op. ------
puts "Releasing the Job"

(result, error)=drmaa_control(job_id, DRMAA_CONTROL_RELEASE)

if result != DRMAA_ERRNO_SUCCESS
  STDERR.puts "drmaa_control() failed: #{error}"
  exit -1
end

# ------ Check job state again ------
print_job_status(job_id)

# ------ synchronize() ------
puts "Synchronizing with job..."

(result, error)=drmaa_synchronize(job_ids, DRMAA_TIMEOUT_WAIT_FOREVER, 0)

if result != DRMAA_ERRNO_SUCCESS
  STDERR.puts "drmaa_synchronize failed: #{error}"
  exit -1
end

# ------ Control op. --------------------------------------- #
# You can kill the job using dispose=1 in the previous call  #
# You could also use drmaa_wait to get rusage and remove job #
# ---------------------------------------------------------- #

puts "Killing the Job"

(result, error)=drmaa_control(job_id, DRMAA_CONTROL_TERMINATE)

if result != DRMAA_ERRNO_SUCCESS
  STDERR.puts "drmaa_control() failed: #{error}"
  exit -1
end

puts "Your job has been deleted"

# ------ Finalize ------
(result, error)=drmaa_delete_job_template(jt)

(result, error)=drmaa_exit

if result != DRMAA_ERRNO_SUCCESS
  STDERR.puts "drmaa_exit() failed: #{error}"
  exit -1
end
