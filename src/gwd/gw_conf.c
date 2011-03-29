/* -------------------------------------------------------------------------- */
/* Copyright 2002-2011, GridWay Project Leads (GridWay.org)                   */
/*                                                                            */
/* Licensed under the Apache License, Version 2.0 (the "License"); you may    */
/* not use this file except in compliance with the License. You may obtain    */
/* a copy of the License at                                                   */
/*                                                                            */
/* http://www.apache.org/licenses/LICENSE-2.0                                 */
/*                                                                            */
/* Unless required by applicable law or agreed to in writing, software        */
/* distributed under the License is distributed on an "AS IS" BASIS,          */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   */
/* See the License for the specific language governing permissions and        */
/* limitations under the License.                                             */
/* -------------------------------------------------------------------------- */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "gw_conf.h"
#include "gw_log.h"
#include "gw_common.h"
#include "gw_job.h"

#include <unistd.h>
#include <pwd.h>

gw_conf_t gw_conf;

int  gw_conf_init (gw_boolean_t multiuser)
{
    int i;
    int loc_length;
	struct passwd *    pw_ent;
	
	gw_conf.multiuser = multiuser;
	
	pw_ent            = getpwuid(getuid());
    gw_conf.gwadmin   = strdup(pw_ent->pw_name);
    
    gw_conf.gw_location = getenv("GW_LOCATION");
    
    if ( gw_conf.gw_location == NULL)
    {
           gw_log_print("GW",'E',"Variable GW_LOCATION is not defined.\n");
           fprintf(stderr,"Variable GW_LOCATION is not defined!\n");
           return -1;
    }
    
    loc_length = strlen(gw_conf.gw_location) + sizeof(GW_ETC_DIR);
  
    gw_conf.conf_file        = (char *) malloc (sizeof(char) * (loc_length + 11));
    gw_conf.sch_conf_file    = (char *) malloc (sizeof(char) * (loc_length + 13));
    gw_conf.template_default = (char *) malloc (sizeof(char) * (loc_length + 23));
    gw_conf.gw_globus_seg    = (char *) malloc (sizeof(char) * (loc_length + 16));
    gw_conf.gw_acct          = (char *) malloc (sizeof(char) * (loc_length + 11));
    
    if ((gw_conf.conf_file == NULL) || 
        (gw_conf.template_default == NULL) ||
        (gw_conf.gw_globus_seg == NULL) ||
        (gw_conf.gw_acct == NULL) )
    {
           return -1;
    }
        
    sprintf(gw_conf.conf_file,"%s/" GW_ETC_DIR "/gwd.conf",gw_conf.gw_location);
    sprintf(gw_conf.sch_conf_file,"%s/" GW_ETC_DIR "/sched.conf",gw_conf.gw_location);    
    sprintf(gw_conf.template_default,"%s/" GW_ETC_DIR "/job_template.default",gw_conf.gw_location);
    sprintf(gw_conf.gw_globus_seg,"%s/" GW_VAR_DIR "/globus-gw.log", gw_conf.gw_location);
    sprintf(gw_conf.gw_acct,"%s/" GW_VAR_DIR "/acct/", gw_conf.gw_location);
    
    gw_conf.number_of_arrays = GW_NUMBER_OF_ARRAYS_DEFAULT;
    gw_conf.number_of_jobs   = GW_NUMBER_OF_JOBS_DEFAULT;
    gw_conf.number_of_hosts  = GW_NUMBER_OF_HOSTS_DEFAULT;    
    gw_conf.number_of_users  = GW_NUMBER_OF_USERS_DEFAULT;
    
    gw_conf.scheduling_interval = GW_SCHEDULING_INTERVAL_DEFAULT;
    gw_conf.discovery_interval  = GW_DISCOVERY_INTERVAL_DEFAULT;
    gw_conf.monitoring_interval = GW_MONITORING_INTERVAL_DEFAULT;
    gw_conf.poll_interval       = GW_POLL_INTERVAL_DEFAULT;
	
    gw_conf.max_active_im_queries = GW_MAX_ACTIVE_IM_QUERIES_DEFAULT;
    
    gw_conf.gwd_port = GW_GWD_PORT_DEFAULT;
    gw_conf.max_number_of_clients = GW_MAX_NUMBER_OF_CLIENTS_DEFAULT;
    
    gw_conf.im_mads  = (char ***) malloc (sizeof (char **) * GW_MAX_MADS );
    gw_conf.tm_mads = (char ***) malloc (sizeof (char **) * GW_MAX_MADS );
    gw_conf.em_mads  = (char ***) malloc (sizeof (char **) * GW_MAX_MADS );

    if ( gw_conf.im_mads == NULL || gw_conf.tm_mads ==NULL
            || gw_conf.em_mads == NULL )
    {
           return -1;
    }
            
    for (i=0 ; i<GW_MAX_MADS; i++)
    {
        gw_conf.im_mads[i] = (char **) malloc (sizeof (char *) * GW_MAD_IM_MAX);
        gw_conf.tm_mads[i] = (char **) malloc (sizeof (char *) * GW_MAD_TM_MAX);
        gw_conf.em_mads[i] = (char **) malloc (sizeof (char *) * GW_MAD_EM_MAX);

        if ( (gw_conf.im_mads[i] == NULL) || (gw_conf.tm_mads[i] ==NULL) || 
                (gw_conf.em_mads[i] == NULL) )
        {
            return -1;
        }
        
        gw_conf.im_mads[i][GW_MAD_IM_NAME_INDEX] = NULL;
        gw_conf.im_mads[i][GW_MAD_IM_PATH_INDEX] = NULL;
        gw_conf.im_mads[i][GW_MAD_IM_ARGS_INDEX] = NULL;
        gw_conf.im_mads[i][GW_MAD_IM_TM_INDEX]   = NULL;
        gw_conf.im_mads[i][GW_MAD_IM_EM_INDEX]   = NULL;

        gw_conf.tm_mads[i][GW_MAD_TM_NAME_INDEX] = NULL;
        gw_conf.tm_mads[i][GW_MAD_TM_PATH_INDEX] = NULL;
        gw_conf.tm_mads[i][GW_MAD_TM_ARGS_INDEX] = NULL;

        gw_conf.em_mads[i][GW_MAD_EM_NAME_INDEX] = NULL;
        gw_conf.em_mads[i][GW_MAD_EM_PATH_INDEX] = NULL;
        gw_conf.em_mads[i][GW_MAD_EM_ARGS_INDEX] = NULL;
        gw_conf.em_mads[i][GW_MAD_EM_RSL_INDEX]  = NULL;
    }

    gw_conf.dm_mad = (char **) malloc (sizeof (char *) * GW_MAD_DM_MAX );
    
    if ( gw_conf.dm_mad == NULL )
    {
            return -1;
    }
        
    gw_conf.dm_mad[GW_MAD_DM_NAME_INDEX] = NULL;
    gw_conf.dm_mad[GW_MAD_DM_PATH_INDEX] = NULL;
    gw_conf.dm_mad[GW_MAD_DM_ARGS_INDEX] = NULL;

    /* Built-int scheduling policies */
    
    gw_sch_conf_init (&(gw_conf.sch_conf));
    
    return 0;
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

int gw_loadconf ()
{
    int rc;
    int i=0;
    
    rc = gw_conf_lex_parser ();
    
    if ( rc != 0 )
        return rc;

    gw_log_print("GW",'I'," ---------------------------------------------------\n");
    gw_log_print("GW",'I',"                   gwd.conf values                  \n");
    gw_log_print("GW",'I'," ---------------------------------------------------\n");
    gw_log_print("GW",'I',"  Core configuration attributes                     \n");    
    gw_log_print("GW",'I',"    GWD_PORT                 : %i\n",gw_conf.gwd_port);
    gw_log_print("GW",'I',"    MAX_NUMBER_OF_CLIENTS    : %i\n",gw_conf.max_number_of_clients);
    gw_log_print("GW",'I',"    NUMBER_OF_ARRAYS         : %i\n",gw_conf.number_of_arrays);
    gw_log_print("GW",'I',"    NUMBER_OF_JOBS           : %i\n",gw_conf.number_of_jobs);
    gw_log_print("GW",'I',"    NUMBER_OF_HOSTS          : %i\n",gw_conf.number_of_hosts);
    gw_log_print("GW",'I',"    NUMBER_OF_USERS          : %i\n",gw_conf.number_of_users);
    gw_log_print("GW",'I',"    SCHEDULING_INTERVAL      : %i\n",gw_conf.scheduling_interval);
    gw_log_print("GW",'I',"    DISCOVERY_INTERVAL       : %i\n",gw_conf.discovery_interval);
    gw_log_print("GW",'I',"    MONITORING_INTERVAL      : %i\n",gw_conf.monitoring_interval);
    gw_log_print("GW",'I',"    POLL_INTERVAL            : %i\n",gw_conf.poll_interval);
    gw_log_print("GW",'I',"    MAX_ACTIVE_IM_QUERIES    : %i\n",gw_conf.max_active_im_queries);

    gw_log_print("GW",'I',"  Information Manager MADs\n");  
    while ( ( i < GW_MAX_MADS ) && (gw_conf.im_mads[i][0] != NULL ) )
    {
        gw_log_print("GW",'I',"    MAD(%-1i)  name  : %s\n",i,GWNSTR(gw_conf.im_mads[i][GW_MAD_IM_NAME_INDEX]));
        gw_log_print("GW",'I',"        executable: %s\n",GWNSTR(gw_conf.im_mads[i][GW_MAD_IM_PATH_INDEX]));
        gw_log_print("GW",'I',"        argument  : %s\n",GWNSTR(gw_conf.im_mads[i][GW_MAD_IM_ARGS_INDEX]));
        gw_log_print("GW",'I',"        TM        : %s\n",GWNSTR(gw_conf.im_mads[i][GW_MAD_IM_TM_INDEX]));
        gw_log_print("GW",'I',"        EM        : %s\n",GWNSTR(gw_conf.im_mads[i][GW_MAD_IM_EM_INDEX]));
        i++;
    }

    i = 0;
    gw_log_print("GW",'I',"  Transfer Manager MADs\n");
  
    while ( ( i < GW_MAX_MADS ) && (gw_conf.tm_mads[i][0] != NULL ) )
    {
        gw_log_print("GW",'I',"    MAD(%-1i)  name  : %s\n",i,GWNSTR(gw_conf.tm_mads[i][GW_MAD_TM_NAME_INDEX]));
        gw_log_print("GW",'I',"        executable: %s\n",GWNSTR(gw_conf.tm_mads[i][GW_MAD_TM_PATH_INDEX]));
        gw_log_print("GW",'I',"        argument  : %s\n",GWNSTR(gw_conf.tm_mads[i][GW_MAD_TM_ARGS_INDEX]));
        i++;
    }

    i = 0;
    gw_log_print("GW",'I',"  Execution Manager MADs\n");
  
    while ( ( i < GW_MAX_MADS ) && (gw_conf.em_mads[i][0] != NULL ) )
    {
        gw_log_print("GW",'I',"    MAD(%-1i)  name  : %s\n",i,GWNSTR(gw_conf.em_mads[i][GW_MAD_EM_NAME_INDEX]));
        gw_log_print("GW",'I',"        executable: %s\n",GWNSTR(gw_conf.em_mads[i][GW_MAD_EM_PATH_INDEX]));
        gw_log_print("GW",'I',"        argument  : %s\n",GWNSTR(gw_conf.em_mads[i][GW_MAD_EM_ARGS_INDEX]));
        gw_log_print("GW",'I',"        rsl mode  : %s\n",GWNSTR(gw_conf.em_mads[i][GW_MAD_EM_RSL_INDEX]));        
        i++;
    }
  
    gw_log_print("GW",'I',"  Dispatch Manager Scheduler\n");
    gw_log_print("GW",'I',"        name      : %s\n",GWNSTR(gw_conf.dm_mad[GW_MAD_DM_NAME_INDEX]));
    gw_log_print("GW",'I',"        executable: %s\n",GWNSTR(gw_conf.dm_mad[GW_MAD_DM_PATH_INDEX]));    
    gw_log_print("GW",'I',"        argument  : %s\n",GWNSTR(gw_conf.dm_mad[GW_MAD_DM_ARGS_INDEX]));    
  
    gw_log_print("GW",'I'," ---------------------------------------------------\n");
    
    rc = gw_sch_loadconf(&(gw_conf.sch_conf), gw_conf.sch_conf_file);
    
    if (rc != 0 )
    	return rc;
    	
	gw_log_print("GW",'I',"            sched.conf built-in policies\n");
	gw_log_print("GW",'I'," ---------------------------------------------------\n");
    gw_log_print("GW",'I',"  Scheduler configuration attributes                \n");    
    gw_log_print("GW",'I',"    DISABLE                  : %s\n",gw_conf.sch_conf.disable?"yes":"no");
    gw_log_print("GW",'I',"    DISPATCH_CHUNK           : %i\n",gw_conf.sch_conf.max_dispatch);
    gw_log_print("GW",'I',"    MAX_RUNNING_USER         : %i\n",gw_conf.sch_conf.max_user);
    gw_log_print("GW",'I',"    MAX_RUNNING_RESOURCE     : %i\n",gw_conf.sch_conf.max_resource);
	
	gw_log_print("GW",'I',"  Job Fixed Priority Policy\n");
	gw_log_print("GW",'I',"    FP_WEIGHT                : %-8.2f\n",gw_conf.sch_conf.wfixed);	
	gw_log_print("GW",'I',"    Fixed Priority Values (users)\n");
	gw_log_print("GW",'I',"      DEFAULT                : %i\n",gw_conf.sch_conf.ufixed_default);	
	for (i = 0 ; i < gw_conf.sch_conf.nufixed ; i++)
		gw_log_print("GW",'I',"      %-16s       : %i\n",gw_conf.sch_conf.ufixed[i].name,gw_conf.sch_conf.ufixed[i].value);
	
	if ( gw_conf.sch_conf.ngfixed > 0 )
	{
	    gw_log_print("GW",'I',"    Fixed Priority Values (groups)\n");
	    for (i = 0 ; i < gw_conf.sch_conf.ngfixed ; i++)
		    gw_log_print("GW",'I',"      %-16s       : %i\n",gw_conf.sch_conf.gfixed[i].name,gw_conf.sch_conf.gfixed[i].value);
	}

	gw_log_print("GW",'I',"  Job Share Policy\n");
	gw_log_print("GW",'I',"    SH_WEIGHT (share)        : %-8.2f\n",gw_conf.sch_conf.wshare);
	gw_log_print("GW",'I',"    SH_WINDOW_SIZE           : %-8.2f\n",gw_conf.sch_conf.window_size);
	gw_log_print("GW",'I',"    SH_WINDOW_DEPTH          : %i\n",gw_conf.sch_conf.window_depth);	
	gw_log_print("GW",'I',"    User Shares\n");
	gw_log_print("GW",'I',"      DEFAULT                : %i\n",gw_conf.sch_conf.ushare_default);	
	for (i = 0 ; i < gw_conf.sch_conf.nushare ; i++)
		gw_log_print("GW",'I',"      %-16s       : %i\n",gw_conf.sch_conf.ushare[i].name,gw_conf.sch_conf.ushare[i].value);

	gw_log_print("GW",'I',"  Job Waiting time Policy\n");
	gw_log_print("GW",'I',"    WT_WEIGHT                : %-8.2f\n",gw_conf.sch_conf.wwaiting);

	gw_log_print("GW",'I',"  Job Deadline Policy\n");
	gw_log_print("GW",'I',"    DL_WEIGHT (deadline)     : %-8.2f\n",gw_conf.sch_conf.wdeadline);
	gw_log_print("GW",'I',"    DL_HALF                  : %i\n",gw_conf.sch_conf.dl_half);
	

	gw_log_print("GW",'I',"  Resource Fixed Priority Policy\n");
	gw_log_print("GW",'I',"    RP_WEIGHT                : %-8.2f\n",gw_conf.sch_conf.wrfixed);
	gw_log_print("GW",'I',"    Fixed Priority Values (information managers)\n");
	gw_log_print("GW",'I',"      DEFAULT                : %i\n",gw_conf.sch_conf.rfixed_default);
	for (i = 0 ; i < gw_conf.sch_conf.nifixed ; i++)
		gw_log_print("GW",'I',"      %-23s: %i\n",gw_conf.sch_conf.ifixed[i].name,gw_conf.sch_conf.ifixed[i].value);
	
	if ( gw_conf.sch_conf.nhfixed > 0 )
	{
	    gw_log_print("GW",'I',"    Fixed Priority Values (hosts)\n");
	    for (i = 0 ; i < gw_conf.sch_conf.nhfixed ; i++)
		    gw_log_print("GW",'I',"      %-23s: %i\n",gw_conf.sch_conf.hfixed[i].name,gw_conf.sch_conf.hfixed[i].value);
	}
	
	gw_log_print("GW",'I',"  Resource Failure Rate Policy\n");
	gw_log_print("GW",'I',"    RA_WEIGHT                : %-8.2f\n",gw_conf.sch_conf.wrank);
	
		
	gw_log_print("GW",'I',"  Resource Failure Rank Policy\n");	
	gw_log_print("GW",'I',"    FR_MAX_BANNED            : %i\n",gw_conf.sch_conf.fr_max_banned);
	gw_log_print("GW",'I',"    FR_BANNED_C              : %-8.2f\n",gw_conf.sch_conf.fr_banned_c);
	
	gw_log_print("GW",'I',"  Resource Usage Policy\n");	
	gw_log_print("GW",'I',"    UG_WEIGHT                : %-8.2f\n",gw_conf.sch_conf.wusage);
	gw_log_print("GW",'I',"    UG_HISTORY_WINDOW        : %-8.2f\n",gw_conf.sch_conf.ug_window);
	gw_log_print("GW",'I',"    UG_HISTORY_RATIO         : %-8.2f\n",gw_conf.sch_conf.ug_ratio);	
	
    gw_log_print("GW",'I'," ---------------------------------------------------\n");
        	
    return 0;
}

