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

# -----------------------------
# Automake Options
# -----------------------------

AUTOMAKE_OPTIONS = subdir-objects no-dependencies
AM_YFLAGS = -d

# Get rid of libtool by default & make use of globus compiler/linker

if GW_GLOBUS_DIRS
LINK_PROG    = $(CCLD)
COMPILE_PROG = $(CC)
else
COMPILE_PROG = $(GLOBUS_CC)
LINK_PROG    = $(GLOBUS_LD)
endif

LINK_OPTS = $(AM_CFLAGS) $(CFLAGS) $(AM_LDFLAGS) $(LDFLAGS) -o $@

COMPILE_OPTS = $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) \
			   $(CPPFLAGS) $(AM_CFLAGS) $(CFLAGS)

LINK = $(LINK_PROG) $(LINK_OPTS)
COMPILE = $(COMPILE_PROG) $(COMPILE_OPTS)

if GW_GLOBUS_DIRS
GT_PKG_LIBS = @globus_pkg_libs@
else
GT_PKG_LIBS = $(GLOBUS_LIBS) $(GLOBUS_PKG_LIBS)
endif

# to handle else, not very smart 
NOP =
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

if DRMAA1
DRMAA_SRC_DIR=drmaa/drmaa1.0
else
DRMAA_SRC_DIR=drmaa
endif

DRMAA = $(srcdir)/drmaa
DRMAA_PACK = $(srcdir)/$(DRMAA_SRC_DIR)/org/ggf/drmaa

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

# NORDUGRID MADS

NORDUGRID_MAD_SRC=\
	$(EM_MAD)/gw_em_mad_nordugrid.sh \
	$(EM_MAD)/gw_em_mad_nordugrid.py \
	$(EM_MAD)/em_nordugrid.py \
	$(TM_MAD)/gw_tm_mad_nordugrid.sh \
	$(TM_MAD)/gw_tm_mad_nordugrid.py \
	$(TM_MAD)/tm_nordugrid.py

JSDL_SRC = $(CMDS)/GWJTElement.java \
        $(CMDS)/GWJT.java  \
        $(CMDS)/GWJTWriter.java  \
        $(CMDS)/JSDLElement.java  \
        $(CMDS)/JSDL.java  \
        $(CMDS)/JSDLReader.java  \
        $(CMDS)/JSDLParser.java  \
        $(CMDS)/JSDLTranslator.java

EM_SRC = $(EM)/gw_em_actions.c \
        $(EM)/gw_em.c \
        $(EM)/gw_em_listener.c \
        $(EM)/gw_em_mad.c \
        $(EM)/gw_em_rsl2.c \
        $(EM)/gw_em_rsl.c \
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

EM_MAD_PREWS_SRC = $(EM_MAD)/gw_em_mad_prews_actions.c \
        $(EM_MAD)/gw_em_mad_prews.c \
        $(EM_MAD)/gw_em_mad_prews_pool.c

EM_MAD_WS_SRC = $(EM_MAD)/GW_mad_ws.java

TM_MAD_SRC = $(TM_MAD)/gw_tm_ftp_mad.c \
        $(TM_MAD)/gw_tm_ftp_queue.c \
        $(TM_MAD)/gw_tm_ftp_stack.c \
        $(TM_MAD)/gw_tm_ftp_transfer.c \
        $(TM_MAD)/gw_tm_ftp_xfr_pool.c

TM_MAD_DUMMY_SRC = $(TM_MAD)/gw_tm_dummy_mad.c

IM_MAD_MDS4_SRC = $(IM_MAD)/Host.java \
        $(IM_MAD)/Mds4QueryParser.java \
        $(IM_MAD)/Queue.java \
        $(IM_MAD)/QueryClient.java \
        $(IM_MAD)/GW_im_mad_mds4.java \
        $(IM_MAD)/ServiceIM.java

TM_MAD_RFT_SRC = $(TM_MAD)/GWRftClient.java \
        $(TM_MAD)/GWRftProcessor.java \
        $(TM_MAD)/GWRftResource.java \
        $(TM_MAD)/GWRftUtil.java

SSH_SRC = $(EM_MAD)/gw_em_mad_ssh.sh \
        $(TM_MAD)/gw_tm_mad_ssh.sh \
        $(SSH)/ssh_wrapper.sh \
        $(SSH)/ruby/gw_em_mad_ssh.rb \
        $(SSH)/ruby/gwmad.rb \
        $(SSH)/ruby/gw_tm_mad_ssh.rb \
        $(SSH)/ruby/rslparser.rb \
        $(SSH)/ruby/sshjob.rb \
        $(SSH)/ruby/sshresource.rb

