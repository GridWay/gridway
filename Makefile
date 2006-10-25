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

#-------------------------------------------------
# Configuration of Compilation Flags and Libraries
#-------------------------------------------------

# Programs
# LEX & YACC are not really needed

CC    = gcc
JAVAC = javac
LEX   = flex
YACC  = bison

# Defualt conf for linux

SYSYTEM   = linux
LDOPTIONS = -shared

# Solaris configuration (uncomment)
# NOTE: m64 is needed for gcc64dbg Globus flavors
#SYSYTEM         = solaris
#LIBS_SUNOS      = -lnsl -lsocket
#CFLAGS_SUNOS    = -m64 -DGWSOLARIS
#LDOPTIONS       = -G

# PATH for DB4 library, if not in default locations with -L, -I options
#
# COMMENT DB4_ VARIABLES IF YOU DO NOT WANT ACCT SUPPORT TO BE COMPILED IN GW
#
# EXAMPLE:
# DB4_LD_PATH      = -L /usr/local/BerkeleyDB.4.4/lib
# DB4_INCLUDE_PATH = -I /usr/local/BerkeleyDB.4.4/include
#------------------------------------------------------------------------------
# DB4_LD_PATH      = 
# DB4_INCLUDE_PATH =
DB4_LIB          = -ldb
DB4_FLAGS        = -DHAVE_LIBDB


# Compilation Options
# DEBUG_FLAGS  = -DGWIMDEBUG -DGWJOBDEBUG -DGWHOSTDEBUG -DGWEMDEBUG -DGWDMDEBUG  -DGWTMDEBUG -DGWUSERDEBUG -DGWSCHEDDEBUG 
LD_FLAGS       =  $(DB4_LD_PATH)
LIBS           = -lpthread $(LIBS_SUNOS) $(DB4_LIB)
LIBS_CMD       = -lpthread $(LIBS_SUNOS)
CFLAGS         = -Wall -g $(CFLAGS_SUNOS) $(DB4_FLAGS) $(DEBUG_FLAGS)
INCLUDE_FLAGS  = -I$(INCLUDE_DIR) -I${JAVA_HOME}/include -I${JAVA_HOME}/include/$(SYSYTEM) $(DB4_INCLUDE_PATH)

# Globus related variables
GlCC           = $(GLOBUS_CC)
GCFLAGS        = $(GLOBUS_CFLAGS)
GINCLUDE_FLAGS = -I$(INCLUDE_DIR) $(GLOBUS_INCLUDES)  -I./
GLD_FLAGS      = $(GLOBUS_LDFLAGS)
GLIBS          = $(GLOBUS_LIBS) $(GLOBUS_PKG_LIBS)

# -----------------------------
# Directory definitions
# -----------------------------

include makefile-header

INCLUDE_DIR  = ./include
SRC_DIR      = ./src
EXAMPLES_DIR = ./examples
SCRIPTS_DIR  = ./scripts
LIB_DIR      = ./lib

JOB     = $(SRC_DIR)/job
HOST    = $(SRC_DIR)/host
ARRAY   = $(SRC_DIR)/array
COMMON  = $(SRC_DIR)/common
SCHED   = $(SRC_DIR)/scheduler
EM_MAD  = $(SRC_DIR)/em_mad
TM_MAD  = $(SRC_DIR)/tm_mad
IM_MAD  = $(SRC_DIR)/im_mad
CMDS    = $(SRC_DIR)/cmds
GWD     = $(SRC_DIR)/gwd
CLIENT  = $(SRC_DIR)/client

EM = $(SRC_DIR)/em
TM = $(SRC_DIR)/tm
IM = $(SRC_DIR)/im
DM = $(SRC_DIR)/dm
RM = $(SRC_DIR)/rm
UM = $(SRC_DIR)/um
DB = $(SRC_DIR)/acct

DRMAA              = $(SRC_DIR)/drmaa
DRMAA_JAVA         = $(SRC_DIR)/drmaa/org/ggf/drmaa
DRMAA_JAVA_PACKAGE = $(DRMAA)/package

#----------------------------------------------
#  Objects and Programs
#----------------------------------------------

