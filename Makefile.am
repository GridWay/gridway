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
PREFIX = @prefix@

if GW_GLOBUS_DIRS
GW_ETC_DIR   = etc/gridway
GW_VAR_DIR   = var/gridway
GW_DOC_DIR   = share/gridway/doc
GW_SHARE_DIR = share/gridway
GW_TEST_DIR  = test/gridway
GW_XML_DIR = share/gridway/xml_schema
else
GW_ETC_DIR   = etc
GW_VAR_DIR   = var
GW_DOC_DIR   = share/doc
GW_SHARE_DIR = share
GW_TEST_DIR  = test
GW_XML_DIR = xml_schema
endif


if DOC
SUBDIRS += doc/docbook/execution/gridway
if DRMAA
SUBDIRS += doc
endif
endif

if DRMAA_RUBY
SUBDIRS += src/drmaa/ruby
endif

FLAGS=$(DB4_INCLUDE_PATH)

install-exec-hook:
	@mkdir -p $(PREFIX)/$(GW_SHARE_DIR)
	@cp LICENSE NOTICE $(PREFIX)/$(GW_SHARE_DIR)	
if DOC
	@rm -rf $(PREFIX)/$(GW_DOC_DIR)
	@mkdir -p $(PREFIX)/$(GW_DOC_DIR)
if DOCHTMLC
	@cp -r doc/docbook/execution/gridway $(PREFIX)/$(GW_DOC_DIR)
	@find $(PREFIX)/$(GW_DOC_DIR) -depth -name '*.xml' -exec rm -rf '{}' \;
	@find $(PREFIX)/$(GW_DOC_DIR) -depth -name 'Makefile*' -exec rm -rf '{}' \;
	@find $(PREFIX)/$(GW_DOC_DIR) -depth -name '*.fo' -exec rm -rf '{}' \;
	@find $(PREFIX)/$(GW_DOC_DIR) -depth -name 'target.db' -exec rm -rf '{}' \;
	@find $(PREFIX)/$(GW_DOC_DIR) -depth -name 'images' -exec rm -f '{}' \;

	@ln -s $(PREFIX)/$(GW_DOC_DIR)/gridway/admin $(PREFIX)/$(GW_DOC_DIR)/gridway/admin/admin
	@ln -s $(PREFIX)/$(GW_DOC_DIR)/gridway/user $(PREFIX)/$(GW_DOC_DIR)/gridway/user/user
	@ln -s $(PREFIX)/$(GW_DOC_DIR)/gridway/developer $(PREFIX)/$(GW_DOC_DIR)/gridway/developer/developer
endif

if DRMAA
if DOXYGENC
	@cp -r doc/drmaa_c $(PREFIX)/$(GW_DOC_DIR)
endif
if JAVADOCC
	@cp -r doc/drmaa_java $(PREFIX)/$(GW_DOC_DIR)
endif		
endif
endif

	@rm -rf $(PREFIX)/$(GW_SHARE_DIR)/examples
	@mkdir -p $(PREFIX)/$(GW_SHARE_DIR)
	@cp -r examples $(PREFIX)/$(GW_SHARE_DIR)/examples

if TESTS
	@rm -rf $(PREFIX)/$(GW_TEST_DIR)
	@mkdir -p $(PREFIX)/$(GW_TEST_DIR)
	@cp -r src/test/gwtest src/test/TEST.pl \
	       src/test/jt \
	       $(PREFIX)/$(GW_TEST_DIR)
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

	@mv $(PREFIX)/bin/gw_im_mad_static.sh $(PREFIX)/bin/gw_im_mad_static
if GRIDFTP
	@mv $(PREFIX)/bin/gw_tm_mad_ftp.sh $(PREFIX)/bin/gw_tm_mad_ftp
endif

# ----------------------------
# -- Script renaming - WS
# ----------------------------

