# --------------------------------------------------------------------------
# Copyright 2002-2012, GridWay Project Leads (GridWay.org)          
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

# -----------------------------
# Automake Options
# -----------------------------

AUTOMAKE_OPTIONS = subdir-objects no-dependencies
AM_YFLAGS = -d

LINK_PROG    = $(CCLD)
COMPILE_PROG = $(CC)

LINK_OPTS = $(AM_CFLAGS) $(CFLAGS) $(AM_LDFLAGS) $(LDFLAGS) -o $@

COMPILE_OPTS = $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) \
	$(CPPFLAGS) $(AM_CFLAGS) $(CFLAGS)

LINK = $(LINK_PROG) $(LINK_OPTS)
COMPILE = $(COMPILE_PROG) $(COMPILE_OPTS)

# -----------------------------
# Directory definitions
# -----------------------------

JOB     = $(srcdir)/job
HOST    = $(srcdir)/host
ARRAY   = $(srcdir)/array
COMMON  = $(srcdir)/common
SCHED   = $(srcdir)/scheduler
EM_MAD  = $(srcdir)/em_mad
TM_MAD  = $(srcdir)/tm_mad
IM_MAD  = $(srcdir)/im_mad
CMDS    = $(srcdir)/cmds
GWD     = $(srcdir)/gwd
CLIENT  = $(srcdir)/client

EM = $(srcdir)/em
TM = $(srcdir)/tm
IM = $(srcdir)/im
DM = $(srcdir)/dm
RM = $(srcdir)/rm
UM = $(srcdir)/um
DB = $(srcdir)/acct

DRMAA1_SRC_DIR=drmaa/drmaa1.0
DRMAA_SRC_DIR=drmaa

SUBDIRS = .

if DRMAA_RUBY
SUBDIRS += drmaa/ruby
endif

DRMAA = $(srcdir)/drmaa

DRMAA_PACK = $(srcdir)/$(DRMAA_SRC_DIR)/org/ggf/drmaa
DRMAA1_PACK = $(srcdir)/$(DRMAA1_SRC_DIR)/org/ggf/drmaa

SSH = $(srcdir)/ssh

JAVA_HOME = @java_home@

# --------------------------------
# Object Dependencies
# --------------------------------

COMMON_SRC = $(COMMON)/gw_action.c \
        $(COMMON)/gw_common.c \
        $(COMMON)/gw_file_parser.c \
        $(COMMON)/gw_log.c \
        $(COMMON)/gw_template.c \
        $(COMMON)/gw_template_parser.l

JOB_SRC  =  $(JOB)/gw_history.c \
        $(JOB)/gw_job.c \
        $(JOB)/gw_job_environment.c \
        $(JOB)/gw_job_pool.c \
        $(JOB)/gw_job_recover.c \
        $(JOB)/gw_job_template.c \
        $(JOB)/gw_job_template_var_parser.l \
        $(JOB)/gw_xfr_files.c

HOST_SRC  = $(HOST)/gw_host.c \
        $(HOST)/gw_host_var.c \
        $(HOST)/gw_host_pool.c \
        $(HOST)/gw_host_rank_syntax.y \
        $(HOST)/gw_host_reqs_syntax.y \
        $(HOST)/gw_host_update_syntax.y \
        $(HOST)/gw_host_attr_parser.l

ARRAY_SRC = $(ARRAY)/gw_array.c \
        $(ARRAY)/gw_array_pool.c

CLIENT_SRC = $(CLIENT)/gw_client_job_match.c \
        $(CLIENT)/gw_client_submit.c \
        $(CLIENT)/gw_client_host_status.c \
        $(CLIENT)/gw_client_job_status.c \
        $(CLIENT)/gw_client_user_status.c \
        $(CLIENT)/gw_client_init.c \
        $(CLIENT)/gw_client_signal.c \
        $(CLIENT)/gw_client_wait.c \
        $(HOST)/gw_host_rank_syntax.c \
        $(HOST)/gw_host_reqs_syntax.c \
        $(HOST)/gw_host_attr_parser.c \
        $(HOST)/gw_host_update_syntax.c \
        $(HOST)/gw_host_var.c

