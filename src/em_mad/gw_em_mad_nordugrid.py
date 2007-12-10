#!/usr/bin/env python

# ------------------------------------------------------------------------------
# Copyright 2002-2006 GridWay Team, Distributed Systems Architecture
# Group, Universidad Complutense de Madrid
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


import exceptions
import traceback
import sys
import em_nordugrid

#global job_list
job_list = {}

while True:
	try:
		#global job_list

		em_nordugrid.stop()
		reload(em_nordugrid)
		em_nordugrid.run(job_list)
	except exceptions.KeyboardInterrupt, e:
		sys.exit(-1)
	except exceptions.SystemExit, e:
		print e
		sys.exit(0)
	except Exception, e:
		print "Pero que muuuvi!!!"
		print e.__class__, e
		traceback.print_exc(file=sys.stdout)
	
	
	