if WS
	@mv $(PREFIX)/bin/gw_em_mad_ws.sh $(PREFIX)/bin/gw_em_mad_ws
	@mv $(PREFIX)/bin/gw_im_mad_mds4.sh $(PREFIX)/bin/gw_im_mad_mds4
	@mv $(PREFIX)/bin/gw_im_mad_mds4_thr.sh $(PREFIX)/bin/gw_im_mad_mds4_thr
endif

# ----------------------------
# -- Script renaming - PREWS
# ----------------------------

if PREWS
	@mv $(PREFIX)/bin/gw_im_mad_mds2.sh $(PREFIX)/bin/gw_im_mad_mds2
	@mv $(PREFIX)/bin/gw_im_mad_mds2_glue.sh $(PREFIX)/bin/gw_im_mad_mds2_glue
	@mv $(PREFIX)/bin/gw_tm_mad_dummy.sh $(PREFIX)/bin/gw_tm_mad_dummy
	@mv $(PREFIX)/bin/gw_em_mad_prews.sh $(PREFIX)/bin/gw_em_mad_prews
endif

# ----------------------------
# -- Script renaming - NORDUGRID
# ----------------------------

if NORDUGRID
	@mv $(PREFIX)/bin/gw_em_mad_nordugrid.sh $(PREFIX)/bin/gw_em_mad_nordugrid
	@mv $(PREFIX)/bin/gw_tm_mad_nordugrid.sh $(PREFIX)/bin/gw_tm_mad_nordugrid
	@mkdir -p $(PREFIX)/libexec/python
	@mv $(PREFIX)/bin/gw_em_mad_nordugrid.py $(PREFIX)/libexec/python/gw_em_mad_nordugrid.py
	@mv $(PREFIX)/bin/gw_tm_mad_nordugrid.py $(PREFIX)/libexec/python/gw_tm_mad_nordugrid.py
	@mv $(PREFIX)/bin/em_nordugrid.py $(PREFIX)/libexec/python/em_nordugrid.py
	@mv $(PREFIX)/bin/tm_nordugrid.py $(PREFIX)/libexec/python/tm_nordugrid.py
endif

# ----------------------------
# -- Script renaming - LDAP
# ----------------------------
if LDAP
	@mkdir -p $(PREFIX)/libexec/perl
	@mv $(PREFIX)/bin/gw_im_mad_egee_ldap.sh $(PREFIX)/bin/gw_im_mad_egee_ldap
	@mv $(PREFIX)/bin/gw_im_mad_egee_ldap.pl $(PREFIX)/libexec/perl/gw_im_mad_egee_ldap.pl
endif

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
	mkdir -p $(PREFIX)/$(GW_VAR_DIR)/acct
	mkdir -p $(PREFIX)/$(GW_ETC_DIR)
	mkdir -p $(PREFIX)/$(GW_XML_DIR)
	cp -r $(top_srcdir)/etc/im_examples $(PREFIX)/$(GW_ETC_DIR)/
	cp $(top_srcdir)/etc/gwd.conf \
	   $(top_srcdir)/etc/job_template.default \
	   $(top_srcdir)/etc/sched.conf \
	   $(top_srcdir)/etc/gwrc \
	   $(PREFIX)/$(GW_ETC_DIR)/
	cp $(top_srcdir)/xml_schema/gridway.xsd $(PREFIX)/$(GW_XML_DIR)

