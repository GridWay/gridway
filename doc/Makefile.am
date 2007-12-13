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

if DRMAA1
DRMAA_SRC_DIR=drmaa/drmaa1.0
else
DRMAA_SRC_DIR=drmaa
endif

DRMAA_PACK =../src/$(DRMAA_SRC_DIR)

all: doxygen javadoc

doxygen:
if DOXYGENC
	doxygen drmaa_c/drmaa.dox
	cp drmaa_c/index.html drmaa_c/html/index.html
	cp drmaa_c/gwdoc.css drmaa_c/html/gwdoc.css
endif

javadoc:
if JAVADOCC
	javadoc org.ggf.drmaa -sourcepath $(DRMAA_PACK) -d ./drmaa_java
endif

clean:
	rm -rf drmaa_java drmaa_c/html drmaa_c/latex