CLIENT_ACCT_SRC = $(CLIENT)/gw_client_acct.c

CMDS_SRC = $(CMDS)/gw_acct.c \
        $(CMDS)/gw_history.c \
        $(CMDS)/gw_kill.c \
        $(CMDS)/gw_submit.c \
        $(CMDS)/gw_wait.c \
        $(CMDS)/gw_cmds_common.c \
        $(CMDS)/gw_host.c \
        $(CMDS)/gw_ps.c \
        $(CMDS)/gw_user.c

CMDS_RB_SRC=$(CMDS)/gw_dagman.rb \
            $(CMDS)/dagman/node.rb \
            $(CMDS)/dagman/runner.rb \
            $(CMDS)/dagman/parse.rb \
	    $(CMDS)/dagman/gridway.rb

JSDL_SRC = $(CMDS)/GWJTElement.java \
        $(CMDS)/GWJT.java  \
        $(CMDS)/GWJTWriter.java  \
        $(CMDS)/JSDLElement.java  \
        $(CMDS)/JSDL.java  \
        $(CMDS)/JSDLReader.java  \
        $(CMDS)/JSDLParser.java  \
        $(CMDS)/JSDLTranslator.java

EXTRA_DIST = $(JSDL_SRC)

EM_SRC = $(EM)/gw_em_actions.c \
        $(EM)/gw_em.c \
        $(EM)/gw_em_listener.c \
        $(EM)/gw_em_mad.c \
        $(EM)/gw_em_rsl2.c \
        $(EM)/gw_em_rsl.c \
        $(EM)/gw_em_jdl.c \
        $(EM)/gw_em_jsdl.c \
        $(EM)/gw_em_state.c

TM_SRC = $(TM)/gw_tm_actions.c \
        $(TM)/gw_tm.c \
        $(TM)/gw_tm_checkpoint.c \
        $(TM)/gw_tm_epilog.c \
        $(TM)/gw_tm_listener.c \
        $(TM)/gw_tm_mad.c \
        $(TM)/gw_tm_prolog.c

IM_SRC = $(IM)/gw_im_actions.c \
        $(IM)/gw_im.c \
        $(IM)/gw_im_listener.c \
        $(IM)/gw_im_mad.c

DM_SRC = $(DM)/gw_dm_action_allocate.c \
        $(DM)/gw_dm_action_dispatch.c \
        $(DM)/gw_dm_action_hold_release.c \
        $(DM)/gw_dm_action_kill.c \
        $(DM)/gw_dm_action_reschedule.c \
        $(DM)/gw_dm_action_schedule.c \
        $(DM)/gw_dm_action_stop_resume.c \
        $(DM)/gw_dm_action_wait.c \
        $(DM)/gw_dm.c \
        $(DM)/gw_dm_listener.c \
        $(DM)/gw_dm_mad.c \
        $(DM)/gw_dm_state_epilogs.c \
        $(DM)/gw_dm_state_failed.c \
        $(DM)/gw_dm_state_migrate.c \
        $(DM)/gw_dm_state_pending.c \
        $(DM)/gw_dm_state_prologs.c \
        $(DM)/gw_dm_state_stopped.c \
        $(DM)/gw_dm_state_wrappers.c \
        $(DM)/gw_dm_state_zombie.c

RM_SRC = $(RM)/gw_rm.c \
        $(RM)/gw_rm_connection_list.c \
        $(RM)/gw_rm_hold_release.c \
        $(RM)/gw_rm_host_match.c \
        $(RM)/gw_rm_host_status.c \
        $(RM)/gw_rm_job_history.c \
        $(RM)/gw_rm_job_status.c \
        $(RM)/gw_rm_kill.c \
        $(RM)/gw_rm_reschedule.c \
        $(RM)/gw_rm_stop_resume.c \
        $(RM)/gw_rm_submit.c \
        $(RM)/gw_rm_user_status.c \
        $(RM)/gw_rm_wait.c

UM_SRC = $(UM)/gw_um.c \
        $(UM)/gw_user.c \
        $(UM)/gw_user_pool.c