# ----------------------------
#   Dagman Examples
# ----------------------------
	@ln -s A.jt $(PREFIX)/$(GW_SHARE_DIR)/examples/dagman/B.jt
	@ln -s A.jt $(PREFIX)/$(GW_SHARE_DIR)/examples/dagman/C.jt
	@ln -s A.jt $(PREFIX)/$(GW_SHARE_DIR)/examples/dagman/D.jt
	@ln -s A.jt $(PREFIX)/$(GW_SHARE_DIR)/examples/dagman/E.jt
	@ln -s A.jt $(PREFIX)/$(GW_SHARE_DIR)/examples/dagman/F.jt
	@ln -s A.jt $(PREFIX)/$(GW_SHARE_DIR)/examples/dagman/G.jt
	@ln -s A.jt $(PREFIX)/$(GW_SHARE_DIR)/examples/dagman/H.jt
	@ln -s A.jt $(PREFIX)/$(GW_SHARE_DIR)/examples/dagman/I.jt
	@ln -s A.jt $(PREFIX)/$(GW_SHARE_DIR)/examples/dagman/J.jt
	@ln -s A.jt $(PREFIX)/$(GW_SHARE_DIR)/examples/dagman/K.jt

# ----------------------------
#    remove svn DIRS
# ----------------------------
	@find $(PREFIX) -depth -name .svn -exec rm -rf '{}' \;

# ----------------------------		
# Copy wrapper, monitor & gw_mad_common.sh
# ----------------------------

	@mkdir -p $(PREFIX)/libexec
	@cp $(top_srcdir)/scripts/wrapper.sh $(PREFIX)/libexec/gw_wrapper.sh
	@cp $(top_srcdir)/scripts/monitor.sh $(PREFIX)/libexec/gw_monitor.sh
	@mkdir -p $(PREFIX)/bin
	@cp $(top_srcdir)/scripts/gw_mad_common.sh $(PREFIX)/bin

uninstall-hook:
	@rm -f $(PREFIX)/bin/gwdagman
	@rm $(PREFIX)/bin/gw_im_mad_mds2
	@rm $(PREFIX)/bin/gw_im_mad_mds4
	@rm $(PREFIX)/bin/gw_em_mad_ws
	@rm $(PREFIX)/bin/gw_im_mad_mds2_glue
if GRIDFTP
	@rm $(PREFIX)/bin/gw_tm_mad_ftp
endif
	@rm $(PREFIX)/bin/gw_tm_mad_rft
	@rm $(PREFIX)/bin/gw_em_mad_prews
	@rm $(PREFIX)/bin/gw_im_mad_static
	@rm $(PREFIX)/bin/gw_tm_mad_dummy
	@rm $(PREFIX)/bin/jsdl2gw
	@rm -rf $(PREFIX)/lib/ruby/
	@rm -rf $(PREFIX)/$(GW_VAR_DIR)
if NORDUGRID
	@rm -rf $(PREFIX)/libexec/python
	@rm $(PREFIX)/bin/gw_em_mad_nordugrid
	@rm $(PREFIX)/bin/gw_tm_mad_nordugrid
endif
if LDAP
	@rm -rf $(PREFIX)/libexec/perl
	@rm $(PREFIX)/bin/gw_im_mad_egee_ldap
endif
	@rm -rf $(PREFIX)/$(GW_XML_DIR)

# *****************************************
# Dist specifics (see also src/Makefile.am)
# *****************************************

dist-hook:	
	@rm -rf $(distdir)/src/$(DRMAA_SRC_DIR)/org/ggf/drmaa/.dirstamp
	@rm -rf $(distdir)/src/$(DRMAA_SRC_DIR)/org/ggf/drmaa/.libs
	@rm -f $(distdir)/src/$(DRMAA_SRC_DIR)/org/ggf/drmaa/DrmaaJNI.lo
	@rm -f $(distdir)/src/$(DRMAA_SRC_DIR)/org/ggf/drmaa/DrmaaJNI.o
	@mkdir -p $(distdir)/doc/docbook
	@cp -r doc/images $(distdir)/doc
	@cp -r doc/drmaa_c $(distdir)/doc
	@cp -r doc/docbook $(distdir)/doc/
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
	@cp -r LICENSE NOTICE $(distdir)

# ----------------------------
#    remove CVS DIRS
# ----------------------------
	@find $(distdir) -depth -name .svn -exec rm -rf '{}' \;
