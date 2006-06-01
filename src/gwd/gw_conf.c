/* -------------------------------------------------------------------------- */
/* Copyright 2002-2006 GridWay Team, Distributed Systems Architecture         */
/* Group, Universidad Complutense de Madrid                                   */
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
#include <unistd.h>
#include <pwd.h>

gw_conf_t gw_conf;

int  gw_conf_init (gw_boolean_t multiuser)
{
    int i,j;
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
    
    loc_length = strlen(gw_conf.gw_location);
    
    gw_conf.conf_file        = (char *) malloc (sizeof(char) * (loc_length + 14));
    gw_conf.template_default = (char *) malloc (sizeof(char) * (loc_length + 26));
    gw_conf.gw_globus_seg    = (char *) malloc (sizeof(char) * (loc_length + 19));
    gw_conf.gw_acct          = (char *) malloc (sizeof(char) * (loc_length + 14));
    
    if ((gw_conf.conf_file == NULL) || 
        (gw_conf.template_default == NULL) ||
        (gw_conf.gw_globus_seg == NULL) ||
        (gw_conf.gw_acct == NULL) )
    {
           return -1;
    }
        
    sprintf(gw_conf.conf_file,"%s/etc/gwd.conf",gw_conf.gw_location);    
    sprintf(gw_conf.template_default,"%s/etc/job_template.default",gw_conf.gw_location);
    sprintf(gw_conf.gw_globus_seg,"%s/var/globus-gw.log", gw_conf.gw_location);
    sprintf(gw_conf.gw_acct,"%s/var/acct/", gw_conf.gw_location);
    
    gw_conf.number_of_arrays = GW_NUMBER_OF_ARRAYS_DEFAULT;
    gw_conf.number_of_jobs   = GW_NUMBER_OF_JOBS_DEFAULT;
    gw_conf.number_of_hosts  = GW_NUMBER_OF_HOSTS_DEFAULT;    
    gw_conf.number_of_users  = GW_NUMBER_OF_USERS_DEFAULT;
    
    gw_conf.scheduling_interval = GW_SCHEDULING_INTERVAL_DEFAULT;
    gw_conf.discovery_interval  = GW_DISCOVERY_INTERVAL_DEFAULT;
    gw_conf.monitoring_interval = GW_MONITORING_INTERVAL_DEFAULT;
    gw_conf.poll_interval       = GW_POLL_INTERVAL_DEFAULT;

    gw_conf.jobs_per_sched = GW_MAX_JOBS_PER_SCHED;
    gw_conf.jobs_per_host  = GW_MAX_JOBS_PER_HOST;
    gw_conf.jobs_per_user  = GW_MAX_JOBS_PER_USER;
	
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
        gw_conf.im_mads[i] = (char **) malloc (sizeof (char *) * 6 );
        gw_conf.tm_mads[i] = (char **) malloc (sizeof (char *) * 3 );
        gw_conf.em_mads[i] = (char **) malloc (sizeof (char *) * 3 );

        if ( (gw_conf.im_mads[i] == NULL) || (gw_conf.tm_mads[i] ==NULL) || 
                (gw_conf.em_mads[i] == NULL) )
        {
            return -1;
        }
        
        for (j=0; j<6; j++)
        {
            gw_conf.im_mads[i][j]  = NULL;
        }

        for (j=0; j<3; j++)
        {
            gw_conf.tm_mads[i][j]  = NULL;    
            gw_conf.em_mads[i][j]  = NULL;                
        }
    }

    gw_conf.dm_mad = (char **) malloc (sizeof (char *) * 3 );

    for (j=0; j<3; j++)
    {
        gw_conf.dm_mad[j]  = NULL;                
    }    

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

    gw_log_print("GW",'I',"gwd.conf: Configuration files successfully loaded\n");
    gw_log_print("GW",'I'," ----------- gwd.conf values -----------\n");
    gw_log_print("GW",'I',"  GWD_PORT                         : %i\n",gw_conf.gwd_port);
    gw_log_print("GW",'I',"  MAX_NUMBER_OF_CLIENTS            : %i\n",gw_conf.max_number_of_clients);
    gw_log_print("GW",'I',"  NUMBER_OF_ARRAYS                 : %i\n",gw_conf.number_of_arrays);
    gw_log_print("GW",'I',"  NUMBER_OF_JOBS                   : %i\n",gw_conf.number_of_jobs);
    gw_log_print("GW",'I',"  NUMBER_OF_HOSTS                  : %i\n",gw_conf.number_of_hosts);
    gw_log_print("GW",'I',"  NUMBER_OF_USERS                  : %i\n",gw_conf.number_of_users);
    gw_log_print("GW",'I',"  JOBS_PER_SCHED                   : %i\n",gw_conf.jobs_per_sched);
    gw_log_print("GW",'I',"  JOBS_PER_HOST                    : %i\n",gw_conf.jobs_per_host);
    gw_log_print("GW",'I',"  JOBS_PER_USER                    : %i\n",gw_conf.jobs_per_user);
    gw_log_print("GW",'I',"  SCHEDULING_INTERVAL              : %i\n",gw_conf.scheduling_interval);
    gw_log_print("GW",'I',"  DISCOVERY_INTERVAL               : %i\n",gw_conf.discovery_interval);
    gw_log_print("GW",'I',"  MONITORING_INTERVAL              : %i\n",gw_conf.monitoring_interval);
    gw_log_print("GW",'I',"  POLL_INTERVAL                    : %i\n",gw_conf.poll_interval);

    gw_log_print("GW",'I',"  Information Manager MADs\n");  
    while ( (gw_conf.im_mads[i][0] != NULL ) && ( i < GW_MAX_MADS ) )
    {
        gw_log_print("GW",'I',"    MAD(%-1i)  name      : %s\n",i,GWNSTR(gw_conf.im_mads[i][GW_MAD_NAME_INDEX]));
        gw_log_print("GW",'I',"            executable: %s\n",GWNSTR(gw_conf.im_mads[i][GW_MAD_PATH_INDEX]));
        gw_log_print("GW",'I',"            argument  : %s\n",GWNSTR(gw_conf.im_mads[i][GW_MAD_ARGS_INDEX]));
        gw_log_print("GW",'I',"            nice      : %s\n",GWNSTR(gw_conf.im_mads[i][GW_MAD_NICE_INDEX]));
        gw_log_print("GW",'I',"            TM        : %s\n",GWNSTR(gw_conf.im_mads[i][GW_MAD_TM_INDEX]));
        gw_log_print("GW",'I',"            EM        : %s\n",GWNSTR(gw_conf.im_mads[i][GW_MAD_EM_INDEX]));
        i++;
    }

    i = 0;
    gw_log_print("GW",'I',"  Transfer Manager MADs\n");
  
    while ( (gw_conf.tm_mads[i][0] != NULL ) && ( i < GW_MAX_MADS ) )
    {
        gw_log_print("GW",'I',"    MAD(%-1i)  name      : %s\n",i,GWNSTR(gw_conf.tm_mads[i][GW_MAD_NAME_INDEX]));
        gw_log_print("GW",'I',"            executable: %s\n",GWNSTR(gw_conf.tm_mads[i][GW_MAD_PATH_INDEX]));
        gw_log_print("GW",'I',"            argument  : %s\n",GWNSTR(gw_conf.tm_mads[i][GW_MAD_ARGS_INDEX]));
        i++;
    }

    i = 0;
    gw_log_print("GW",'I',"  Execution Manager MADs\n");
  
    while ( (gw_conf.em_mads[i][0] != NULL ) && ( i < GW_MAX_MADS ) )
    {
        gw_log_print("GW",'I',"    MAD(%-1i)  name      : %s\n",i,GWNSTR(gw_conf.em_mads[i][GW_MAD_NAME_INDEX]));
        gw_log_print("GW",'I',"            executable: %s\n",GWNSTR(gw_conf.em_mads[i][GW_MAD_PATH_INDEX]));    
        gw_log_print("GW",'I',"            argument  : %s\n",GWNSTR(gw_conf.em_mads[i][GW_MAD_ARGS_INDEX]));    
        i++;
    }
  
    gw_log_print("GW",'I',"  Dispatch Manager Scheduler\n");
    gw_log_print("GW",'I',"            name      : %s\n",GWNSTR(gw_conf.dm_mad[GW_MAD_NAME_INDEX]));
    gw_log_print("GW",'I',"            executable: %s\n",GWNSTR(gw_conf.dm_mad[GW_MAD_PATH_INDEX]));    
    gw_log_print("GW",'I',"            argument  : %s\n",GWNSTR(gw_conf.dm_mad[GW_MAD_ARGS_INDEX]));    
  
    gw_log_print("GW",'I'," ---------------------------------------\n");
    return 0;
}
