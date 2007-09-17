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
JAVA_HOME = @java_home@

if GW_GLOBUS_DIRS
GW_ETC_DIR   = etc/gridway
GW_VAR_DIR   = var/gridway
GW_DOC_DIR   = doc/gridway
GW_SHARE_DIR = share/gridway
GW_TEST_DIR  = test/gridway
else
GW_ETC_DIR   = etc
GW_VAR_DIR   = var
GW_DOC_DIR   = doc
GW_SHARE_DIR = share
GW_TEST_DIR  = test
endif


if DOC
SUBDIRS += doc/docbook
if DRMAA
SUBDIRS += doc
endif
endif

FLAGS=$(DB4_INCLUDE_PATH)

install-exec-hook:
	@mkdir -p @prefix@/$(GW_SHARE_DIR)
	@cp -r LICENSE NOTICE @prefix@/$(GW_SHARE_DIR)
if DOC
	@rm -rf @prefix@/$(GW_DOC_DIR)
	@mkdir -p @prefix@/$(GW_DOC_DIR)
if DB2HTMLC
	@cp -r doc/docbook/userguide \
           doc/docbook/appdeveloperguide \
           doc/docbook/commandreference \
           doc/docbook/gwdeveloperguide \
           doc/docbook/installconfguide \
           doc/docbook/egeehowto \
           doc/docbook/gwdoc.css \
           doc/docbook/gwdoc.dsl \
           doc/images \
           @prefix@/$(GW_DOC_DIR)
