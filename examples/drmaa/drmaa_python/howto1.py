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

from DRMAA import *
import sys

(result, error)=drmaa_init(None)

if result != DRMAA_ERRNO_SUCCESS:
	print "drmaa_init() failed: "+error
	sys.exit(-1)
else:
	print "drmaa_init() success"
	
	(result, contact, error)=drmaa_get_contact()
	(result, major, minor, error)=drmaa_version()
	(result, drm, error)=drmaa_get_DRM_system()
	(result, impl, error)=drmaa_get_DRMAA_implementation()
  
	print "Using "+impl+", details:"
	print "\t DRMAA version "+str(major)+"."+str(minor)
	print "\t DRMS "+drm+" (contact: "+contact+")"
	
	(result, error)=drmaa_exit()
	
	if result != DRMAA_ERRNO_SUCCESS:
		print "drmaa_exit() failed: "+error
		sys.exit(-1)
	else:
		print "drmaa_exit() success"
	
	sys.exit(0)