SCHED_SRC = $(SCHED)/gw_scheduler_common.c \
        $(SCHED)/gw_scheduler_hosts.c \
        $(SCHED)/gw_scheduler_jobs.c \
        $(SCHED)/gw_scheduler_users.c

DB_SRC = $(DB)/gw_acct.c

GWD_SRC = $(GWD)/gw_conf.c \
        $(GWD)/gw_conf_parser.l \
        $(GWD)/gwd.c \
        $(GWD)/gw_sch_conf_syntax.y \
        $(GWD)/gw_sch_conf_parser.l \
        $(GWD)/gw_sch_conf.c        

DRMAA_SRC = $(COMMON)/gw_file_parser.c $(CLIENT_SRC) \
        $(DRMAA)/gw_drmaa.c $(DRMAA)/gw_drmaa_jt.c \
        $(COMMON)/gw_template_parser.l \
        $(COMMON)/gw_template.c $(COMMON)/gw_log.c

DRMAA2_SRC = $(COMMON)/gw_file_parser.c $(CLIENT_SRC) \
        $(DRMAA)/gw_drmaa2.c \
        $(COMMON)/gw_template_parser.l \
        $(COMMON)/gw_template.c $(COMMON)/gw_log.c

DRMAAJNI_SRC = $(DRMAA_PACK)/DrmaaJNI.c

DRMAA1JNI_SRC = $(DRMAA1_PACK)/DrmaaJNI.c

DRMAA_JAVA_SRC = $(DRMAA_PACK)/AlreadyActiveSessionException.java \
        $(DRMAA_PACK)/AuthorizationException.java \
        $(DRMAA_PACK)/ConflictingAttributeValuesException.java \
        $(DRMAA_PACK)/DefaultContactStringException.java \
        $(DRMAA_PACK)/DeniedByDrmException.java \
        $(DRMAA_PACK)/DrmaaException.java \
        $(DRMAA_PACK)/DrmaaJNI.java \
        $(DRMAA_PACK)/DrmCommunicationException.java \
        $(DRMAA_PACK)/DrmsExitException.java \
        $(DRMAA_PACK)/DrmsInitException.java \
        $(DRMAA_PACK)/ExitTimeoutException.java \
        $(DRMAA_PACK)/FileTransferMode.java \
        $(DRMAA_PACK)/HoldInconsistentStateException.java \
        $(DRMAA_PACK)/InconsistentStateException.java \
        $(DRMAA_PACK)/InternalException.java \
        $(DRMAA_PACK)/InvalidAttributeFormatException.java \
        $(DRMAA_PACK)/InvalidAttributeValueException.java \
        $(DRMAA_PACK)/InvalidContactStringException.java \
        $(DRMAA_PACK)/InvalidJobException.java \
        $(DRMAA_PACK)/InvalidJobTemplateException.java \
        $(DRMAA_PACK)/JobInfo.java \
        $(DRMAA_PACK)/JobTemplate.java \
        $(DRMAA_PACK)/NoActiveSessionException.java \
        $(DRMAA_PACK)/NoDefaultContactStringSelectedException.java \
        $(DRMAA_PACK)/PartialTimestampFormat.java \
        $(DRMAA_PACK)/PartialTimestamp.java \
        $(DRMAA_PACK)/ReleaseInconsistentStateException.java \
        $(DRMAA_PACK)/ResumeInconsistentStateException.java \
        $(DRMAA_PACK)/SessionFactory.java \
        $(DRMAA_PACK)/Session.java \
        $(DRMAA_PACK)/SuspendInconsistentStateException.java \
        $(DRMAA_PACK)/TryLaterException.java \
        $(DRMAA_PACK)/UnsupportedAttributeException.java \
        $(DRMAA_PACK)/Version.java \
        $(DRMAA_PACK)/DrmaaRuntimeException.java \
        $(DRMAA_PACK)/InvalidAttributeException.java \
        $(DRMAA_PACK)/JobTemplateImpl.java \
        $(DRMAA_PACK)/JobInfoImpl.java \
        $(DRMAA_PACK)/NoResourceUsageException.java \
        $(DRMAA_PACK)/SessionException.java \
        $(DRMAA_PACK)/SessionImpl.java \
        $(DRMAA_PACK)/SessionFactoryImpl.java

