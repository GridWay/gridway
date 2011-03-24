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

GW_ETC_DIR = etc
GW_VAR_DIR = var
GW_SHARE_DIR = share
GW_TEST_DIR = test
GW_XML_DIR = xml_schema
GW_SRC_DIR = src

EXTRA_DIST = etc scripts examples xml_schema/gridway.xsd LICENSE README

if DRMAA_RUBY
EXTRA_DIST+= src/drmaa/ruby
endif

if TESTS
EXTRA_DIST+= src/test/gw_test.c src/test/TEST.pl src/test/jt
endif

FLAGS=$(DB4_INCLUDE_PATH)

install-exec-hook:
if TESTS
	$(MKDIR_P) $(DESTDIR)$(GW_TEST_DIR)
	${mkinstalldirs} $(DESTDIR)$(GW_TEST_DIR)
	cp $(srcdir)/src/test/TEST.pl $(DESTDIR)$(GW_TEST_DIR)
	cp -R $(srcdir)/src/test/jt $(DESTDIR)$(GW_TEST_DIR)

	find $(DESTDIR)$(GW_TEST_DIR) -depth -name .svn -exec rm -rf '{}' \;
endif

# ----------------------------
# -- Script renaming - Commons
# ----------------------------

# -- Dagman Ruby Scripts
	$(MKDIR_P) $(DESTDIR)$(libexecdir)/ruby/dagman
	${mkinstalldirs} $(DESTDIR)$(libexecdir)/ruby/dagman
	mv $(DESTDIR)$(bindir)/gw_dagman.rb $(DESTDIR)$(bindir)/gwdagman
	mv $(DESTDIR)$(bindir)/node.rb $(DESTDIR)$(libexecdir)/ruby/dagman/node.rb
	mv $(DESTDIR)$(bindir)/parse.rb $(DESTDIR)$(libexecdir)/ruby/dagman/parse.rb
	mv $(DESTDIR)$(bindir)/runner.rb $(DESTDIR)$(libexecdir)/ruby/dagman/runner.rb
	mv $(DESTDIR)$(bindir)/gridway.rb $(DESTDIR)$(libexecdir)/ruby/dagman/gridway.rb

# ----------------------------
# -- Script renaming - JSDL
# ----------------------------

if JSDL
	mv $(DESTDIR)$(bindir)/jsdl2gw.sh $(DESTDIR)$(bindir)/jsdl2gw
endif

# ****************************
# DIR LAYOUT
# ****************************

install-data-hook:
	$(MKDIR_P) $(DESTDIR)$(GW_VAR_DIR)/acct
	$(MKDIR_P) $(DESTDIR)$(GW_ETC_DIR)
	$(MKDIR_P) $(DESTDIR)$(GW_XML_DIR)
	$(MKDIR_P) $(DESTDIR)$(GW_SHARE_DIR)
	${mkinstalldirs} $(DESTDIR)$(GW_VAR_DIR)/acct $(DESTDIR)$(GW_ETC_DIR) \
$(DESTDIR)$(GW_XML_DIR) $(DESTDIR)$(GW_SHARE_DIR)
	$(INSTALL_DATA) $(srcdir)/etc/gwd.conf $(DESTDIR)$(GW_ETC_DIR)
	$(INSTALL_DATA) $(srcdir)/etc/job_template.default $(DESTDIR)$(GW_ETC_DIR)
	$(INSTALL_DATA) $(srcdir)/etc/sched.conf $(DESTDIR)$(GW_ETC_DIR)
	$(INSTALL_DATA) $(srcdir)/etc/gwrc $(DESTDIR)$(GW_ETC_DIR)
	$(INSTALL_DATA) $(srcdir)/xml_schema/gridway.xsd $(DESTDIR)$(GW_XML_DIR)
	$(INSTALL_DATA) $(srcdir)/LICENSE $(DESTDIR)$(GW_SHARE_DIR)
	$(INSTALL_DATA) $(srcdir)/README $(DESTDIR)$(GW_SHARE_DIR)

# ----------------------------
#    remove svn DIRS
# ----------------------------
#	find $(DESTDIR). -depth -name .svn -exec rm -rf '{}' \;

# ----------------------------		
# Copy wrapper, monitor, gw_mad_common.sh & static IM MAD
# ----------------------------

	$(INSTALL_DATA) $(top_srcdir)/scripts/wrapper.sh $(DESTDIR)$(libexecdir)/gw_wrapper.sh
	$(INSTALL_DATA) $(top_srcdir)/scripts/monitor.sh $(DESTDIR)$(libexecdir)/gw_monitor.sh
	$(INSTALL_DATA) $(top_srcdir)/scripts/gw_mad_common.sh $(DESTDIR)$(bindir)
	$(INSTALL_DATA) $(top_srcdir)/src/im_mad/gw_im_mad_common.sh $(DESTDIR)$(bindir)
	$(INSTALL_DATA) $(top_srcdir)/src/im_mad/gw_im_mad_static.sh $(DESTDIR)$(bindir)/gw_im_mad_static

uninstall-hook:
	rm -f $(DESTDIR)$(bindir)/gw_im_mad_common.sh $(DESTDIR)$(bindir)/gw_im_mad_static \
$(DESTDIR)$(bindir)/gw_mad_common.sh
	rm -f $(DESTDIR)$(bindir)/gwdagman
	rm -f $(DESTDIR)$(libexecdir)/ruby/dagman/node.rb \
$(DESTDIR)$(libexecdir)/ruby/dagman/parse.rb $(DESTDIR)$(libexecdir)/ruby/dagman/runner.rb \
$(DESTDIR)$(libexecdir)/ruby/dagman/gridway.rb
	rm -f $(DESTDIR)$(libexecdir)/gw_wrapper.sh $(DESTDIR)$(libexecdir)/gw_monitor.sh
	rm -f $(DESTDIR)$(GW_ETC_DIR)/gwd.conf $(DESTDIR)$(GW_ETC_DIR)/gwrc \
$(DESTDIR)$(GW_ETC_DIR)/job_template.default $(DESTDIR)$(GW_ETC_DIR)/sched.conf
	rm -f $(DESTDIR)$(GW_SHARE_DIR)/LICENSE $(DESTDIR)$(GW_SHARE_DIR)/README
	rm -f $(DESTDIR)$(GW_XML_DIR)/gridway.xsd

# *****************************************
# Dist specifics (see also src/Makefile.am)
# *****************************************

dist-hook:
	rm -f $(distdir)/src/$(DRMAA_SRC_DIR)/org/ggf/drmaa/.dirstamp
	rm -rf $(distdir)/src/$(DRMAA_SRC_DIR)/org/ggf/drmaa/.libs
	rm -f $(distdir)/src/$(DRMAA_SRC_DIR)/org/ggf/drmaa/DrmaaJNI.lo
	rm -f $(distdir)/src/$(DRMAA_SRC_DIR)/org/ggf/drmaa/DrmaaJNI.o
#	cp pkgdata/pkg_data_src.gpt  $(distdir)/pkg_data_src.gpt

# ----------------------------
#    remove SVN DIRS
# ----------------------------
	find $(distdir) -depth -name .svn -exec rm -rf '{}' \;