HEADERS := $(wildcard $(INCLUDE_DIR)/*.h)

COMMON_OBJ = $(COMMON)/gw_action.o \
             $(COMMON)/gw_common.o \
             $(COMMON)/gw_file_parser.o \
             $(COMMON)/gw_log.o \
             $(COMMON)/gw_template.o \
             $(COMMON)/gw_template_parser.o
             
COMMON_LEX = $(COMMON)/gw_template_parser.c

JOB_LEX  =  $(JOB)/gw_job_template_var_parser.c           
JOB_OBJ  =  $(JOB)/gw_history.o \
        $(JOB)/gw_job.o \
        $(JOB)/gw_job_environment.o \
        $(JOB)/gw_job_pool.o \
        $(JOB)/gw_job_recover.o \
        $(JOB)/gw_job_template.o \
        $(JOB)/gw_job_template_var_parser.o \
        $(JOB)/gw_xfr_files.o

HOST_LEX  =  $(HOST)/gw_host_update_syntax.c \
        $(HOST)/gw_host_rank_syntax.c \
        $(HOST)/gw_host_reqs_syntax.c \
        $(HOST)/gw_host_attr_parser.c

HOST_OBJ  = $(HOST)/gw_host_attr_parser.o \
        $(HOST)/gw_host.o \
        $(HOST)/gw_host_var.o \
        $(HOST)/gw_host_pool.o \
        $(HOST)/gw_host_rank_syntax.o \
        $(HOST)/gw_host_reqs_syntax.o \
        $(HOST)/gw_host_update_syntax.o
		   
ARRAY_OBJ = $(ARRAY)/gw_array.o \
$(ARRAY)/gw_array_pool.o
         
CLIENT_OBJ = $(CLIENT)/gw_client_job_match.o \
        $(CLIENT)/gw_client_submit.o \
        $(CLIENT)/gw_client_host_status.o \
        $(CLIENT)/gw_client_job_status.o \
        $(CLIENT)/gw_client_user_status.o \
        $(CLIENT)/gw_client_init.o \
        $(CLIENT)/gw_client_signal.o \
        $(CLIENT)/gw_client_wait.o \
        $(HOST)/gw_host_rank_syntax.o \
        $(HOST)/gw_host_reqs_syntax.o \
        $(HOST)/gw_host_attr_parser.o \
        $(HOST)/gw_host_update_syntax.o \
        $(HOST)/gw_host_var.o
			 
CLIENT_ACCT_OBJ = $(CLIENT)/gw_client_acct.o

CMDS_OBJ = $(CMDS)/gw_acct.o \
        $(CMDS)/gw_history.o \
        $(CMDS)/gw_kill.o \
        $(CMDS)/gw_submit.o \
        $(CMDS)/gw_wait.o \
        $(CMDS)/gw_cmds_common.o \
        $(CMDS)/gw_host.o \
        $(CMDS)/gw_ps.o \
        $(CMDS)/gw_user.o
           
EM_OBJ = $(EM)/gw_em_actions.o \
        $(EM)/gw_em.o \
        $(EM)/gw_em_listener.o \
        $(EM)/gw_em_mad.o \
        $(EM)/gw_em_rsl2.o \
        $(EM)/gw_em_rsl.o \
        $(EM)/gw_em_state.o

TM_OBJ = $(TM)/gw_tm_actions.o \
        $(TM)/gw_tm.o \
        $(TM)/gw_tm_checkpoint.o \
        $(TM)/gw_tm_epilog.o \
        $(TM)/gw_tm_listener.o \
        $(TM)/gw_tm_mad.o \
        $(TM)/gw_tm_prolog.o

IM_OBJ = $(IM)/gw_im_actions.o \
        $(IM)/gw_im.o \
        $(IM)/gw_im_listener.o \
        $(IM)/gw_im_mad.o

DM_OBJ = $(DM)/gw_dm_action_allocate.o \
        $(DM)/gw_dm_action_dispatch.o \
        $(DM)/gw_dm_action_hold_release.o \
        $(DM)/gw_dm_action_kill.o \
        $(DM)/gw_dm_action_reschedule.o \
        $(DM)/gw_dm_action_schedule.o \
        $(DM)/gw_dm_action_stop_resume.o \
        $(DM)/gw_dm_action_wait.o \
        $(DM)/gw_dm.o \
        $(DM)/gw_dm_listener.o \
        $(DM)/gw_dm_mad.o \
        $(DM)/gw_dm_state_epilogs.o \
        $(DM)/gw_dm_state_failed.o \
        $(DM)/gw_dm_state_migrate.o \
        $(DM)/gw_dm_state_pending.o \
        $(DM)/gw_dm_state_prologs.o \
        $(DM)/gw_dm_state_stopped.o \
        $(DM)/gw_dm_state_wrappers.o \
        $(DM)/gw_dm_state_zombie.o

RM_OBJ = $(RM)/gw_rm.o \
        $(RM)/gw_rm_connection_list.o \
        $(RM)/gw_rm_hold_release.o \
        $(RM)/gw_rm_host_match.o \
        $(RM)/gw_rm_host_status.o \
        $(RM)/gw_rm_job_history.o \
        $(RM)/gw_rm_job_status.o \
        $(RM)/gw_rm_kill.o \
        $(RM)/gw_rm_reschedule.o \
        $(RM)/gw_rm_stop_resume.o \
        $(RM)/gw_rm_submit.o \
        $(RM)/gw_rm_user_status.o \
        $(RM)/gw_rm_wait.o

UM_OBJ = $(UM)/gw_um.o \
        $(UM)/gw_user.o \
        $(UM)/gw_user_pool.o

SCHED_OBJ = $(SCHED)/gw_scheduler_common.o \
          $(SCHED)/gw_scheduler_hosts.o \
          $(SCHED)/gw_scheduler_jobs.o \
          $(SCHED)/gw_scheduler_users.o
          
DB_OBJ = $(DB)/gw_acct.o

GWD_LEX = $(GWD)/gw_conf_parser.c
GWD_OBJ = $(GWD)/gw_conf.o \
        $(GWD)/gw_conf_parser.o \
        $(GWD)/gwd.o

EM_MAD_PREWS_OBJ = $(EM_MAD)/gw_em_mad_prews_actions.o \
        $(EM_MAD)/gw_em_mad_prews.o \
        $(EM_MAD)/gw_em_mad_prews_pool.o

TM_MAD_OBJ = $(TM_MAD)/gw_tm_ftp_mad.o \
        $(TM_MAD)/gw_tm_ftp_queue.o \
        $(TM_MAD)/gw_tm_ftp_stack.o \
        $(TM_MAD)/gw_tm_ftp_transfer.o \
        $(TM_MAD)/gw_tm_ftp_xfr_pool.o

TM_MAD_DUMMY_OBJ = $(TM_MAD)/gw_tm_dummy_mad.o
        
IM_MAD_MDS4_OBJ = $(IM_MAD)/Host.class \
        $(IM_MAD)/Mds4QueryParser.class \
        $(IM_MAD)/Queue.class

TM_MAD_RFT_OBJ := $(TM_MAD)/GWRftClient.class \
        $(TM_MAD)/GWRftProcessor.class \
        $(TM_MAD)/GWRftResource.class \
        $(TM_MAD)/GWRftUtil.class

COBJS = $(EM_OBJ) ${TM_OBJ} ${IM_OBJ} $(JOB_OBJ) ${HOST_OBJ} $(COMMON_OBJ) \
        $(PM_OBJ) $(ARRAY_OBJ) $(DM_OBJ) $(RM_OBJ) $(GWD_OBJ) $(UM_OBJ) $(DB_OBJ)
        
DRMAA_OBJS      = $(COMMON)/gw_file_parser.o $(CLIENT_OBJ) \
        $(DRMAA)/gw_drmaa.o $(DRMAA)/gw_drmaa_jt.o $(COMMON)/gw_template_parser.o \
        $(COMMON)/gw_template.o $(COMMON)/gw_log.o
                  
DRMAAJNI_OBJS   = $(DRMAA_JAVA)/DrmaaJNI.o 
              
#------------------------
# Compilation Rules
#------------------------
.SUFFIXES: .c .o .h .l

.c.o:
	$(CC) $(CFLAGS) $(INCLUDE_FLAGS) $(GINCLUDE_FLAGS) -g -c -o $@ $<
	
%.c: %.l
	$(LEX) -o$@ $<

%.c: %.y
	$(YACC) -d -o $@ $<

%.class: %.java
	$(JAVAC) -classpath $(CLASSPATH):$(IM_MAD):$(DRMAA):$(TM_MAD) $<

#--------------------
# Targets
#--------------------

all: gwd cmds mads drmaa

#------- GW DAEMON -------

gwd: bin/gwd

#------- GW COMMANDS -------

cmds: gwps gwkill gwhistory gwsubmit gwhost gwwait gwuser gwacct

gwps: bin/gwps
gwkill: bin/gwkill
gwhistory: bin/gwhistory
gwsubmit: bin/gwsubmit
gwhost: bin/gwhost
gwwait: bin/gwwait
gwuser: bin/gwuser
gwacct: bin/gwacct

#------- GW MADS -------

mads: scheds em_mads tm_mads im_mads

scheds: bin/gw_flood_scheduler
em_mads: bin/gw_em_mad_ws bin/gw_em_mad_prews
tm_mads: bin/gw_tm_mad_ftp bin/gw_tm_mad_rft bin/gw_tm_mad_dummy
im_mads: bin/gw_im_mad_static bin/gw_im_mad_mds2 bin/gw_im_mad_mds2_glue bin/gw_im_mad_mds4

#------- DRMAA LIB -------

drmaa: lib/libdrmaa.so lib/drmaa.jar

#------- PARSER TARGETS -------

parsers: $(HOST_LEX) $(JOB_LEX) $(GWD_LEX) $(TEMPLATE_LEX)

#------- GWD RULE -------

bin/gwd: $(COBJS) $(HEADERS)
	$(CC) $(CFLAGS) -o bin/gwd $(COBJS) $(LD_FLAGS) $(LIBS)
	chmod 744 bin/gwd

#------- CMD RULES -------

bin/gwsubmit: $(HEADERS) $(CMDS)/gw_submit.o $(CMDS)/gw_cmds_common.o \
                $(CLIENT_OBJ) $(COMMON_OBJ)
	$(CC) $(CFLAGS) -o bin/gwsubmit $(CMDS)/gw_submit.o $(CMDS)/gw_cmds_common.o \
                $(CLIENT_OBJ) $(COMMON_OBJ) $(LIBS_CMD)
	chmod 755 bin/gwsubmit

bin/gwps: $(HEADERS) $(CMDS)/gw_ps.o $(CMDS)/gw_cmds_common.o \
                $(CLIENT_OBJ) $(COMMON_OBJ)
	$(CC) $(CFLAGS) -o bin/gwps $(CMDS)/gw_ps.o $(CMDS)/gw_cmds_common.o \
                $(CLIENT_OBJ) $(COMMON_OBJ) $(LIBS_CMD)
	chmod 755 bin/gwps

bin/gwhistory: $(HEADERS) $(CMDS)/gw_history.o $(CMDS)/gw_cmds_common.o \
                $(CLIENT_OBJ) $(COMMON_OBJ)
	$(CC) $(CFLAGS) -o bin/gwhistory $(CMDS)/gw_history.o $(CMDS)/gw_cmds_common.o \
        	$(CLIENT_OBJ) $(COMMON_OBJ) $(LIBS_CMD)
	chmod 755 bin/gwhistory

bin/gwkill: $(HEADERS) $(CMDS)/gw_kill.o $(CMDS)/gw_cmds_common.o \
                $(CLIENT_OBJ) $(COMMON_OBJ)
	$(CC) $(CFLAGS) -o bin/gwkill $(CMDS)/gw_kill.o $(CMDS)/gw_cmds_common.o \
                $(CLIENT_OBJ) $(COMMON_OBJ) $(LIBS_CMD)
	chmod 755 bin/gwkill

bin/gwhost: $(HEADERS) $(CMDS)/gw_host.o $(CMDS)/gw_cmds_common.o \
        	$(CLIENT_OBJ) $(COMMON_OBJ)
	$(CC) $(CFLAGS) -o bin/gwhost $(CMDS)/gw_host.o $(CMDS)/gw_cmds_common.o \
                $(CLIENT_OBJ) $(COMMON_OBJ) $(LIBS_CMD)
	chmod 755 bin/gwhost
			
bin/gwwait: $(HEADERS) $(CMDS)/gw_wait.o $(CMDS)/gw_cmds_common.o \
                $(COMMON_OBJ) $(CLIENT_OBJ)
	$(CC) $(CFLAGS) -o bin/gwwait $(CMDS)/gw_wait.o $(CMDS)/gw_cmds_common.o \
                $(CLIENT_OBJ) $(COMMON_OBJ) $(LIBS_CMD)
	chmod 755 bin/gwwait

bin/gwuser: $(HEADERS) $(CMDS)/gw_user.o $(CMDS)/gw_cmds_common.o \
                $(COMMON_OBJ) $(CLIENT_OBJ)
	$(CC) $(CFLAGS) -o bin/gwuser $(CMDS)/gw_user.o $(CMDS)/gw_cmds_common.o \
                $(CLIENT_OBJ) $(COMMON_OBJ) $(LIBS_CMD)
	chmod 755 bin/gwuser

bin/gwacct: $(HEADERS) $(CMDS)/gw_acct.o $(CMDS)/gw_cmds_common.o \
                $(COMMON_OBJ) $(DB_OBJ) $(CLIENT_ACCT_OBJ) $(CLIENT)/gw_client_init.o
	$(CC) $(CFLAGS) -o bin/gwacct $(CMDS)/gw_acct.o $(CMDS)/gw_cmds_common.o \
                $(COMMON_OBJ) $(DB_OBJ) $(CLIENT_ACCT_OBJ) $(CLIENT)/gw_client_init.o $(LD_FLAGS) $(LIBS)
	chmod 755 bin/gwacct

# ------ SCHEDULER RULE

bin/gw_flood_scheduler: $(SCHED)/gw_flood_scheduler.o $(CLIENT_OBJ) \
	 $(COMMON)/gw_common.o $(COMMON)/gw_file_parser.o $(SCHED_OBJ) $(CLIENT_ACCT_OBJ) 
	gcc	$(CFLAGS) -o bin/gw_flood_scheduler $(SCHED)/gw_flood_scheduler.o \
        $(COMMON_OBJ) $(CLIENT_OBJ) $(LIBS_CMD) $(SCHED_OBJ) $(DB_OBJ) -lm $(LD_FLAGS) $(LIBS)
	chmod 755 bin/gw_flood_scheduler

# ------ PRE-WS MAD RULES
	
bin/gw_em_mad_prews: $(EM_MAD_PREWS_OBJ)
	$(GlCC) $(GCFLAGS) $(GINCLUDE_FLAGS) -o bin/gw_em_mad_prews.bin \
        $(EM_MAD_PREWS_OBJ) $(GLD_FLAGS) $(GLIBS)
	cp -f $(EM_MAD)/gw_em_mad_prews.sh bin/gw_em_mad_prews
	chmod 755 bin/gw_em_mad_prews bin/gw_em_mad_prews.bin

bin/gw_tm_mad_ftp: $(TM_MAD_OBJ)
	$(GlCC) $(GCFLAGS) $(GINCLUDE_FLAGS) -o bin/gw_tm_mad_ftp.bin \
        $(TM_MAD_OBJ) $(GLD_FLAGS) $(GLIBS)
	cp -f $(TM_MAD)/gw_tm_mad_ftp.sh bin/gw_tm_mad_ftp
	chmod 755 bin/gw_tm_mad_ftp bin/gw_tm_mad_ftp.bin

bin/gw_tm_mad_dummy: $(TM_MAD_DUMMY_OBJ)
	$(GlCC) $(GCFLAGS) $(GINCLUDE_FLAGS) -o bin/gw_tm_mad_dummy.bin \
        $(GLD_FLAGS) $(GLIBS) $(TM_MAD_DUMMY_OBJ)
	cp -f $(TM_MAD)/gw_tm_mad_dummy.sh bin/gw_tm_mad_dummy
	chmod 755 bin/gw_tm_mad_dummy bin/gw_tm_mad_dummy.bin

# ------ WS MAD RULES
        
bin/gw_tm_mad_rft: $(TM_MAD_RFT_OBJ)
	cp -f $(TM_MAD)/*.class bin
	cp -f $(TM_MAD)/gw_tm_mad_rft.sh bin/gw_tm_mad_rft
	chmod 544 bin/*.class
	chmod 755 bin/gw_tm_mad_rft
	
bin/gw_em_mad_ws: $(EM_MAD)/GW_mad_ws.class
	cp -f $(EM_MAD)/*.class bin
	cp -f $(EM_MAD)/gw_em_mad_ws.sh bin/gw_em_mad_ws
	chmod 544 bin/*.class
	chmod 755 bin/gw_em_mad_ws
	
# ------ IMs MAD RULES

bin/gw_im_mad_mds4: $(IM_MAD_MDS4_OBJ) $(IM_MAD)/gw_im_mad_mds4.sh $(IM_MAD)/gw_im_mad_common.sh
	cp -f $(IM_MAD)/*.class bin/
	cp -f $(IM_MAD)/gw_im_mad_common.sh bin/gw_im_mad_common.sh
	cp -f $(IM_MAD)/gw_im_mad_mds4.sh bin/gw_im_mad_mds4
	chmod 744 bin/gw_im_mad_mds4
	chmod 644 bin/gw_im_mad_common.sh
	chmod 544 bin/*.class
	
bin/gw_im_mad_mds2: $(IM_MAD)/gw_im_mad_mds2.sh $(IM_MAD)/gw_im_mad_common.sh
	cp -f $(IM_MAD)/gw_im_mad_common.sh bin/gw_im_mad_common.sh
	cp -f $(IM_MAD)/gw_im_mad_mds2.sh bin/gw_im_mad_mds2
	chmod 744 bin/gw_im_mad_mds2
	chmod 644 bin/gw_im_mad_common.sh

bin/gw_im_mad_mds2_glue: $(IM_MAD)/gw_im_mad_mds2_glue.sh $(IM_MAD)/gw_im_mad_common.sh
	cp -f $(IM_MAD)/gw_im_mad_common.sh bin/gw_im_mad_common.sh
	cp -f $(IM_MAD)/gw_im_mad_mds2_glue.sh bin/gw_im_mad_mds2_glue
	chmod 744 bin/gw_im_mad_mds2_glue
	chmod 644 bin/gw_im_mad_common.sh

bin/gw_im_mad_static: $(IM_MAD)/gw_im_mad_static.sh $(IM_MAD)/gw_im_mad_common.sh
	cp -f $(IM_MAD)/gw_im_mad_common.sh bin/gw_im_mad_common.sh
	cp -f $(IM_MAD)/gw_im_mad_static.sh bin/gw_im_mad_static
	chmod 744 bin/gw_im_mad_static
	chmod 644 bin/gw_im_mad_common.sh
		
# ------- DRMAA LIB
				
lib/drmaa.jar: lib/libDrmaaJNI.so
	rm -f lib/drmaa.jar
	mkdir -p $(DRMAA_JAVA_PACKAGE)
	javac $(DRMAA_JAVA)/*.java -classpath $(DRMAA) -d $(DRMAA_JAVA_PACKAGE)
	jar cf lib/drmaa.jar -C $(DRMAA_JAVA_PACKAGE)/ .
	rm -r $(DRMAA_JAVA_PACKAGE)
	chmod 544 lib/drmaa.jar
	
lib/libDrmaaJNI.so: $(DRMAAJNI_OBJS) lib/libdrmaa.so
	rm -f lib/libDrmaaJNI.so
	ld $(DRMAAJNI_OBJS) -L lib -ldrmaa -o lib/libDrmaaJNI.so $(LDOPTIONS)
	chmod 544 lib/libDrmaaJNI.so
	
lib/libdrmaa.so: $(DRMAA_OBJS)
	rm -f lib/libdrmaa.so
#	ar -rcs lib/libdrmaa.a $(DRMAA_OBJS)
	ld $(DRMAA_OBJS) -L lib -o lib/libdrmaa.so $(LDOPTIONS) -lpthread
	chmod 544 lib/libdrmaa.so
	
clean: 
	@rm -f $(COBJS) $(CLIENT_OBJ) $(CLIENT_ACCT_OBJ) $(CMDS_OBJ) $(GWD_OBJ) \
	$(EM_MAD_PREWS_OBJ) $(TM_MAD_OBJ) $(IM_MAD_MDS4_OBJ) $(TM_MAD_RFT_OBJ) \
	$(DRMAA_OBJS) $(DRMAAJNI_OBJS) $(EM_MAD)/*.class $(SCHED)/*.o $(DRMAA_JAVA)/*.class \
	bin/*.class bin/core* var/core* bin/gw* lib/drmaa.jar lib/libdrmaa.so lib/libDrmaaJNI.so \
	$(IM_MAD)/Mds4QueryParser*.class $(TM_MAD_DUMMY_OBJ) $(SCHED_OBJ)

clean_all: clean
	@rm -fr var/[0-9]* var/.search* var/.lock var/gwd.* var/globus-gw.log var/sched.log
		
clean_parsers:
	@rm -f $(HOST)/gw_host_reqs_syntax.h $(HOST)/gw_host_update_syntax.h $(HOST)/gw_host_rank_syntax.h \
	$(HOST_LEX) $(JOB_LEX) $(GWD_LEX) $(TEMPLATE_LEX)

	