DRMAA1_JAVA_SRC = $(DRMAA1_PACK)/AlreadyActiveSessionException.java \
        $(DRMAA1_PACK)/AuthorizationException.java \
        $(DRMAA1_PACK)/ConflictingAttributeValuesException.java \
        $(DRMAA1_PACK)/DefaultContactStringException.java \
        $(DRMAA1_PACK)/DeniedByDrmException.java \
        $(DRMAA1_PACK)/DrmaaException.java \
        $(DRMAA1_PACK)/DrmaaJNI.java \
        $(DRMAA1_PACK)/DrmCommunicationException.java \
        $(DRMAA1_PACK)/DrmsExitException.java \
        $(DRMAA1_PACK)/DrmsInitException.java \
        $(DRMAA1_PACK)/ExitTimeoutException.java \
        $(DRMAA1_PACK)/FileTransferMode.java \
        $(DRMAA1_PACK)/HoldInconsistentStateException.java \
        $(DRMAA1_PACK)/InconsistentStateException.java \
        $(DRMAA1_PACK)/InternalException.java \
        $(DRMAA1_PACK)/InvalidAttributeFormatException.java \
        $(DRMAA1_PACK)/InvalidAttributeValueException.java \
        $(DRMAA1_PACK)/InvalidContactStringException.java \
        $(DRMAA1_PACK)/InvalidJobException.java \
        $(DRMAA1_PACK)/InvalidJobTemplateException.java \
        $(DRMAA1_PACK)/JobInfo.java \
        $(DRMAA1_PACK)/JobTemplate.java \
        $(DRMAA1_PACK)/NoActiveSessionException.java \
        $(DRMAA1_PACK)/NoDefaultContactStringSelectedException.java \
        $(DRMAA1_PACK)/PartialTimestampFormat.java \
        $(DRMAA1_PACK)/PartialTimestamp.java \
        $(DRMAA1_PACK)/ReleaseInconsistentStateException.java \
        $(DRMAA1_PACK)/ResumeInconsistentStateException.java \
        $(DRMAA1_PACK)/SessionFactory.java \
        $(DRMAA1_PACK)/Session.java \
        $(DRMAA1_PACK)/SuspendInconsistentStateException.java \
        $(DRMAA1_PACK)/TryLaterException.java \
        $(DRMAA1_PACK)/UnsupportedAttributeException.java \
        $(DRMAA1_PACK)/Version.java \
        $(DRMAA1_PACK)/GridWayJobInfo.java \
        $(DRMAA1_PACK)/GridWayJobTemplate.java \
        $(DRMAA1_PACK)/GridWaySessionFactory.java \
        $(DRMAA1_PACK)/GridWaySession.java \
        $(DRMAA1_PACK)/SimpleJobTemplate.java

EXTRA_DIST += $(DRMAA_JAVA_SRC) $(DRMAA_PACK)/DrmaaJNI.h $(DRMAA_PACK)/DrmaaJNI.c

EXTRA_DIST += $(DRMAA1_JAVA_SRC) $(DRMAA1_PACK)/DrmaaJNI.h $(DRMAA1_PACK)/DrmaaJNI.c

# -----------------------------
# -- Includes and Headers
# -----------------------------

INCLUDES = -I$(top_srcdir)/include \
             $(DB4_INCLUDE_PATH) \
           -I${JAVA_HOME}/include \
           -I${JAVA_HOME}/include/$(SYSTEM)

SYSTEM=@system@
flavorincludedir = $(prefix)/include

