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


use DRMAA;

sub setup_job_template {
	my $args=["-l", "-a"];
	my $jt;

	($result, $jt, $error)=drmaa_allocate_job_template();
	
	if($result != $DRMAA_ERRNO_SUCCESS) {
		die "drmaa_allocate_job_template() failed: ".$error;
		exit -1;
	}
	
	$cwd=`pwd`;
	chomp($cwd);
	
	($result, $error)=drmaa_set_attribute($jt, $DRMAA_WD, $cwd);
	if($result != $DRMAA_ERRNO_SUCCESS) {
		die "Error setting job template attribute: ".$error;
		exit -1;
	}
	
	($result, $error)=drmaa_set_attribute($jt, $DRMAA_JOB_NAME, "ht2");
	($result, $error)=drmaa_set_attribute($jt, $DRMAA_REMOTE_COMMAND, "/bin/ls");
	($result, $error)=drmaa_set_vector_attribute($jt, $DRMAA_V_ARGV, $args);
	
	($result, $value, $error)=drmaa_get_attribute($jt, $DRMAA_JOB_NAME);
	
	if($result != $DRMAA_ERRNO_SUCCESS) {
		die "Error setting remote command arguments: ".$error;
		exit -1;
	}
	
	($result, $error)=drmaa_set_attribute($jt, $DRMAA_OUTPUT_PATH,
												"stdout.".$DRMAA_GW_JOB_ID);
	($result, $error)=drmaa_set_attribute($jt, $DRMAA_ERROR_PATH,
												"stderr.".$DRMAA_GW_JOB_ID);
	
	return $jt;
}

($result, $error)=drmaa_init(undef);

if ($result != $DRMAA_ERRNO_SUCCESS) {
  die "drmaa_init() failed: ${error}";
  exit -1;
} else {
  print "drmaa_init() success\n";
}

my $jt=setup_job_template();

($result, $job_id, $error)=drmaa_run_job($jt);

if ($result != $DRMAA_ERRNO_SUCCESS) {
  die "drmaa_run_job() failed: ${error}";
  exit -1;
}

print "Job successfully submited ID: ${job_id}\n";

($result, $job_id_out, $stat, $rusage, $error)=drmaa_wait($job_id, $DRMAA_TIMEOUT_WAIT_FOREVER);

if ($result != $DRMAA_ERRNO_SUCCESS) {
  die "drmaa_wait() failed: ${error}";
  exit -1;
}

($result, $stat, $error)=drmaa_wexitstatus($stat);

print "Job finished with exit code ${stat}, usage: ${job_id}\n";

($result, $attr_value, $error )=drmaa_get_next_attr_value($rusage);
while ($result != $DRMAA_ERRNO_NO_MORE_ELEMENTS) {
  print "\t${attr_value}\n";
  ($result, $attr_value, $error)=drmaa_get_next_attr_value($rusage);
}

($result, $error)=drmaa_release_attr_values($rusage);

# ----- Finalize -----

($result, $error)=drmaa_delete_job_template($jt);

if ($result != $DRMAA_ERRNO_SUCCESS) {
  die "drmaa_delete_job_template() failed: ${error}";
  exit -1;
}

($result, $error)=drmaa_exit();

if (result != $DRMAA_ERRNO_SUCCESS) {
  die "drmaa_exit() failed: ${error}";
  exit -1;
}

