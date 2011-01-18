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

SUBDIRS = src
JAVA_HOME = @java_home@
PREFIX = @prefix@

GW_ETC_DIR   = etc
GW_VAR_DIR   = var
GW_SHARE_DIR = share
GW_TEST_DIR  = test
GW_XML_DIR = xml_schema

if DRMAA_RUBY
SUBDIRS += src/drmaa/ruby
endif

FLAGS=$(DB4_INCLUDE_PATH)

install-exec-hook:
	@mkdir -p $(PREFIX)/$(GW_SHARE_DIR)
	@cp LICENSE README $(PREFIX)/$(GW_SHARE_DIR)

	@rm -rf $(PREFIX)/$(GW_SHARE_DIR)/examples
	@mkdir -p $(PREFIX)/$(GW_SHARE_DIR)
	@cp -r examples $(PREFIX)/$(GW_SHARE_DIR)/examples

if TESTS
	@rm -rf $(PREFIX)/$(GW_TEST_DIR)
	@mkdir -p $(PREFIX)/$(GW_TEST_DIR)
	@cp -r src/test/gwtest src/test/TEST.pl src/test/jt $(PREFIX)/$(GW_TEST_DIR)
endif

# ----------------------------
# -- Script renaming - Commons
# ----------------------------

# -- Dagman Ruby Scripts
	@mkdir -p $(PREFIX)/libexec/ruby/dagman
	@mv $(PREFIX)/bin/gw_dagman.rb $(PREFIX)/bin/gwdagman
	@mv $(PREFIX)/bin/node.rb $(PREFIX)/libexec/ruby/dagman/node.rb
	@mv $(PREFIX)/bin/parse.rb $(PREFIX)/libexec/ruby/dagman/parse.rb
	@mv $(PREFIX)/bin/runner.rb $(PREFIX)/libexec/ruby/dagman/runner.rb
	@mv $(PREFIX)/bin/gridway.rb $(PREFIX)/libexec/ruby/dagman/gridway.rb

# ----------------------------
# -- Script renaming - JSDL
# ----------------------------

if JSDL
	@mv $(PREFIX)/bin/jsdl2gw.sh $(PREFIX)/bin/jsdl2gw
endif

# ****************************
# DIR LAYOUT
# ****************************

install-data-hook:
	@mkdir -p $(PREFIX)/$(GW_VAR_DIR)/acct
	@mkdir -p $(PREFIX)/$(GW_ETC_DIR)
	@mkdir -p $(PREFIX)/$(GW_XML_DIR)
	@cp -rf $(top_srcdir)/etc/im_examples $(PREFIX)/$(GW_ETC_DIR)/
	@cp $(top_srcdir)/etc/gwd.conf $(top_srcdir)/etc/job_template.default \
$(top_srcdir)/etc/sched.conf $(top_srcdir)/etc/gwrc $(PREFIX)/$(GW_ETC_DIR)/
	@cp $(top_srcdir)/xml_schema/gridway.xsd $(PREFIX)/$(GW_XML_DIR)

# ----------------------------
#   Dagman Examples
# ----------------------------
	@ln -sf A.jt $(PREFIX)/$(GW_SHARE_DIR)/examples/dagman/B.jt
	@ln -sf A.jt $(PREFIX)/$(GW_SHARE_DIR)/examples/dagman/C.jt
	@ln -sf A.jt $(PREFIX)/$(GW_SHARE_DIR)/examples/dagman/D.jt
	@ln -sf A.jt $(PREFIX)/$(GW_SHARE_DIR)/examples/dagman/E.jt
	@ln -sf A.jt $(PREFIX)/$(GW_SHARE_DIR)/examples/dagman/F.jt
	@ln -sf A.jt $(PREFIX)/$(GW_SHARE_DIR)/examples/dagman/G.jt
	@ln -sf A.jt $(PREFIX)/$(GW_SHARE_DIR)/examples/dagman/H.jt
	@ln -sf A.jt $(PREFIX)/$(GW_SHARE_DIR)/examples/dagman/I.jt
	@ln -sf A.jt $(PREFIX)/$(GW_SHARE_DIR)/examples/dagman/J.jt
	@ln -sf A.jt $(PREFIX)/$(GW_SHARE_DIR)/examples/dagman/K.jt

# ----------------------------
#    remove svn DIRS
# ----------------------------
	@find $(PREFIX) -depth -name .svn -exec rm -rf '{}' \;

# ----------------------------		
# Copy wrapper, monitor, gw_mad_common.sh & static IM MAD
# ----------------------------

	@mkdir -p $(PREFIX)/libexec
	@cp $(top_srcdir)/scripts/wrapper.sh $(PREFIX)/libexec/gw_wrapper.sh
	@cp $(top_srcdir)/scripts/monitor.sh $(PREFIX)/libexec/gw_monitor.sh
	@mkdir -p $(PREFIX)/bin
	@cp $(top_srcdir)/scripts/gw_mad_common.sh $(PREFIX)/bin
	@cp $(top_srcdir)/im_mad/gw_im_mad_common.sh $(PREFIX)/bin
	@cp $(top_srcdir)/im_mad/gw_im_mad_static.sh $(PREFIX)/bin/gw_im_mad_static

uninstall-hook:
	@rm -rf $(PREFIX)/libexec/perl
	@rm -rf $(PREFIX)/bin
	@rm -rf $(PREFIX)/lib/ruby/
	@rm -rf $(PREFIX)/$(GW_VAR_DIR)
	@rm -rf $(PREFIX)/libexec/python
	@rm -rf $(PREFIX)/$(GW_XML_DIR)

# *****************************************
# Dist specifics (see also src/Makefile.am)
# *****************************************

dist-hook:	
	@rm -rf $(distdir)/src/$(DRMAA_SRC_DIR)/org/ggf/drmaa/.dirstamp
	@rm -rf $(distdir)/src/$(DRMAA_SRC_DIR)/org/ggf/drmaa/.libs
	@rm -f $(distdir)/src/$(DRMAA_SRC_DIR)/org/ggf/drmaa/DrmaaJNI.lo
	@rm -f $(distdir)/src/$(DRMAA_SRC_DIR)/org/ggf/drmaa/DrmaaJNI.o
	@cp -r examples $(distdir)
	@cp -r etc $(distdir)
	@cp -r src/test/jt $(distdir)/src/test
	@cp -r src/test/TEST.pl $(distdir)/src/test
	@mkdir -p $(distdir)/src/drmaa/ruby
	@cp src/drmaa/ruby/extconf.rb $(distdir)/src/drmaa/ruby
	@cp src/drmaa/ruby/drmaa_r.i $(distdir)/src/drmaa/ruby
	@cp src/drmaa/ruby/drmaa_r_wrap.c $(distdir)/src/drmaa/ruby
	@cp -r scripts $(distdir)
	@cp pkg_data_src.gpt  $(distdir)/pkg_data_src.gpt
	@cp -r LICENSE README $(distdir)

# ----------------------------
#    remove CVS DIRS
# ----------------------------
	@find $(distdir) -depth -name .svn -exec rm -rf '{}' \;
