/* -------------------------------------------------------------------------- */
/* Copyright 2002-2010, GridWay Project Leads (GridWay.org)                   */
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
#include <time.h> 
#include <limits.h>

#include "gw_em.h"
#include "gw_log.h"
#include "gw_user_pool.h"

void gw_em_listener(void *arg)
{
    fd_set       in_pipes;
    int          i,j;
    int *        job_id;    
    int          greater, rc, rcm;
    char         c;
 
    char         info[GW_EM_MAX_INFO];
    char         s_job_id[GW_EM_MAX_JOB_ID];
    char         result[GW_EM_MAX_RESULT];
    char         action[GW_EM_MAX_ACTION];
    char         str[GW_EM_MAX_STRING];
 
    int          fd;
    gw_job_t *   job;

    char         contact_file[PATH_MAX];
    FILE         *file;

    gw_boolean_t  assume_done;
    gw_em_mad_t * em_mad;
 
    char *ptmp;

    int  *         fds;    
    int            num_fds;
    gw_em_mad_t ** em_mads;
    
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL); 
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    fds     = (int *) malloc(sizeof(int)*gw_conf.number_of_users*GW_MAX_MADS);
    
    em_mads = (gw_em_mad_t **) malloc(sizeof(gw_em_mad_t *) * 
                                      gw_conf.number_of_users * GW_MAX_MADS);
                                      	
    while (1)
    {
        greater = gw_user_pool_set_em_pipes (&in_pipes,
                                             fds, 
                                             &num_fds, 
                                             em_mads, 
                                             gw_em.um_em_pipe_r);
                                                     
        rc = select( greater+1, &in_pipes, NULL, NULL, NULL);

        if ( rc <= 0 )
            continue;
        
        for (i=0; i<num_fds; i++)
        {   
            fd = fds[i];
            
            if ( FD_ISSET(fd, &in_pipes) )
            {
            	if ( fd == gw_em.um_em_pipe_r )
            	{
                    rc = read(fd, (void *) &c, sizeof(char));
#ifdef GWEMDEBUG
                    gw_log_print("EM",'D',"Updating MAD pipes (action is %c)\n",c);
#endif					
                    continue;	
            	}

#ifdef GWEMDEBUG
                    gw_log_print("EM",'D',"Reading from MAD pipe %i.\n",i);
#endif
            	
                j = 0;

                do
                {
                    rc = read(fd, (void *) &c, sizeof(char));
                    str[j++] = c;    
                }
                while ((rc > 0) && (c != '\n') && (j < (GW_EM_MAX_STRING-1)));

                str[j] = '\0';

                if (rc <= 0)
                {					
                    gw_log_print("EM",'W',"Error reading MAD (%s) message\n",
                            em_mads[i]->name);
                    
                    rcm = gw_em_mad_reload (em_mads[i]);
                    
                    if ( rcm == 0 )
                    {
                        gw_log_print("EM",'I',"MAD (%s) successfully reloaded\n",
                            em_mads[i]->name);
                        
                        gw_job_pool_em_recover(em_mads[i]);
                    }
                    else
                    {
                        gw_log_print("EM",'E',"Error reloading MAD (%s)\n",
                            em_mads[i]->name);
                        
                        em_mads[i]->mad_em_pipe = -1;
                    }
                    continue;
                }

                sscanf(str,"%" GW2STR(GW_EM_MAX_ACTION) "s %" 
                       GW2STR(GW_EM_MAX_JOB_ID) "s %" 
                       GW2STR(GW_EM_MAX_RESULT) "s %" 
                       GW2STR(GW_EM_MAX_INFO) "[^\n]", 
                       action, 
                       s_job_id, 
                       result, 
                       info);

#ifdef GWEMDEBUG
                gw_log_print("EM",'D',"MAD message received:\"%s %s %s %s\".\n",
                             action, s_job_id, result, info);
#endif                      
                if (s_job_id[0] == '-')
                    continue;

                job_id = (int *) malloc (sizeof(int));
                	
                *job_id = atoi(s_job_id);
                    
                job = gw_job_pool_get(*job_id, GW_TRUE);

                if (job == NULL)
                {
                    gw_log_print("EM",'W',"MAD message for job %s, but it does not exist: \"%s %s %s %s\".\n",
                                 s_job_id,action, s_job_id, result, info);
                    free(job_id);
                        
                    continue;
                }
                if ((job->job_state != GW_JOB_STATE_PRE_WRAPPER)
                        && (job->job_state != GW_JOB_STATE_WRAPPER)
                        && (job->job_state != GW_JOB_STATE_MIGR_CANCEL)
                        && (job->job_state != GW_JOB_STATE_STOP_CANCEL)
                        && (job->job_state != GW_JOB_STATE_KILL_CANCEL))
		{
                    gw_log_print("EM",'W',"MAD message for job %i but not in an execution state.\n",
                            *job_id);

                    free(job_id);
                    pthread_mutex_unlock(&(job->mutex));
                    continue; 		
                }
                else if ( job->em_state == GW_EM_STATE_HARD_KILL )
                {
                    gw_log_print("EM",'W',"MAD message for job %i but it is being killed (hard).\n",
                            *job_id);

                    free(job_id);
                    pthread_mutex_unlock(&(job->mutex));
                    continue; 		
                }

                if (strcmp(action, "SUBMIT") == 0)
                {
                    if (strcmp(result, "FAILURE") == 0)
                    {
                        gw_job_print(job, "EM",'E',"Job submission failed: %s\n",
                                info);
                                    
                        gw_log_print("EM",'E',"Submission of job %d failed: %s.\n",
                                job->id, info);

                        gw_am_trigger(&(gw_em.am), "GW_EM_STATE_FAILED",
                                (void *) job_id); 
                    }
                    else /* Save persistent job contact */
                    {
                        snprintf(contact_file, 
                                 PATH_MAX-1, 
                                 "%s/" GW_VAR_DIR "/%i/job.contact",
                                 gw_conf.gw_location, 
                                 job->id);
                                 
                        file = fopen(contact_file, "w");
                        
                        if ( file != NULL )
                        {
                            fprintf(file, "%s\n", info);
                            fclose(file);
                        }

                        gw_am_trigger(&(gw_em.am), "GW_EM_STATE_PENDING",
                                    (void *) job_id);
                    }
                }                
                else if (strcmp(action, "CANCEL") == 0)
                {
                    if (strcmp(result, "FAILURE") == 0)
                    {
                        gw_job_print(job, "EM",'E',"Job cancel failed (%s), assuming the job is done.\n",info);
                        gw_log_print("EM",'E',"Cancel of job %d failed: %s.\n",job->id, info);

	                    gw_am_trigger(&(gw_em.am), "GW_EM_STATE_DONE",(void *) job_id);
                    }
                    else
                    	free(job_id);
                }
                else if (strcmp(action, "POLL") == 0)
                {
                    if (strcmp(result, "SUCCESS") == 0)
                    {
                        if (strcmp(info, "PENDING") == 0)
                            gw_am_trigger(&(gw_em.am), "GW_EM_STATE_PENDING",
                                    (void *) job_id); 
                                        
                        else if (strcmp(info, "SUSPENDED") == 0)
                            gw_am_trigger(&(gw_em.am), "GW_EM_STATE_SUSPENDED",
                                    (void *) job_id);
                                        
                        else if (strcmp(info, "ACTIVE") == 0)
                            gw_am_trigger(&(gw_em.am),"GW_EM_STATE_ACTIVE",
                                    (void *) job_id);
                                
                        else if (strstr(info, "DONE") != NULL)
                        {
                        	ptmp = strstr(info,"DONE:");
                             	
                            if ((ptmp != NULL) && (strlen(ptmp+5) > 0))/*No-wrapper mode*/
	                           	job->exit_code=atoi(ptmp+5);

                             gw_am_trigger(&(gw_em.am), "GW_EM_STATE_DONE",
	                                    (void *) job_id);
                        }                
                        else if (strcmp(info, "FAILED") == 0)
                            gw_am_trigger(&(gw_em.am), "GW_EM_STATE_FAILED",
                                    (void *) job_id);
                    } 
                    else 
                    {						
                        job->history->failed_polls++;
						
                        if (job->history->failed_polls == 3 )
                            assume_done = GW_TRUE;
                        else
                            assume_done = GW_FALSE;
						
                        em_mad = job->history->em_mad;
							                    
                    	if ( assume_done )
                        {
                            gw_job_print(job, "EM",'E',"Job poll failed (%s), assuming the job is done.\n",info);
                                                
                            gw_am_trigger(&(gw_em.am), "GW_EM_STATE_DONE", (void *) job_id);
                        }
                        else
                        {
                            gw_job_print(job, "EM",'E',"Job poll failed (%s), will poll again.\n",info);
							
                            free(job_id);                     
                        }
                    }
                }
                else if (strcmp(action, "RECOVER") == 0)
                {
                    if (strcmp(result, "SUCCESS") == 0)
                    {
                        if (strcmp(info, "PENDING") == 0)
                            gw_am_trigger(&(gw_em.am), "GW_EM_STATE_PENDING",
                                    (void *) job_id); 
                        else if (strcmp(info, "SUSPENDED") == 0)
                            gw_am_trigger(&(gw_em.am), "GW_EM_STATE_SUSPENDED",
                                    (void *) job_id);
                        else if (strcmp(info, "ACTIVE") == 0)
                            gw_am_trigger(&(gw_em.am),"GW_EM_STATE_ACTIVE",
                                    (void *) job_id);
                        else if (strstr(info, "DONE") != NULL)
                        {
                        	ptmp = strstr(info,"DONE:");
                             	
                            if ((ptmp != NULL) && (strlen(ptmp+5) > 0))/*No-wrapper mode*/
                                job->exit_code = atoi(ptmp+5);

                            gw_am_trigger(&(gw_em.am), "GW_EM_STATE_DONE",
                                    (void *) job_id);
                        }            
                        else if (strcmp(info, "FAILED") == 0)
                            gw_am_trigger(&(gw_em.am), "GW_EM_STATE_FAILED",
                                    (void *) job_id);
                    } 
                    else 
                    {
                        gw_job_print(job,"EM",'E',"Job recover failed (%s), assuming the job is done.\n", info);
                        
                        gw_log_print("EM",'E',"Recover of job %i failed.\n", *job_id);
                                                
                        gw_am_trigger(&(gw_em.am), "GW_EM_STATE_DONE", (void *) job_id);
                    }
                }
                else if (strcmp(action, "CALLBACK") == 0)
                {
                    if (strcmp(result, "SUCCESS") == 0)
                    {
                        if (strcmp(info, "PENDING") == 0)
                            gw_am_trigger(&(gw_em.am),"GW_EM_STATE_PENDING",
                            	(void *) job_id);
                                    
                        else if (strcmp(info, "SUSPENDED") == 0)
                            gw_am_trigger(&(gw_em.am),"GW_EM_STATE_SUSPENDED",
                            	(void *) job_id);
                                
                        else if (strcmp(info, "ACTIVE") == 0)
                            gw_am_trigger(&(gw_em.am),"GW_EM_STATE_ACTIVE", 
                            	(void *) job_id);
                                    
                        else if (strstr(info, "DONE") != NULL)
                        {
                        	ptmp = strstr(info,"DONE:");
                             	
                            if ((ptmp != NULL) && (strlen(ptmp+5) > 0))/*No-wrapper mode*/
	                           	job->exit_code=atoi(ptmp+5);

                            gw_am_trigger(&(gw_em.am), "GW_EM_STATE_DONE",
                                    (void *) job_id);
                        }    
                                    
                        else if (strcmp(info, "FAILED") == 0)/* user cancelled */
                        {
                            gw_job_print(job, "EM",'I',"Job cancelled (%s).\n", info);

                            gw_log_print("EM",'I',"Job %i cancelled (%s).\n",*job_id, info);
                                    
                            gw_am_trigger(&(gw_em.am), "GW_EM_STATE_DONE",
                            	(void *) job_id);
                        }
                    }
                    else
                    {
                        gw_job_print(job, "EM",'E',"Job callback failed (%s).\n", info);

                        gw_log_print("EM",'E',"Callback of job %i failed: %s.\n",
                                *job_id, info);
		                
                        if ( (job->job_state == GW_JOB_STATE_MIGR_CANCEL)||
                                (job->job_state == GW_JOB_STATE_STOP_CANCEL)||
                                (job->job_state == GW_JOB_STATE_KILL_CANCEL))
                            gw_am_trigger(&(gw_em.am), "GW_EM_STATE_DONE",
                                    (void *) job_id);
                        else
                            gw_am_trigger(&(gw_em.am), "GW_EM_STATE_FAILED",
                                    (void *) job_id);
                    }
                }
                
                pthread_mutex_unlock(&(job->mutex));            	
            }
        }
    }
}
