# --------------------------------------------------------------------------
# Copyright 2002-2011, GridWay Project Leads (GridWay.org)          
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


require 'DRMAA'
include DRMAA

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
	(result, error)=drmaa_set_attribute(jt, DRMAA_JOB_NAME, "ht4")
	(result, error)=drmaa_set_attribute(jt, DRMAA_REMOTE_COMMAND, "/bin/ls")
	(result, error)=drmaa_set_vector_attribute(jt, DRMAA_V_ARGV, args)
	
	return jt
end


########
# MAIN #
########

# ------ INIT DRMAA Library ------
(result, error)=drmaa_init(nil)

if result != DRMAA_ERRNO_SUCCESS
	STDERR.puts "drmaa_init() dailed: #{error}"
	exit -1
end

jt=setup_job_template

(result, error)=drmaa_set_attribute(jt, DRMAA_OUTPUT_PATH, 
																		"stdout."+DRMAA_PLACEHOLDER_INCR)

(result, error)=drmaa_set_attribute(jt, DRMAA_OUTPUT_PATH, 
																		"stdout."+DRMAA_PLACEHOLDER_INCR)


# ------ Submit 5 jobs ------
(result, jobids, error)=drmaa_run_bulk_jobs(jt, 0, 1, 1)

if result != DRMAA_ERRNO_SUCCESS
	STDERR.puts "drmaa_run_bulk() failed: #{error}"
	exit -1
end

STDERR.puts "Bulk job succesfully submited IDs are"

begin
	(result, value, error)=drmaa_get_next_job_id(jobids)
		
	if result == DRMAA_ERRNO_SUCCESS
		STDERR.puts "\t#{value}"
	end
end while result != DRMAA_ERRNO_NO_MORE_ELEMENTS
	
STDERR.puts "Waiting for bulk job to finish..."

(result, error)=drmaa_synchronize([DRMAA_JOB_IDS_SESSION_ALL], DRMAA_TIMEOUT_WAIT_FOREVER, 0)

STDERR.puts "All Jobs finished"

begin
	(resultj, value, error)=drmaa_get_next_job_id(jobids)
	
	if(resultj == DRMAA_ERRNO_SUCCESS)
		(result, job_id_out, stat, rusage, error)=drmaa_wait(value, DRMAA_TIMEOUT_WAIT_FOREVER)
		
		(result, stat, error)=drmaa_wexitstatus(stat)
		
		STDERR.puts "Rusage for task #{value} (exit code #{stat})"
		
		begin
			(result, value, error)=drmaa_get_next_attr_value(rusage)
			
			if result == DRMAA_ERRNO_SUCCESS
				STDERR.puts "\t#{value}"
			end
		end while result != DRMAA_ERRNO_NO_MORE_ELEMENTS

		drmaa_release_attr_values(rusage)
	end
end while resultj != DRMAA_ERRNO_NO_MORE_ELEMENTS
	
drmaa_release_job_ids(jobids)

# ------ Finalize ------
(result, error)=drmaa_delete_job_template(jt)

(result, error)=drmaa_exit

if result != DRMAA_ERRNO_SUCCESS
	STDERR.puts "drmaa_exit() failed: #{error}"
	exit -1
end