flavorinclude_HEADERS = $(top_srcdir)/include/drmaa.h \
        $(top_srcdir)/include/drmaa2.h \
        $(top_srcdir)/include/gw_acct.h \
        $(top_srcdir)/include/gw_action.h \
        $(top_srcdir)/include/gw_array.h \
        $(top_srcdir)/include/gw_array_pool.h \
        $(top_srcdir)/include/gw_client.h \
        $(top_srcdir)/include/gw_cmds_common.h \
        $(top_srcdir)/include/gw_common.h \
        $(top_srcdir)/include/gw_conf.h \
        $(top_srcdir)/include/gw_dm.h \
        $(top_srcdir)/include/gw_dm_mad.h \
        $(top_srcdir)/include/gw_drmaa_jt.h \
        $(top_srcdir)/include/gw_em.h \
        $(top_srcdir)/include/gw_em_mad.h \
        $(top_srcdir)/include/gw_em_rsl.h \
        $(top_srcdir)/include/gw_file_parser.h \
        $(top_srcdir)/include/gw_history.h \
        $(top_srcdir)/include/gw_host.h \
        $(top_srcdir)/include/gw_host_pool.h \
        $(top_srcdir)/include/gw_im.h \
        $(top_srcdir)/include/gw_im_mad.h \
        $(top_srcdir)/include/gw_job.h \
        $(top_srcdir)/include/gw_job_pool.h \
        $(top_srcdir)/include/gw_job_template.h \
        $(top_srcdir)/include/gw_log.h \
        $(top_srcdir)/include/gw_rm_connection_list.h \
        $(top_srcdir)/include/gw_rm.h \
        $(top_srcdir)/include/gw_rm_msg.h \
        $(top_srcdir)/include/gw_scheduler.h \
        $(top_srcdir)/include/gw_template.h \
        $(top_srcdir)/include/gw_tm.h \
        $(top_srcdir)/include/gw_tm_mad.h \
        $(top_srcdir)/include/gw_um.h \
        $(top_srcdir)/include/gw_user.h \
        $(top_srcdir)/include/gw_user_pool.h \
        $(top_srcdir)/include/gw_xfr_files.h \
        $(top_srcdir)/include/gw_sch_conf.h

#-----------------------------------
# GWD
#----------------------------------

bin_PROGRAMS = $(srcdir)/gwd/gwd
__srcdir__gwd_gwd_SOURCES = $(EM_SRC) \
        $(TM_SRC) \
        $(IM_SRC) \
        $(JOB_SRC) \
        $(HOST_SRC) \
        $(COMMON_SRC) \
        $(PM_SRC) \
        $(ARRAY_SRC) \
        $(DM_SRC) \
        $(RM_SRC) \
        $(GWD_SRC) \
        $(UM_SRC)  \
        $(DB_SRC)

bin_SCRIPTS   =
dist_bin_SCRIPTS =
CLEANFILES =

# -----------------------------
# Commands
# -----------------------------

bin_PROGRAMS += $(srcdir)/cmds/gwps
__srcdir__cmds_gwps_SOURCES = $(CMDS)/gw_ps.c \
        $(CMDS)/gw_cmds_common.c \
        $(CLIENT_SRC) \
        $(COMMON_SRC)

bin_PROGRAMS += $(srcdir)/cmds/gwsubmit
__srcdir__cmds_gwsubmit_SOURCES = $(CMDS)/gw_submit.c \
        $(CMDS)/gw_cmds_common.c \
        $(CLIENT_SRC) \
        $(COMMON_SRC)

bin_PROGRAMS += $(srcdir)/cmds/gwhistory
__srcdir__cmds_gwhistory_SOURCES = $(CMDS)/gw_history.c \
        $(CMDS)/gw_cmds_common.c \
        $(CLIENT_SRC) \
        $(COMMON_SRC)

bin_PROGRAMS += $(srcdir)/cmds/gwkill
__srcdir__cmds_gwkill_SOURCES = $(CMDS)/gw_kill.c \
        $(CMDS)/gw_cmds_common.c \
        $(CLIENT_SRC) \
        $(COMMON_SRC)

bin_PROGRAMS += $(srcdir)/cmds/gwhost
__srcdir__cmds_gwhost_SOURCES = $(CMDS)/gw_host.c \
        $(CMDS)/gw_cmds_common.c \
        $(CLIENT_SRC) \
        $(COMMON_SRC)

bin_PROGRAMS += $(srcdir)/cmds/gwwait
__srcdir__cmds_gwwait_SOURCES = $(CMDS)/gw_wait.c \
        $(CMDS)/gw_cmds_common.c \
        $(CLIENT_SRC) \
        $(COMMON_SRC)

