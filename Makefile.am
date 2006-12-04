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

SUBDIRS = src

if DOC
SUBDIRS += doc/docbook
if DRMAA
SUBDIRS += doc
endif
endif

FLAGS=$(DB4_INCLUDE_PATH)

install-exec-hook:
if DOC
        @rm -rf @prefix@/doc
        @mkdir -p @prefix@/doc
        @cp -r doc/docbook/userguide \
                   doc/docbook/appdeveloperguide \
                   doc/docbook/commandreference \
                   doc/docbook/gwdeveloperguide \
                   doc/docbook/installconfguide \
                   doc/images \
                   doc/docbook/*.pdf \
                   @prefix@/doc
if DRMAA
        @cp -r doc/drmaa_java doc/drmaa_c @prefix@/doc
endif
endif
if EXAMPLES
        @rm -rf @prefix@/examples
        @cp -r examples @prefix@/examples
endif
if TESTS
        @rm -rf @prefix@/test
        @mkdir -p @prefix@/test
        @cp -r src/test/gwtest src/test/jt @prefix@/test
endif
# ------------------
# -- Script renaming - Commons
# -------------------
        @mv @prefix@/bin/gw_im_mad_static.sh @prefix@/bin/gw_im_mad_static
        @mv @prefix@/bin/gw_tm_mad_ftp.sh @prefix@/bin/gw_tm_mad_ftp
# ------------------
# -- Script renaming - WS
# -------------------
if WS
        @mv @prefix@/bin/gw_em_mad_ws.sh @prefix@/bin/gw_em_mad_ws
        @mv @prefix@/bin/gw_tm_mad_rft.sh @prefix@/bin/gw_tm_mad_rft
        @mv @prefix@/bin/gw_im_mad_mds4.sh @prefix@/bin/gw_im_mad_mds4
endif
# ------------------
# -- Script renaming - PREWS
# -------------------
if PREWS
        @mv @prefix@/bin/gw_im_mad_mds2.sh @prefix@/bin/gw_im_mad_mds2
        @mv @prefix@/bin/gw_im_mad_mds2_glue.sh @prefix@/bin/gw_im_mad_mds2_glue
        @mv @prefix@/bin/gw_tm_mad_dummy.sh @prefix@/bin/gw_tm_mad_dummy

        @mv @prefix@/bin/gw_em_mad_prews.sh @prefix@/bin/gw_em_mad_prews

endif

uninstall-hook:
        @rm @prefix@/bin/gw_im_mad_mds2
        @rm @prefix@/bin/gw_im_mad_mds4
        @rm @prefix@/bin/gw_em_mad_ws
        @rm @prefix@/bin/gw_im_mad_mds2_glue
        @rm @prefix@/bin/gw_tm_mad_ftp
        @rm @prefix@/bin/gw_tm_mad_rft
        @rm @prefix@/bin/gw_em_mad_prews
        @rm @prefix@/bin/gw_im_mad_static
        @rm @prefix@/bin/gw_tm_mad_dummy
        @rm -rf @prefix@/var

# ----------------------------
# Dist specifics
# ----------------------------

dist-hook:
        @cp -r src/drmaa/org $(distdir)/src/drmaa
        @rm -rf $(distdir)/drmaa/org/CVS $(distdir)/drmaa/org/ggf/CVS $(distdir)/drmaa/org/ggf/drmaa/CVS
        @mkdir -p $(distdir)/doc/docbook
        @cp -r doc/images $(distdir)/doc
        @cp -r doc/drmaa_c $(distdir)/doc
        @cp -r doc/docbook/*.sgml $(distdir)/doc/docbook
        @cp -r doc/docbook/*.css $(distdir)/doc/docbook
        @cp -r doc/docbook/*.dsl $(distdir)/doc/docbook
        @rm -rf $(distdir)/doc/images/CVS $(distdir)/doc/drmaa_c/CVS $(distdir)/doc/docbook/CVS
        @cp -r examples $(distdir)
        @cp -r src/test/jt $(distdir)/src/test