DRMAA_SRC = $(COMMON)/gw_file_parser.c $(CLIENT_SRC) \
        $(DRMAA)/gw_drmaa.c $(DRMAA)/gw_drmaa_jt.c \
        $(COMMON)/gw_template_parser.l \
        $(COMMON)/gw_template.c $(COMMON)/gw_log.c

DRMAAJNI_SRC = $(DRMAA_PACK)/DrmaaJNI.c

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
        $(DRMAA_PACK)/Version.java
if DRMAA1        
        DRMAA_JAVA_SRC += $(DRMAA_PACK)/GridWayJobInfo.java \
        $(DRMAA_PACK)/GridWayJobTemplate.java \
        $(DRMAA_PACK)/GridWaySessionFactory.java \
        $(DRMAA_PACK)/GridWaySession.java \
        $(DRMAA_PACK)/SimpleJobTemplate.java
else
        DRMAA_JAVA_SRC += $(DRMAA_PACK)/DrmaaRuntimeException.java \
        $(DRMAA_PACK)/InvalidAttributeException.java \
        $(DRMAA_PACK)/JobTemplateImpl.java \
        $(DRMAA_PACK)/JobInfoImpl.java \
        $(DRMAA_PACK)/NoResourceUsageException.java \
        $(DRMAA_PACK)/SessionException.java \
        $(DRMAA_PACK)/SessionImpl.java \
        $(DRMAA_PACK)/SessionFactoryImpl.java        
endif        

# -----------------------------
# -- Includes and Headers
# -----------------------------

INCLUDES = -I$(top_srcdir)/include \
             $(DB4_INCLUDE_PATH) \
           -I${JAVA_HOME}/include \
           -I${JAVA_HOME}/include/$(SYSTEM)

if GW_GLOBUS_DIRS
SYSTEM=@system@
flavorincludedir = $(GLOBUS_LOCATION)/include/@GLOBUS_FLAVOR_NAME@
else
SYSTEM=@system@
flavorincludedir = $(prefix)/include
@incgtmk@
INCLUDES += $(GLOBUS_INCLUDES)
endif

flavorinclude_HEADERS = $(top_srcdir)/include/drmaa.h \
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
                        $(top_srcdir)/include/gw_em_mad_prews.h \
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
                        $(top_srcdir)/include/gw_tm_ftp_transfer.h \
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

lib_LTLIBRARIES =

if DRMAA
if GW_GLOBUS_DIRS
lib_LTLIBRARIES += $(srcdir)/drmaa/libdrmaa_$(GLOBUS_FLAVOR_NAME).la
lib_LTLIBRARIES += $(srcdir)/drmaa/libDrmaaJNI_$(GLOBUS_FLAVOR_NAME).la
else
lib_LTLIBRARIES += $(srcdir)/drmaa/libdrmaa.la
lib_LTLIBRARIES += $(srcdir)/drmaa/libDrmaaJNI.la
endif
endif

if GW_GLOBUS_DIRS
__srcdir__drmaa_libdrmaa___GLOBUS_FLAVOR_NAME__la_SOURCES = $(DRMAA_SRC)
__srcdir__drmaa_libDrmaaJNI___GLOBUS_FLAVOR_NAME__la_SOURCES = $(DRMAAJNI_SRC)
__srcdir__drmaa_libDrmaaJNI___GLOBUS_FLAVOR_NAME__la_LIBADD  = $(srcdir)/drmaa/libdrmaa_$(GLOBUS_FLAVOR_NAME).la 

__srcdir__drmaa_libdrmaa___GLOBUS_FLAVOR_NAME__la_CFLAGS = $(AM_CFLAGS)

# Use libtool to generate drmaa library files
__srcdir__drmaa_libdrmaa___GLOBUS_FLAVOR_NAME__la_LINK = $(LIBTOOL) --tag=CC --mode=link $(LINK)
__srcdir__drmaa_libDrmaaJNI___GLOBUS_FLAVOR_NAME__la_LINK = $(LIBTOOL) --tag=CC --mode=link $(LINK)
else
__srcdir__drmaa_libdrmaa_la_SOURCES = $(DRMAA_SRC)
__srcdir__drmaa_libDrmaaJNI_la_SOURCES = $(DRMAAJNI_SRC)
__srcdir__drmaa_libDrmaaJNI_la_LIBADD  = $(srcdir)/drmaa/libdrmaa.la

