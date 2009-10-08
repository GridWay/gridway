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
GW_DOC_DIR   = share/gridway/doc
GW_SHARE_DIR = share/gridway
GW_TEST_DIR  = test/gridway
else
GW_ETC_DIR   = etc
GW_VAR_DIR   = var
GW_DOC_DIR   = share/doc
GW_SHARE_DIR = share
GW_TEST_DIR  = test
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
	@mkdir -p @prefix@/$(GW_SHARE_DIR)
	@cp -r LICENSE NOTICE @prefix@/$(GW_SHARE_DIR)	
if DOC
	@rm -rf @prefix@/$(GW_DOC_DIR)
	@mkdir -p @prefix@/$(GW_DOC_DIR)
if DOCHTMLC
	@cp -r doc/docbook/execution/gridway \
           @prefix@/$(GW_DOC_DIR)
	@find @prefix@/$(GW_DOC_DIR) -depth -name '*.xml' -exec rm -rf '{}' \;
	@find @prefix@/$(GW_DOC_DIR) -depth -name 'Makefile*' -exec rm -rf '{}' \;
	@find @prefix@/$(GW_DOC_DIR) -depth -name '*.fo' -exec rm -rf '{}' \;
	@find @prefix@/$(GW_DOC_DIR) -depth -name 'target.db' -exec rm -rf '{}' \;
	@find @prefix@/$(GW_DOC_DIR) -depth -name 'images' -exec rm -f '{}' \;

	@ln -s @prefix@/$(GW_DOC_DIR)/gridway/admin @prefix@/$(GW_DOC_DIR)/gridway/admin/admin
	@ln -s @prefix@/$(GW_DOC_DIR)/gridway/user @prefix@/$(GW_DOC_DIR)/gridway/user/user
	@ln -s @prefix@/$(GW_DOC_DIR)/gridway/developer @prefix@/$(GW_DOC_DIR)/gridway/developer/developer
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
	@mv @prefix@/bin/gw_im_mad_mds4_thr.sh @prefix@/bin/gw_im_mad_mds4_thr
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
# -- Script renaming - NORDUGRID
# ----------------------------

if NORDUGRID
	@mv @prefix@/bin/gw_em_mad_nordugrid.sh @prefix@/bin/gw_em_mad_nordugrid
	@mv @prefix@/bin/gw_tm_mad_nordugrid.sh @prefix@/bin/gw_tm_mad_nordugrid
	@mkdir -p @prefix@/libexec/python
	@mv @prefix@/bin/gw_em_mad_nordugrid.py @prefix@/libexec/python/gw_em_mad_nordugrid.py
	@mv @prefix@/bin/gw_tm_mad_nordugrid.py @prefix@/libexec/python/gw_tm_mad_nordugrid.py
	@mv @prefix@/bin/em_nordugrid.py @prefix@/libexec/python/em_nordugrid.py
	@mv @prefix@/bin/tm_nordugrid.py @prefix@/libexec/python/tm_nordugrid.py
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
	@find @prefix@ -depth -name CVS -exec rm -rf '{}' \;

# ----------------------------		
# Copy wrapper, monitor & gw_mad_common.sh
# ----------------------------

		@mkdir -p @prefix@/libexec
		@cp $(top_srcdir)/scripts/wrapper.sh @prefix@/libexec/gw_wrapper.sh
		@cp $(top_srcdir)/scripts/monitor.sh @prefix@/libexec/gw_monitor.sh
		@mkdir -p @prefix@/bin
		@cp $(top_srcdir)/scripts/gw_mad_common.sh @prefix@/bin

# TODO: Uninstall nordugrid files

uninstall-hook:
	@rm -f @prefix@/bin/gwdagman
	@rm @prefix@/bin/gw_im_mad_mds2
	@rm @prefix@/bin/gw_im_mad_mds4
	@rm @prefix@/bin/gw_em_mad_ws
	@rm @prefix@/bin/gw_im_mad_mds2_glue
if GRIDFTP
	@rm @prefix@/bin/gw_tm_mad_ftp
endif
	@rm @prefix@/bin/gw_tm_mad_rft
	@rm @prefix@/bin/gw_em_mad_prews
	@rm @prefix@/bin/gw_im_mad_static
	@rm @prefix@/bin/gw_tm_mad_dummy
	@rm @prefix@/bin/jsdl2gw
	@rm -rf @prefix@/lib/ruby/
	@rm -rf @prefix@/$(GW_VAR_DIR)

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
	@find $(distdir) -depth -name CVS -exec rm -rf '{}' \;
	
