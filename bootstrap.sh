#! /bin/sh
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

clean_bt(){
	make clean > /dev/null 2>&1 
	find . -name .dirstamp -exec rm {} \;
	rm -rf `find . -name .libs` > /dev/null 2>&1 
	rm -rf autom4te.cache > /dev/null 2>&1 
	rm Makefile > /dev/null 2>&1 	
	rm src/Makefile > /dev/null 2>&1 
	rm autoscan.log config.status libtool configure.scan config.log > /dev/null 2>&1 
	rm makefile-header gpt_build_temp.sh> /dev/null 2>&1 
	rm src/drmaa/drmaa.jar > /dev/null 2>&1 
}

clean_auto()
{
	rm config/compile config/config.guess config/config.sub > /dev/null 2>&1
	rm config/install-sh config/ltmain.sh config/missing config/ylwrap > /dev/null 2>&1 
	rm Makefile.in > /dev/null 2>&1 	
	rm src/Makefile.in > /dev/null 2>&1 	
	rm aclocal.m4 configure > /dev/null 2>&1
	rm configure > /dev/null 2>&1 	
}

if [ "x$1" = "xclean" ] ; then
	clean_bt
	exit 0 
fi

if [ "x$1" = "xclean_auto" ] ; then
	clean_bt
	clean_auto
	exit 0 
fi

if [ ! -d config ]; then 
	mkdir config
fi

autoscan
aclocal
libtoolize --copy --force
automake -i --foreign --copy --add-missing
autoconf
