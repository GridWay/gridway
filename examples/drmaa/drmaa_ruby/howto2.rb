# --------------------------------------------------------------------------
# Copyright 2002-2013, GridWay Project Leads (GridWay.org)          
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

#######################
# CREATE JOB TEMPLATE #
#######################

def setup_job_template
  
  args=["-l", "-a"]
  
  (result, jt, error)=drmaa_allocate_job_template
  
  if result != DRMAA_ERRNO_SUCCESS
    STDERR.puts "drmaa_allocate_job_template() failed: #{error}"
    exit -1
  end
  
  cwd=Dir.getwd
  
  (result, error)=drmaa_set_attribute(jt, DRMAA_WD, cwd)
  if result != DRMAA_ERRNO_SUCCESS
    STDERR.puts "Error setting job template attribute: #{error}"
    exit -1
  end
  
  (result, error)=drmaa_set_attribute(jt, DRMAA_JOB_NAME, "ht2")
  (result, error)=drmaa_set_attribute(jt, DRMAA_REMOTE_COMMAND, "/bin/ls")
  (result, error)=drmaa_set_vector_attribute(jt, DRMAA_V_ARGV, args)
  
  if result != DRMAA_ERRNO_SUCCESS
    STDERR.puts "Error setting remote command arguments: #{error}"
    exit -1
  end
  
  (result, error)=drmaa_set_attribute(jt, DRMAA_OUTPUT_PATH, 
                                          ":stdout."+DRMAA_GW_JOB_ID)
                                        
  (result, error)=drmaa_set_attribute(jt, DRMAA_ERROR_PATH,
                                          ":stderr."+DRMAA_GW_JOB_ID)
                                          
  return jt
end

#############
# MAIN CODE #
#############

(result, error)=drmaa_init(nil)

if result != DRMAA_ERRNO_SUCCESS
  STDERR.puts "drmaa_init() failed: #{error}"
  exit -1
else
  puts "drmaa_init() success"
end

jt=setup_job_template

(result, job_id, error)=drmaa_run_job(jt)

if result != DRMAA_ERRNO_SUCCESS
  STDERR.puts "drmaa_run_job() failed: #{error}"
  exit -1
end

STDERR.puts "Job successfully submited ID: #{job_id}"

(result, job_id_out, stat, rusage, error)=drmaa_wait(job_id, DRMAA_TIMEOUT_WAIT_FOREVER)

if result != DRMAA_ERRNO_SUCCESS
  STDERR.puts "drmaa_wait() failed: #{error}"
  pp result, job_id_out, stat, rusage, error
  exit -1
end

(result, stat, error)=drmaa_wexitstatus(stat)

STDERR.puts "Job finished with exit code #{stat}, usage: #{job_id}"

(result, attr_value, error )=drmaa_get_next_attr_value(rusage)
while result != DRMAA_ERRNO_NO_MORE_ELEMENTS
  STDERR.puts "\t#{attr_value}"
  (result, attr_value, error )=drmaa_get_next_attr_value(rusage)
end

(result, error)=drmaa_release_attr_values(rusage)

# ----- Finalize -----

(result, error)=drmaa_delete_job_template(jt)

if result != DRMAA_ERRNO_SUCCESS
  STDERR.puts "drmaa_delete_job_template() failed: #{error}"
  exit -1
end

(result, error)=drmaa_exit

if result != DRMAA_ERRNO_SUCCESS
  STDERR.puts "drmaa_exit() failed: #{error}"
  exit -1
end