bin_PROGRAMS += $(srcdir)/cmds/gwuser
__srcdir__cmds_gwuser_SOURCES = $(CMDS)/gw_user.c \
        $(CMDS)/gw_cmds_common.c \
        $(CLIENT_SRC) \
        $(COMMON_SRC)

bin_PROGRAMS += $(srcdir)/cmds/gwacct
__srcdir__cmds_gwacct_SOURCES = $(CMDS)/gw_acct.c \
        $(CMDS)/gw_cmds_common.c \
        $(CLIENT)/gw_client_init.c \
        $(DB_SRC) \
        $(CLIENT_ACCT_SRC) \
        $(COMMON_SRC)

dist_bin_SCRIPTS += $(CMDS_RB_SRC) 

# ----------------------------
# JSDL Support
# ----------------------------

if JSDL

dist_bin_SCRIPTS += $(CMDS)/jsdl2gw.sh

# Create gw_jsdl.jar
jsdl_jardir = $(prefix)/lib
jsdl_jar_DATA = $(srcdir)/cmds/gw_jsdl.jar
$(srcdir)/cmds/gw_jsdl.jar:
	@rm -f lib/gw_jsdl.jar
	@mkdir -p $(CMDS)/package
	$(JAVAC_PATH) $(JSDL_SRC) \
	    -classpath @classpath@:$(CMDS) \
        -d $(CMDS)/package
	jar cf $(srcdir)/cmds/gw_jsdl.jar -C $(CMDS)/package/ .
	@rm -rf $(CMDS)/package

CLEANFILES += $(srcdir)/cmds/gw_jsdl.jar

endif

# ----------------------------
# Schedulers
# ----------------------------

bin_PROGRAMS += $(srcdir)/scheduler/gw_flood_scheduler
__srcdir__scheduler_gw_flood_scheduler_SOURCES = \
        $(SCHED)/gw_flood_scheduler.c \
        $(COMMON_SRC) \
        $(CLIENT_SRC) \
        $(DB_SRC) \
        $(SCHED_SRC) \
        $(GWD)/gw_sch_conf.c \
        $(GWD)/gw_sch_conf_syntax.y \
        $(GWD)/gw_sch_conf_parser.l

__srcdir__scheduler_gw_flood_scheduler_LDFLAGS = -lm

bin_PROGRAMS += $(srcdir)/scheduler/gw_sched
__srcdir__scheduler_gw_sched_SOURCES = \
        $(SCHED)/gw_scheduler.c \
        $(COMMON_SRC) \
        $(CLIENT_SRC) \
        $(DB_SRC) \
        $(SCHED_SRC) \
        $(GWD)/gw_sch_conf.c \
        $(GWD)/gw_sch_conf_syntax.y \
        $(GWD)/gw_sch_conf_parser.l

__srcdir__scheduler_gw_sched_LDFLAGS = -lm

# ----------------------------
# DRMAA
# ----------------------------

lib_LTLIBRARIES = $(srcdir)/drmaa/libdrmaa.la $(srcdir)/drmaa/libdrmaa2.la

if DRMAA_JAVA
lib_LTLIBRARIES += $(srcdir)/drmaa/libDrmaaJNI.la
endif

__srcdir__drmaa_libdrmaa_la_SOURCES = $(DRMAA_SRC)
__srcdir__drmaa_libdrmaa2_la_SOURCES = $(DRMAA2_SRC)

if DRMAA_JAVA
if DRMAA1_JAVA
__srcdir__drmaa_libDrmaaJNI_la_SOURCES = $(DRMAA1JNI_SRC)
else
__srcdir__drmaa_libDrmaaJNI_la_SOURCES = $(DRMAAJNI_SRC)
endif
endif
__srcdir__drmaa_libDrmaaJNI_la_LIBADD  = $(srcdir)/drmaa/libdrmaa.la

__srcdir__drmaa_libdrmaa_la_CFLAGS = $(AM_CFLAGS)
__srcdir__drmaa_libdrmaa2_la_CFLAGS = $(AM_CFLAGS)