__srcdir__drmaa_libdrmaa_la_CFLAGS = $(AM_CFLAGS)

# Use libtool to generate drmaa library files
__srcdir__drmaa_libdrmaa_la_LINK = $(LIBTOOL) --tag=CC --mode=link $(LINK)
__srcdir__drmaa_libDrmaaJNI_la_LINK = $(LIBTOOL) --tag=CC --mode=link $(LINK)
endif

if DRMAA

jardir = $(prefix)/lib

jar_DATA = $(srcdir)/drmaa/drmaa.jar
$(srcdir)/drmaa/drmaa.jar:
	@rm -f lib/drmaa.jar
	@mkdir -p $(DRMAA)/package
	$(JAVAC_PATH) $(DRMAA_JAVA_SRC) \
	    -classpath $(DRMAA) \
        -d $(DRMAA)/package
	jar cf $(srcdir)/drmaa/drmaa.jar -C $(DRMAA)/package/ .
	@rm -rf $(DRMAA)/package
endif

CLEANFILES += $(srcdir)/drmaa/drmaa.jar

# --------------------------------
# Pre-WS Middleware Access Drivers
# --------------------------------

# ---------------------- EM MADS ------------------------

if GRAM5
bin_PROGRAMS += $(srcdir)/em_mad/gw_em_mad_prews.bin
dist_bin_SCRIPTS  += $(EM_MAD)/gw_em_mad_prews.sh
endif

__srcdir__em_mad_gw_em_mad_prews_bin_SOURCES = $(EM_MAD_PREWS_SRC)
__srcdir__em_mad_gw_em_mad_prews_bin_LDADD   = $(GT_PKG_LIBS)

if GW_GLOBUS_DIRS
NOP +=
else
__srcdir__em_mad_gw_em_mad_prews_bin_LDFLAGS = $(GLOBUS_LDFLAGS)
__srcdir__em_mad_gw_em_mad_prews_bin_CFLAGS  = $(GLOBUS_INCLUDES) \
        $(GLOBUS_CFLAGS)
endif


# ---------------------- TM MADS ------------------------

if DUMMY
bin_PROGRAMS += $(srcdir)/tm_mad/gw_tm_mad_dummy.bin
dist_bin_SCRIPTS  += $(TM_MAD)/gw_tm_mad_dummy.sh
endif

if GRIDFTP
bin_PROGRAMS += $(srcdir)/tm_mad/gw_tm_mad_ftp.bin
dist_bin_SCRIPTS += $(TM_MAD)/gw_tm_mad_ftp.sh

__srcdir__tm_mad_gw_tm_mad_ftp_bin_SOURCES = $(TM_MAD_SRC)
__srcdir__tm_mad_gw_tm_mad_ftp_bin_LDADD   = $(GT_PKG_LIBS)

if GW_GLOBUS_DIRS
NOP +=
else
__srcdir__tm_mad_gw_tm_mad_ftp_bin_LDFLAGS = $(GLOBUS_LDFLAGS)
__srcdir__tm_mad_gw_tm_mad_ftp_bin_CFLAGS  = $(GLOBUS_INCLUDES) \
        $(GLOBUS_CFLAGS)
endif

endif

__srcdir__tm_mad_gw_tm_mad_dummy_bin_SOURCES = $(TM_MAD_DUMMY_SRC)
__srcdir__tm_mad_gw_tm_mad_dummy_bin_LDADD   = $(GT_PKG_LIBS)

if GW_GLOBUS_DIRS
NOP +=
else
__srcdir__tm_mad_gw_tm_mad_dummy_bin_LDFLAGS = $(GLOBUS_LDFLAGS)
__srcdir__tm_mad_gw_tm_mad_dummy_bin_CFLAGS  = $(GLOBUS_INCLUDES) \
        $(GLOBUS_CFLAGS)
endif

# ---------------------- IM MADS ------------------------

dist_bin_SCRIPTS  += $(IM_MAD)/gw_im_mad_common.sh
dist_bin_SCRIPTS  += $(IM_MAD)/gw_im_mad_static.sh

if MDS2
dist_bin_SCRIPTS  += $(IM_MAD)/gw_im_mad_mds2.sh
dist_bin_SCRIPTS  += $(IM_MAD)/gw_im_mad_mds2_glue.sh
dist_bin_SCRIPTS  += $(IM_MAD)/gw_im_mad_egee_ldap.sh
dist_bin_SCRIPTS  += $(IM_MAD)/gw_im_mad_egee_ldap.pl
endif