endif
if DB2PDFC
	@cp -r doc/docbook/*.pdf \
           @prefix@/$(GW_DOC_DIR)
endif

if DRMAA
if DOXYGENC
	@cp -r doc/drmaa_c @prefix@/$(GW_DOC_DIR)
endif
if JAVADOCC
	@cp -r doc/drmaa_java @prefix@/$(GW_DOC_DIR)
endif
endif
endif

	@rm -rf @prefix@/$(GW_SHARE_DIR)/examples
	@mkdir -p @prefix@/$(GW_SHARE_DIR)
	@cp -r examples @prefix@/$(GW_SHARE_DIR)/examples

if TESTS
	@rm -rf @prefix@/$(GW_TEST_DIR)
	@mkdir -p @prefix@/$(GW_TEST_DIR)
	@cp -r src/test/gwtest src/test/TEST.pl \
	       src/test/jt \
	       @prefix@/$(GW_TEST_DIR)
endif

# ----------------------------
# -- Script renaming - Commons
# ----------------------------

# -- Dagman Ruby Scripts
	@mkdir -p @prefix@/libexec/ruby/dagman
	@mv @prefix@/bin/gw_dagman.rb @prefix@/bin/gwdagman
	@mv @prefix@/bin/node.rb @prefix@/libexec/ruby/dagman/node.rb
	@mv @prefix@/bin/parse.rb @prefix@/libexec/ruby/dagman/parse.rb
	@mv @prefix@/bin/runner.rb @prefix@/libexec/ruby/dagman/runner.rb
	@mv @prefix@/bin/gridway.rb @prefix@/libexec/ruby/dagman/gridway.rb

	@mv @prefix@/bin/gw_im_mad_static.sh @prefix@/bin/gw_im_mad_static
if GRIDFTP
	@mv @prefix@/bin/gw_tm_mad_ftp.sh @prefix@/bin/gw_tm_mad_ftp
endif

# ----------------------------
# -- Script renaming - WS
# ----------------------------

if WS
	@mv @prefix@/bin/gw_em_mad_ws.sh @prefix@/bin/gw_em_mad_ws
	@mv @prefix@/bin/gw_im_mad_mds4.sh @prefix@/bin/gw_im_mad_mds4
endif

# ----------------------------
# -- Script renaming - PREWS
# ----------------------------

if PREWS
	@mv @prefix@/bin/gw_im_mad_mds2.sh @prefix@/bin/gw_im_mad_mds2
	@mv @prefix@/bin/gw_im_mad_mds2_glue.sh @prefix@/bin/gw_im_mad_mds2_glue
	@mv @prefix@/bin/gw_tm_mad_dummy.sh @prefix@/bin/gw_tm_mad_dummy
	@mv @prefix@/bin/gw_em_mad_prews.sh @prefix@/bin/gw_em_mad_prews
endif

# ----------------------------
# -- Script renaming - JSDL
# ----------------------------

if JSDL
	@mv @prefix@/bin/jsdl2gw.sh @prefix@/bin/jsdl2gw
endif

# ****************************
# DIR LAYOUT
# ****************************

install-data-hook:
	mkdir -p @prefix@/$(GW_VAR_DIR)/acct
	mkdir -p @prefix@/$(GW_ETC_DIR)
	cp -r $(top_srcdir)/etc/im_examples @prefix@/$(GW_ETC_DIR)/
	cp $(top_srcdir)/etc/gwd.conf \
	   $(top_srcdir)/etc/job_template.default \
	   $(top_srcdir)/etc/sched.conf \
	   $(top_srcdir)/etc/gwrc \
	   @prefix@/$(GW_ETC_DIR)/

# ----------------------------
#   Dagman Examples
# ----------------------------
	@ln -s A.jt @prefix@/$(GW_SHARE_DIR)/examples/dagman/B.jt
	@ln -s A.jt @prefix@/$(GW_SHARE_DIR)/examples/dagman/C.jt
	@ln -s A.jt @prefix@/$(GW_SHARE_DIR)/examples/dagman/D.jt
	@ln -s A.jt @prefix@/$(GW_SHARE_DIR)/examples/dagman/E.jt
	@ln -s A.jt @prefix@/$(GW_SHARE_DIR)/examples/dagman/F.jt
	@ln -s A.jt @prefix@/$(GW_SHARE_DIR)/examples/dagman/G.jt
	@ln -s A.jt @prefix@/$(GW_SHARE_DIR)/examples/dagman/H.jt
	@ln -s A.jt @prefix@/$(GW_SHARE_DIR)/examples/dagman/I.jt
	@ln -s A.jt @prefix@/$(GW_SHARE_DIR)/examples/dagman/J.jt
	@ln -s A.jt @prefix@/$(GW_SHARE_DIR)/examples/dagman/K.jt
# ----------------------------
#    remove CVS DIRS
# ----------------------------

	@rm -rf @prefix@/$(GW_DOC_DIR)/drmaa_c/CVS \
                @prefix@/$(GW_DOC_DIR)/images/CVS \
                @prefix@/$(GW_ETC_DIR)/im_examples/CVS \
                @prefix@/$(GW_TEST_DIR)/jt/CVS \
		@prefix@/$(GW_ETC_DIR)/im_examples/CVS \
		@prefix@/$(GW_SHARE_DIR)/examples/im/CVS \
		@prefix@/$(GW_SHARE_DIR)/examples/ls/CVS \
		@prefix@/$(GW_SHARE_DIR)/examples/pi/CVS \
		@prefix@/$(GW_SHARE_DIR)/examples/CVS \
		@prefix@/$(GW_SHARE_DIR)/examples/drmaa_java/CVS \
		@prefix@/$(GW_SHARE_DIR)/examples/drmaa/CVS \
		@prefix@/$(GW_SHARE_DIR)/examples/drmaa/drmaa_java/CVS \
		@prefix@/$(GW_SHARE_DIR)/examples/drmaa/drmaa_c/CVS \
		@prefix@/$(GW_SHARE_DIR)/examples/drmaa/TestSuite/CVS \
		@prefix@/$(GW_SHARE_DIR)/examples/drmaa/TestSuite/org/CVS \
		@prefix@/$(GW_SHARE_DIR)/examples/drmaa/TestSuite/org/ggf/CVS \
		@prefix@/$(GW_SHARE_DIR)/examples/drmaa/TestSuite/org/ggf/drmaa/CVS \
		@prefix@/$(GW_SHARE_DIR)/examples/drmaa/TestSuite/org/ggf/drmaa/testSuite/CVS \
		@prefix@/$(GW_SHARE_DIR)/examples/sleep/CVS \
		@prefix@/$(GW_SHARE_DIR)/examples/drmaaTestSuite/CVS \
		@prefix@/$(GW_SHARE_DIR)/examples/drmaaTestSuite/org/CVS \
		@prefix@/$(GW_SHARE_DIR)/examples/drmaaTestSuite/org/ggf/CVS \
		@prefix@/$(GW_SHARE_DIR)/examples/drmaaTestSuite/org/ggf/drmaa/CVS \
		@prefix@/$(GW_SHARE_DIR)/examples/drmaaTestSuite/org/ggf/drmaa/testSuite/CVS \
		@prefix@/$(GW_SHARE_DIR)/examples/workflow/CVS \
		@prefix@/$(GW_SHARE_DIR)/examples/mpi/CVS \
		@prefix@/$(GW_SHARE_DIR)/examples/dagman/CVS \
		@prefix@/$(GW_SHARE_DIR)/examples/jsdl/CVS

# ----------------------------
# Copy wrapper, monitor & gw_mad_common.sh
# ----------------------------

		@mkdir -p @prefix@/libexec
		@cp $(top_srcdir)/scripts/wrapper.sh @prefix@/libexec/gw_wrapper.sh
		@cp $(top_srcdir)/scripts/monitor.sh @prefix@/libexec/gw_monitor.sh
		@mkdir -p @prefix@/bin
		@cp $(top_srcdir)/scripts/gw_mad_common.sh @prefix@/bin

uninstall-hook:
	@rm -f @prefix@/bin/gwdagman
	@rm -f @prefix@/bin/gw_im_mad_mds2
	@rm -f @prefix@/bin/gw_im_mad_mds4
	@rm -f @prefix@/bin/gw_em_mad_ws
	@rm -f @prefix@/bin/gw_im_mad_mds2_glue
	@rm -f @prefix@/bin/gw_tm_mad_ftp
	@rm -f @prefix@/bin/gw_tm_mad_rft
	@rm -f @prefix@/bin/gw_em_mad_prews
	@rm -f @prefix@/bin/gw_im_mad_static
	@rm -f @prefix@/bin/gw_tm_mad_dummy
	@rm -f @prefix@/bin/jsdl2gw
	@rm -rf @prefix@/$(GW_VAR_DIR)
	@rm -rf @prefix@/lib/ruby/

# *****************************************
# Dist specifics (see also src/Makefile.am)
# *****************************************

dist-hook:
	@rm -rf $(distdir)/src/drmaa/org/ggf/drmaa/.dirstamp
	@rm -rf $(distdir)/src/drmaa/org/ggf/drmaa/.libs
	@rm -f $(distdir)/src/drmaa/org/ggf/drmaa/DrmaaJNI.lo
	@rm -f $(distdir)/src/drmaa/org/ggf/drmaa/DrmaaJNI.o
	@mkdir -p $(distdir)/doc/docbook
	@cp -r doc/images $(distdir)/doc
	@cp -r doc/drmaa_c $(distdir)/doc
	@cp -r doc/docbook/*.sgml $(distdir)/doc/docbook
	@cp -r doc/docbook/*.css $(distdir)/doc/docbook
	@cp -r doc/docbook/*.dsl $(distdir)/doc/docbook
	@cp -r examples $(distdir)
	@cp -r etc $(distdir)
	@cp -r src/test/jt $(distdir)/src/test
	@cp -r src/test/TEST.pl $(distdir)/src/test
	@cp -r scripts $(distdir)
	@cp pkg_data_src.gpt  $(distdir)/pkg_data_src.gpt

# ----------------------------
#    remove CVS DIRS
# ----------------------------

	@rm -rf $(distdir)/doc/drmaa_c/CVS \
	        $(distdir)/doc/images/CVS \
	        $(distdir)/scripts/CVS \
                $(distdir)/etc/im_examples/CVS \
                $(distdir)/src/cmds/dagman/CVS \
                $(distdir)/src/drmaa/org/CVS \
                $(distdir)/src/drmaa/org/ggf/CVS \
                $(distdir)/src/drmaa/org/ggf/drmaa/CVS \
                $(distdir)/src/test/jt/CVS \
		$(distdir)/doc/CVS \
		$(distdir)/etc/im_examples/CVS \
		$(distdir)/etc/CVS \
		$(distdir)/examples/im/CVS \
		$(distdir)/examples/ls/CVS \
		$(distdir)/examples/pi/CVS \
		$(distdir)/examples/CVS \
		$(distdir)/examples/drmaa_java/CVS \
		$(distdir)/examples/drmaa/CVS \
		$(distdir)/examples/drmaa/drmaa_java/CVS \
		$(distdir)/examples/drmaa/drmaa_c/CVS \
		$(distdir)/examples/drmaa/TestSuite/CVS \
		$(distdir)/examples/drmaa/TestSuite/org/CVS \
		$(distdir)/examples/drmaa/TestSuite/org/ggf/CVS \
		$(distdir)/examples/drmaa/TestSuite/org/ggf/drmaa/CVS \
		$(distdir)/examples/drmaa/TestSuite/org/ggf/drmaa/testSuite/CVS \
		$(distdir)/examples/sleep/CVS \
		$(distdir)/examples/drmaaTestSuite/CVS \
		$(distdir)/examples/drmaaTestSuite/org/CVS \
		$(distdir)/examples/drmaaTestSuite/org/ggf/CVS \
		$(distdir)/examples/drmaaTestSuite/org/ggf/drmaa/CVS \
		$(distdir)/examples/drmaaTestSuite/org/ggf/drmaa/testSuite/CVS \
		$(distdir)/examples/workflow/CVS \
		$(distdir)/examples/dagman/CVS \
		$(distdir)/examples/mpi/CVS \
		$(distdir)/examples/jsdl/CVS

	@cp -r LICENSE NOTICE $(distdir)
