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
	find -name .dirstamp -delete
	rm -rf `find -name .libs`
	rm -rf config
	rm -rf autom4te.cache
	rm Makefile Makefile.in
	rm doc/Makefile doc/Makefile.in
	rm doc/docbook/Makefile doc/docbook/Makefile.in	
	rm src/Makefile src/Makefile.in
	rm aclocal.m4 autoscan.log config.status configure libtool configure.scan
    rm makefile-header config.log
    rm src/drmaa/drmaa.jar
	return 0
fi

if [ ! -d config ] ; then
    mkdir config
    autoscan
    touch makefile-header
fi    

libtoolize --force 2>&1 \
&& aclocal  2>&1 \
&& automake -i --foreign --copy --add-missing 2>&1 \
&& autoconf  2>&1