# --------------------------------
# WS Middleware Access Drivers
# --------------------------------
JAVAROOT = .

if MDS4
im_ws_jardir = $(prefix)/lib
im_ws_jar_DATA = $(srcdir)/im_mad/gw_im_mad_ws.jar
$(srcdir)/im_mad/gw_im_mad_ws.jar:
	@rm -f lib/gw_im_mad_ws.jar
	@mkdir -p $(IM_MAD)/package 
	$(JAVAC_PATH) $(IM_MAD_MDS4_SRC) \
	    -classpath @classpath@:$(IM_MAD) \
        -d $(IM_MAD)/package
	jar cf $(srcdir)/im_mad/gw_im_mad_ws.jar -C $(IM_MAD)/package/ .
	@rm -rf $(IM_MAD)/package

dist_bin_SCRIPTS += $(IM_MAD)/gw_im_mad_mds4.sh
dist_bin_SCRIPTS += $(IM_MAD)/gw_im_mad_mds4_thr.sh

CLEANFILES += $(srcdir)/im_mad/gw_im_mad_ws.jar
endif

if GRAM4
em_ws_jardir = $(prefix)/lib
em_ws_jar_DATA = $(srcdir)/em_mad/gw_em_mad_ws.jar
$(srcdir)/em_mad/gw_em_mad_ws.jar:
	@rm -f lib/gw_em_mad_ws.jar
	@mkdir -p $(EM_MAD)/package
	$(JAVAC_PATH) $(EM_MAD_WS_SRC) \
	    -classpath @classpath@:$(EM_MAD) \
        -d $(EM_MAD)/package
	jar cf $(srcdir)/em_mad/gw_em_mad_ws.jar -C $(EM_MAD)/package/ .
	@rm -rf $(EM_MAD)/package

dist_bin_SCRIPTS += $(EM_MAD)/gw_em_mad_ws.sh

CLEANFILES += $(srcdir)/em_mad/gw_em_mad_ws.jar

endif

# --------------------------------
# NORDUGRID Middleware Access Drivers
# --------------------------------

if NORDUGRID
dist_bin_SCRIPTS += $(NORDUGRID_MAD_SRC)
endif

# --------------------------------
# SSH Middleware Access Drivers
# --------------------------------
if SSH
dist_bin_SCRIPTS += $(SSH_SRC)
endif

# ----------------------------
# TESTS
# ----------------------------

if TESTS
testdir = @prefix@/test/gridway

test_PROGRAMS = $(srcdir)/test/gwtest

__srcdir__test_gwtest_SOURCES = test/gw_test.c \
        $(CMDS)/gw_cmds_common.c \
        $(CLIENT_SRC) \
        $(COMMON_SRC)
endif

# ----------------------------
# Distribute JAVA sources
# ----------------------------


EM_MAD_WS_SRC_DIST=$(EM_MAD)/GW_mad_ws.GT4.0.java \
	$(EM_MAD)/GW_mad_ws.GT4.2.java

IM_MAD_MDS4_SRC_DIST = $(IM_MAD)/Host.java \
        $(IM_MAD)/Mds4QueryParser.GT4.0.java \
        $(IM_MAD)/Mds4QueryParser.GT4.2.java \
        $(IM_MAD)/Queue.java \
        $(IM_MAD)/QueryClient.GT4.0.java \
        $(IM_MAD)/QueryClient.GT4.2.java \
        $(IM_MAD)/GW_im_mad_mds4.java \
        $(IM_MAD)/ServiceIM.java

dist-hook:
	@mkdir -p $(distdir)/cmds
	@mkdir -p $(distdir)/im_mad
	@mkdir -p $(distdir)/em_mad
	@mkdir -p $(distdir)/$(DRMAA_SRC_DIR)/org/ggf/drmaa
	@cp $(JSDL_SRC) $(distdir)/cmds/
	@cp $(IM_MAD_MDS4_SRC_DIST) $(distdir)/im_mad
	@cp $(EM_MAD_WS_SRC_DIST) $(distdir)/em_mad
	@cp $(DRMAA_JAVA_SRC) $(distdir)/$(DRMAA_SRC_DIR)/org/ggf/drmaa
	@cp $(DRMAA_PACK)/DrmaaJNI.h $(distdir)/$(DRMAA_SRC_DIR)/org/ggf/drmaa
