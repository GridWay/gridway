# --------------------------------------------------------------------------
# Copyright 2002-2013, GridWay Project Leads (GridWay.org)          
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

vardir = @localstatedir@
testdir = $(PREFIX)/test
xmldir = $(PREFIX)/xml_schema

EXTRA_DIST = scripts/gw_mad_common.sh scripts/monitor.sh scripts/wrapper.sh \
scripts/ssh_wrapper.sh examples xml_schema/gridway.xsd src/test/TEST.pl src/test/jt \
src/drmaa/ruby/drmaa_r.i src/drmaa/ruby/drmaa_r_wrap.c src/drmaa/ruby/extconf.rb

FLAGS=$(DB4_INCLUDE_PATH)
 
dist_sysconf_DATA = etc/gwd.conf etc/sched.conf etc/gwrc etc/job_template.default
dist_data_DATA = LICENSE README NOTICE RELEASE_NOTES

install-exec-hook:
if TESTS
	cp $(srcdir)/src/test/TEST.pl $(DESTDIR)$(testdir)
	cp -R $(srcdir)/src/test/jt $(DESTDIR)$(testdir)

	find $(DESTDIR)$(testdir) -depth -name .svn -exec rm -rf '{}' \;
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
	$(MKDIR_P) $(DESTDIR)$(vardir)/acct
	$(MKDIR_P) $(DESTDIR)$(xmldir)
	${mkinstalldirs} $(DESTDIR)$(vardir)/acct $(DESTDIR)$(xmldir)
	$(INSTALL_DATA) $(srcdir)/xml_schema/gridway.xsd $(DESTDIR)$(xmldir)

# ----------------------------
#    remove svn DIRS
# ----------------------------
#	find $(DESTDIR). -depth -name .svn -exec rm -rf '{}' \;

# ----------------------------		
# Copy wrapper, monitor, gw_mad_common.sh & static IM MAD
# ----------------------------

	$(INSTALL_DATA) $(top_srcdir)/scripts/wrapper.sh $(DESTDIR)$(libexecdir)/gw_wrapper.sh
	$(INSTALL_DATA) $(top_srcdir)/scripts/monitor.sh $(DESTDIR)$(libexecdir)/gw_monitor.sh
	$(INSTALL_DATA) $(top_srcdir)/scripts/ssh_wrapper.sh $(DESTDIR)$(libexecdir)/ssh_wrapper.sh
	$(INSTALL_DATA) $(top_srcdir)/scripts/gw_mad_common.sh $(DESTDIR)$(bindir)
	$(INSTALL_DATA) $(top_srcdir)/src/im_mad/gw_im_mad_common.sh $(DESTDIR)$(bindir)
	$(INSTALL_DATA) $(top_srcdir)/src/im_mad/gw_im_mad_static.sh $(DESTDIR)$(bindir)/gw_im_mad_static
	chmod a+x $(DESTDIR)$(bindir)/gw_im_mad_static

uninstall-hook:
	rm -f $(DESTDIR)$(bindir)/gw_im_mad_common.sh $(DESTDIR)$(bindir)/gw_im_mad_static \
$(DESTDIR)$(bindir)/gw_mad_common.sh
	rm -f $(DESTDIR)$(bindir)/gwdagman
	rm -f $(DESTDIR)$(libexecdir)/ruby/dagman/node.rb \
$(DESTDIR)$(libexecdir)/ruby/dagman/parse.rb $(DESTDIR)$(libexecdir)/ruby/dagman/runner.rb \
$(DESTDIR)$(libexecdir)/ruby/dagman/gridway.rb
	rm -f $(DESTDIR)$(libexecdir)/gw_wrapper.sh $(DESTDIR)$(libexecdir)/gw_monitor.sh \
$(DESTDIR)$(libexecdir)/ssh_wrapper.sh
	rm -f $(DESTDIR)$(xmldir)/gridway.xsd

# *****************************************
# Dist specifics (see also src/Makefile.am)
# *****************************************

dist-hook:
	rm -f $(distdir)/src/$(DRMAA_SRC_DIR)/org/ggf/drmaa/.dirstamp
	rm -rf $(distdir)/src/$(DRMAA_SRC_DIR)/org/ggf/drmaa/.libs
	rm -f $(distdir)/src/$(DRMAA_SRC_DIR)/org/ggf/drmaa/DrmaaJNI.lo
	rm -f $(distdir)/src/$(DRMAA_SRC_DIR)/org/ggf/drmaa/DrmaaJNI.o
	rm -f $(distdir)/src/$(DRMAA_SRC_DIR)/org/ggf/drmaa/drmaa1.0/.dirstamp
	rm -rf $(distdir)/src/$(DRMAA_SRC_DIR)/org/ggf/drmaa/drmaa1.0/.libs
	rm -f $(distdir)/src/$(DRMAA_SRC_DIR)/org/ggf/drmaa/drmaa1.0/DrmaaJNI.lo
	rm -f $(distdir)/src/$(DRMAA_SRC_DIR)/org/ggf/drmaa/drmaa1.0/DrmaaJNI.o

# ----------------------------
#    remove SVN dirs
# ----------------------------
	find $(distdir) -depth -name .svn -exec rm -rf '{}' \;