# Use libtool to generate drmaa library files
__srcdir__drmaa_libdrmaa_la_LINK = $(LIBTOOL) --tag=CC --mode=link $(LINK)
__srcdir__drmaa_libdrmaa2_la_LINK = $(LIBTOOL) --tag=CC --mode=link $(LINK)
__srcdir__drmaa_libDrmaaJNI_la_LINK = $(LIBTOOL) --tag=CC --mode=link $(LINK)

if DRMAA_JAVA

jardir = $(prefix)/lib

jar_DATA = $(srcdir)/drmaa/drmaa.jar
$(srcdir)/drmaa/drmaa.jar:
	@rm -f lib/drmaa.jar
	@mkdir -p $(DRMAA)/package
if DRMAA1_JAVA
	$(JAVAC_PATH) $(DRMAA1_JAVA_SRC) \
	        -classpath $(DRMAA) \
                -d $(DRMAA)/package
else
	$(JAVAC_PATH) $(DRMAA_JAVA_SRC) \
	        -classpath $(DRMAA) \
	        -d $(DRMAA)/package
endif
	jar cf $(srcdir)/drmaa/drmaa.jar -C $(DRMAA)/package/ .
	@rm -rf $(DRMAA)/package
endif

CLEANFILES += $(srcdir)/drmaa/drmaa.jar

# ----------------------------
# TESTS
# ----------------------------

if TESTS
testdir = @prefix@/test/

test_PROGRAMS = $(srcdir)/test/gwtest

__srcdir__test_gwtest_SOURCES = test/gw_test.c \
        $(CMDS)/gw_cmds_common.c \
        $(CLIENT_SRC) \
        $(COMMON_SRC)
endif

# ----------------------------
# Distribute Java and MAD sources
# ----------------------------

IM_MAD_STATIC_SRC_DIST = $(IM_MAD)/gw_im_mad_static.sh \
	$(IM_MAD)/gw_im_mad_common.sh

IM_MAD_MDS2_SRC_DIST = $(IM_MAD)/mds2/gw_im_mad_mds2.sh \
        $(IM_MAD)/mds2/gw_im_mad_mds2_glue.sh \
        $(IM_MAD)/mds2/Makefile

IM_MAD_MDS4_SRC_DIST = $(IM_MAD)/mds4/Host.java \
        $(IM_MAD)/mds4/Mds4QueryParser_GT40.java \
        $(IM_MAD)/mds4/Mds4QueryParser_GT42.java \
        $(IM_MAD)/mds4/Queue.java \
        $(IM_MAD)/mds4/QueryClient_GT40.java \
        $(IM_MAD)/mds4/QueryClient_GT42.java \
        $(IM_MAD)/mds4/Gw_im_mad_mds4.java \
        $(IM_MAD)/mds4/ServiceIM.java \
        $(IM_MAD)/mds4/gw_im_mad_mds4.sh \
        $(IM_MAD)/mds4/Makefile

IM_MAD_BDII_SRC_DIST = $(IM_MAD)/bdii/gw_im_mad_bdii.pl \
        $(IM_MAD)/bdii/gw_im_mad_bdii.sh \
        $(IM_MAD)/bdii/Makefile

EM_MAD_GRAM2_SRC_DIST = $(EM_MAD)/gram2/gw_em_mad_gram2.c \
	$(EM_MAD)/gram2/gw_em_mad_gram2_actions.c \
	$(EM_MAD)/gram2/gw_em_mad_gram2_pool.c \
	$(EM_MAD)/gram2/gw_em_mad_gram2.h \
	$(EM_MAD)/gram2/gw_em_mad_gram2.sh \
	$(EM_MAD)/gram2/Makefile

EM_MAD_GRAM4_SRC_DIST = $(EM_MAD)/gram4/Gw_em_mad_gram4_GT40.java \
	$(EM_MAD)/gram4/Gw_em_mad_gram4_GT42.java \
	$(EM_MAD)/gram4/gw_em_mad_gram4.sh \
	$(EM_MAD)/gram4/Makefile

EM_MAD_CREAM_SRC_DIST = $(EM_MAD)/cream/gw_em_mad_cream.cpp \
	$(EM_MAD)/cream/gw_em_mad_cream_actions.cpp \
	$(EM_MAD)/cream/gw_em_mad_cream.h \
	$(EM_MAD)/cream/gw_em_mad_cream.sh \
	$(EM_MAD)/cream/README \
	$(EM_MAD)/cream/Makefile

