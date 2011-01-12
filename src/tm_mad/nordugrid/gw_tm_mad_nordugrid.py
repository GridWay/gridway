#!/usr/bin/env python

# ------------------------------------------------------------------------------
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
# ------------------------------------------------------------------------------


import exceptions
import traceback
import sys
import tm_nordugrid

while True:
	try:
		tm_nordugrid.stop()
		reload(tm_nordugrid)
		tm_nordugrid.run()
	except exceptions.KeyboardInterrupt, e:
		sys.exit(-1)
	except exceptions.SystemExit, e:
		print e
		sys.exit(0)
	except Exception, e:
		print "Pero que muuuvi!!!"
		print e.__class__, e
		traceback.print_exc(file=sys.stdout)
	
	
	




