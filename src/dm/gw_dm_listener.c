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

#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>

#include "gw_dm.h"
#include "gw_host.h"
#include "gw_host_pool.h"
#include "gw_log.h"
#include "gw_user_pool.h"

void gw_dm_listener(void *arg)
{
    fd_set  in_pipes;
    int     i, j,num_mads;
    int     greater, rc, rcm;
    char    c;
    char    info[GW_DM_MAX_INFO];
    char    s_id[GW_DM_MAX_ID];
    char    result[GW_DM_MAX_RESULT];
    char    action[GW_DM_MAX_ACTION];
    char    str[GW_DM_MAX_STRING];
    char    *s_host_id, *queue_name, *s_rank, *lasts;
    int     job_id, host_id, rank;
    gw_dm_mad_t * dm_mad;
    
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL); 
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    
    while (1)
    {
        greater = gw_dm_set_pipes (&in_pipes, &num_mads);
                
        select( greater+1, &in_pipes, NULL, NULL, NULL);

        for (i= 0; i<=greater; i++) 
        {
            if ( FD_ISSET(i, &in_pipes) )
            {                
                dm_mad = gw_dm_get_mad_by_fd(i);
                
                if ( dm_mad == NULL )
                    continue;
                                                
                j = 0;

                do
                {
                    rc = read(i, (void *) &c, sizeof(char));
                    str[j++] = c;    
                }                
                while ((rc > 0) && (c != '\n') && (j < (GW_DM_MAX_STRING-1)));
                
                str[j] = '\0';    

                if (rc <= 0) /* Error Reading message from MAD! */
                {
                    gw_log_print("DM",'W',"Error reading MAD (%s) message\n",
                            dm_mad->name);
                    
                    pthread_mutex_lock(&(gw_dm.mutex));
                    
                    gw_dm.scheduling = GW_TRUE; /* Prevents to call the scheduler while recovering*/
                    
                    pthread_mutex_unlock(&(gw_dm.mutex));                    
                    
                    rcm = gw_dm_mad_reload(dm_mad);
                    
                    if ( rcm == 0 )
                    {
                        gw_log_print("DM",'I',"MAD (%s) successfully reloaded\n",
                            dm_mad->name);
                        
                        gw_user_pool_dm_recover (dm_mad);
                        
                        gw_host_pool_dm_recover (dm_mad);
                        
                        gw_job_pool_dm_recover  (dm_mad);
                    }
                    else
                    {
                        gw_log_print("DM",'E',"Error reloading the scheduler (%s)\n",
                            dm_mad->name);
                            
                        dm_mad->mad_dm_pipe = -1;
                        
                        if ( num_mads == 1 )
                        {
                            gw_log_print("DM",'E',"GridWay needs to be restarted! (no scheduler)\n");
                            
                            return;
                        }
                    }
                    
                    pthread_mutex_lock(&(gw_dm.mutex));
                    
                    gw_dm.scheduling = GW_FALSE; /* Activate the scheduler again*/
                    
                    pthread_mutex_unlock(&(gw_dm.mutex));                    
                    
                    continue;
                }
                
                info[0] = '\0';

                sscanf(str,"%" GW2STR(GW_DM_MAX_ACTION) "s %"
                       GW2STR(GW_DM_MAX_ID) "s %"
                       GW2STR(GW_DM_MAX_RESULT) "s %"
                       GW2STR(GW_DM_MAX_INFO) "[^\n]", 
                       action, 
                       s_id, 
                       result, 
                       info);
  
#ifdef GWDMDEBUG
                gw_log_print("DM",'D',"MAD (%s) message %s %s %s %s (info length=%d).\n",
                        dm_mad->name, 
                        action, 
                        s_id, 
                        result,
                        info, 
                        strlen(info));
#endif
                if (strcmp(action, "SCHEDULE_JOB") == 0)
                {
                    if (strcmp(result, "SUCCESS") == 0)
                    {
                        job_id     = atoi(s_id);
                        s_host_id  = strtok_r(info, ":", &lasts);
                        queue_name = strtok_r(NULL, ":", &lasts);
                        s_rank     = lasts;
                        
                        if (s_host_id == NULL || queue_name == NULL || s_rank == NULL)
                        {
                            gw_log_print("DM",'E',"Bad resource specification (%s) from scheduler.\n",
                                         info);
                            continue;
                        }

                        host_id = atoi(s_host_id);
                        rank    = atoi(s_rank);
                        
                        gw_dm_dispatch_job(job_id, host_id, queue_name, rank);
                    }
                    else
                    {
                        gw_log_print("DM",'E',"Can't schedule job %s: %s.\n", s_id, info);

                        job_id = atoi(s_id);
                        
                        gw_dm_uncheck_job(job_id);
                    }
                }
                else if (strcmp(action, "SCHEDULE_END") == 0)
                {
                	pthread_mutex_lock(&(gw_dm.mutex));
                	
                	gw_dm.scheduling = GW_FALSE;
                	
                	pthread_mutex_unlock(&(gw_dm.mutex));	
                }
                else
                {
                    gw_log_print("DM",'E',"Unknown MAD action %s.\n", action);
                }
            }
        }    
    }
}