EM_MAD_NORDU_SRC_DIST = $(EM_MAD)/nordugrid/gw_em_mad_nordugrid.py \
	$(EM_MAD)/nordugrid/em_nordugrid.py \
	$(EM_MAD)/nordugrid/gw_em_mad_nordugrid.sh

EM_MAD_SSH_SRC_DIST = $(EM_MAD)/ssh/gw_em_mad_ssh.rb \
	$(EM_MAD)/ssh/gwmad.rb \
	$(EM_MAD)/ssh/rslparser.rb \
	$(EM_MAD)/ssh/sshjob.rb \
	$(EM_MAD)/ssh/sshresource.rb \
	$(EM_MAD)/ssh/gw_em_mad_ssh.sh \
	$(EM_MAD)/ssh/Makefile

EM_MAD_BES_SRC_DIST = $(EM_MAD)/bes/GW_em_mad_bes.java \
        $(EM_MAD)/bes/GW_em_mad_bes_actions.java \
	$(EM_MAD)/bes/gw_em_mad_bes.sh \
	$(EM_MAD)/bes/README \
	$(EM_MAD)/bes/client-config.wsdd \
	$(EM_MAD)/bes/crypto.properties \
	$(EM_MAD)/bes/Makefile

TM_MAD_GRIDFTP_SRC_DIST = $(TM_MAD)/gridftp/gw_tm_mad_ftp.c \
	$(TM_MAD)/gridftp/gw_tm_ftp_transfer.c \
	$(TM_MAD)/gridftp/gw_tm_ftp_xfr_pool.c \
	$(TM_MAD)/gridftp/gw_tm_ftp_stack.c \
	$(TM_MAD)/gridftp/gw_tm_ftp_queue.c \
	$(TM_MAD)/gridftp/gw_tm_ftp_transfer.h \
	$(TM_MAD)/gridftp/gw_tm_mad_ftp.sh \
	$(TM_MAD)/gridftp/Makefile

TM_MAD_DUMMY_SRC_DIST = $(TM_MAD)/dummy/gw_tm_mad_dummy.c \
	$(TM_MAD)/dummy/gw_tm_mad_dummy.sh \
	$(TM_MAD)/dummy/Makefile

TM_MAD_NORDU_SRC_DIST = $(TM_MAD)/nordugrid/gw_tm_mad_nordugrid.py \
        $(TM_MAD)/nordugrid/tm_nordugrid.py \
        $(TM_MAD)/nordugrid/gw_tm_mad_nordugrid.sh

TM_MAD_SSH_SRC_DIST = $(TM_MAD)/ssh/gw_tm_mad_ssh.rb \
	$(TM_MAD)/ssh/gw_tm_mad_ssh.sh \
	$(TM_MAD)/ssh/Makefile

TM_MAD_RFT_SRC_DIST = $(TM_MAD)/rft/GWRftClient.java \
	$(TM_MAD)/rft/GWRftProcessor.java \
	$(TM_MAD)/rft/GWRftResource.java \
	$(TM_MAD)/rft/GWRftUtil.java \
	$(TM_MAD)/rft/gw_tm_mad_rft.sh

EXTRA_DIST += $(IM_MAD_STATIC_SRC_DIST) $(IM_MAD_MDS2_SRC_DIST) \
$(IM_MAD_MDS4_SRC_DIST) $(IM_MAD_BDII_SRC_DIST) \
$(EM_MAD_GRAM2_SRC_DIST) $(EM_MAD_GRAM4_SRC_DIST) $(EM_MAD_CREAM_SRC_DIST) \
$(EM_MAD_NORDU_SRC_DIST) $(EM_MAD_SSH_SRC_DIST) $(EM_MAD_BES_SRC_DIST) \
$(TM_MAD_GRIDFTP_SRC_DIST) $(TM_MAD_DUMMY_SRC_DIST) $(TM_MAD_NORDU_SRC_DIST) \
$(TM_MAD_SSH_SRC_DIST) $(TM_MAD_RFT_SRC_DIST)
