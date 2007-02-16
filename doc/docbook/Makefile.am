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

%.html: %.sgml
	db2html -d gwdoc.dsl#html $<

%.pdf: %.sgml
	db2pdf -d gwdoc.dsl#print $<

images:
	ln -s ../images ./images

TARGETS=
if DB2HTMLC
if DB2PDFC
TARGETS+=htmls pdfs
else
TARGETS+=htmls
endif
else 
if DB2PDFC
TARGETS+=pdfs
endif
endif

all: $(TARGETS)

pdfs: images appdeveloperguide.pdf gwdeveloperguide.pdf \
        installconfguide.pdf  userguide.pdf \
        commandreference.pdf egeehowto.pdf

htmls: images appdeveloperguide.html  \
        gwdeveloperguide.html  \
        installconfguide.html  \
        userguide.html commandreference.html \
        egeehowto.html

clean:
	rm -rf userguide appdeveloperguide commandreference gwdeveloperguide installconfguide egeehowto *.junk *~ *.pdf images
