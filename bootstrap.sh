#! /bin/sh
# --------------------------------------------------------------------------
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
# --------------------------------------------------------------------------



if [ "x$1" = "xclean" ] ; then
    make clean > /dev/null 2>&1 
	find -name .dirstamp -delete > /dev/null 2>&1 
	rm -rf `find -name .libs` > /dev/null 2>&1 
#	rm -rf config > /dev/null 2>&1 
	rm -rf autom4te.cache > /dev/null 2>&1 
	rm Makefile > /dev/null 2>&1 
#	rm Makefile.in > /dev/null 2>&1 	
	rm doc/Makefile > /dev/null 2>&1 
#	rm doc/Makefile.in > /dev/null 2>&1 	
	rm doc/docbook/Makefile > /dev/null 2>&1 
#	rm doc/docbook/Makefile.in	> /dev/null 2>&1 	
	rm src/Makefile > /dev/null 2>&1 
#	rm src/Makefile.in > /dev/null 2>&1 	
	rm autoscan.log config.status libtool configure.scan config.log > /dev/null 2>&1 
#	rm aclocal.m4 configure > /dev/null 2>&1 	
    rm makefile-header > /dev/null 2>&1 
    rm src/drmaa/drmaa.jar > /dev/null 2>&1 
	exit 0 
fi

if [ ! -d config ] ; then
    mkdir config
    autoscan
fi    

libtoolize --force \
&& aclocal  \
&& automake -i --foreign --copy --add-missing \
&& autoconf 
