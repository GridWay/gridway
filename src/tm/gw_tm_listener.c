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
#include <time.h> 

#include "gw_tm.h"
#include "gw_log.h"
#include "gw_user_pool.h"

static void gw_tm_start_action_cb(gw_job_t *job, gw_tm_mad_t *tm_mad, int *job_id, char * result, char * info);

static void gw_tm_end_action_cb(gw_job_t *job, gw_tm_mad_t *tm_mad, int *job_id, char * result, char *info);

static void gw_tm_rmdir_action_cb(gw_job_t *job, gw_tm_mad_t *tm_mad, char * result, char *info);

static void gw_tm_mkdir_action_cb(gw_job_t *job, gw_tm_mad_t *tm_mad, char * result, char *info);

static void gw_tm_cp_action_cb(gw_job_t *job, int cp_xfr_id, char * result);

void gw_tm_listener(void *arg)
{
    fd_set         in_pipes;
    int            i, j, cp_xfr_id;
    int *          job_id;    
    int            greater, rc, rcm;
    char           c;
    
    char           info[GW_TM_MAX_INFO];
    char           s_job_id[GW_TM_MAX_JOB_ID];
    char           s_cp_xfr_id[GW_TM_MAX_XFR_ID];
    char           result[GW_TM_MAX_RESULT];
    char           action[GW_TM_MAX_ACTION];
    char           str[GW_TM_MAX_STRING];    
    
    int  *         fds;
    int            fd;
    int            num_fds;
    gw_job_t *     job;
    gw_tm_mad_t ** tm_mads;

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL); 
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    
    fds     = (int *) malloc(sizeof(int)*gw_conf.number_of_users*GW_MAX_MADS);
    
    tm_mads = (gw_tm_mad_t **) malloc(sizeof(gw_tm_mad_t *) * 
                                      gw_conf.number_of_users * GW_MAX_MADS);
	                                      
    while (1)
    {
   		greater = gw_user_pool_set_tm_pipes (&in_pipes, 
   		                                     fds, 
   		                                     &num_fds, 
   		                                     tm_mads, 
   		                                     gw_tm.um_tm_pipe_r);
        
        rc = select( greater+1, &in_pipes, NULL, NULL, NULL);
		
		if ( rc <= 0 )
			continue;

        for (i=0; i<num_fds; i++)
        {
            fd = fds[i];
            
            if ( FD_ISSET(fd, &in_pipes) )
            {
            	if ( fd == gw_tm.um_tm_pipe_r )
            	{
            		rc = read(fd, (void *) &c, sizeof(char));
#ifdef GWTMDEBUG            		
            		gw_log_print("TM",'D',"Updating MAD pipes (action is %c).\n",c);
#endif            		            		
            		continue;	            			
            	}
            	
                j = 0;

                do
                {
                    rc = read(fd, (void *) &c, sizeof(char));
                    str[j++] = c;   
                }                
                while ((rc > 0) && (c != '\n') && (j < (GW_TM_MAX_STRING-1)));
                
                str[j] = '\0';    

                if (rc <= 0) /* Error Reading message from MAD! */
                {
                    gw_log_print("TM",'E',"Error reading MAD message.\n");
                    
                    rcm = gw_tm_mad_reload (tm_mads[i]);
                    
                    if ( rcm == 0 )
                    {
                        gw_log_print("TM",'I',"MAD (%s) successfully reloaded\n",
                            tm_mads[i]->name);
                        
                        /* prolog/epilog needs to be restarted per job */
						
						gw_job_pool_tm_recover (gw_tm.dm_am);

                    }
                    else
                    {
                        gw_log_print("TM",'E',"Error reloading MAD (%s)\n",
                            tm_mads[i]->name);
                        
                        tm_mads[i]->mad_tm_pipe = -1;
                    }
                    continue;

                }
    
                sscanf(str,"%" GW2STR(GW_TM_MAX_ACTION) "s %"
                       GW2STR(GW_TM_MAX_JOB_ID)"s %"
                       GW2STR(GW_TM_MAX_XFR_ID)"s %"
                       GW2STR(GW_TM_MAX_RESULT)"s %"
                       GW2STR(GW_TM_MAX_INFO)"[^\n]", 
                       action, 
                       s_job_id, 
                       s_cp_xfr_id, 
                       result, 
                       info);
                
#ifdef GWTMDEBUG
                gw_log_print("TM",'D',"MAD message received: \"%s %s %s %s %s\".\n",
                        action, s_job_id, s_cp_xfr_id, result, info);                
#endif

                if (s_job_id[0] != '-')
                {
                	job_id  = (int *) malloc (sizeof(int));  	
                    *job_id = atoi(s_job_id);                    
                    
                    job = gw_job_pool_get(*job_id, GW_TRUE);

                    if (job == NULL)
                    {
                        gw_log_print("TM",'E',"MAD message for job %d, but it does not exist: \"(%s %s %s %s)\".\n",
                                     *job_id, action, s_job_id, result, info);
                        free(job_id);
                        continue;
                    }
                }
                else
                    continue;

				if ( job->tm_state == GW_TM_STATE_HARD_KILL )
				{
		            gw_log_print("TM",'W',"MAD message for job %i but it is being killed (hard).\n",
                                 *job_id);

                    free(job_id);
                    pthread_mutex_unlock(&(job->mutex));
                    continue;					
				}				

                if (strcmp(action, "START") == 0)
                {
					gw_tm_start_action_cb(job, tm_mads[i], job_id, result, info);
                }	
                else if (strcmp(action, "END") == 0)
                {
                	gw_tm_end_action_cb(job, tm_mads[i], job_id, result, info);
                }	
                else if (strcmp(action, "MKDIR") == 0)
                {
      				free(job_id);                	
					gw_tm_mkdir_action_cb(job, tm_mads[i], result, info);
                }
                else if (strcmp(action, "RMDIR") == 0)
                {
      				free(job_id);
					gw_tm_rmdir_action_cb(job, tm_mads[i], result, info);               	
                }
                else if (strcmp(action, "CP") == 0)
                {
                	cp_xfr_id = atoi (s_cp_xfr_id);
                	free(job_id);
                	
                	gw_tm_cp_action_cb(job, cp_xfr_id, result);
                }                
                
                pthread_mutex_unlock(&(job->mutex));
            }
        }    
    }
}

/* ------------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------------- */


void gw_tm_start_action_cb( gw_job_t *job, gw_tm_mad_t *tm_mad, int *job_id, char * result, char * info )
{
	switch (job->tm_state)
	{
		case GW_TM_STATE_PROLOG:
			if (strcmp(result, "FAILURE") == 0)
			{
				gw_job_print(job,"TM",'E',"Could not initialize transfer, %s.\n", info);
				
				gw_am_trigger(gw_tm.dm_am, "GW_DM_PROLOG_FAILED", (void *) job_id);
				job->tm_state = GW_TM_STATE_INIT;
			}
			else
			{
				gw_job_print(job,"TM",'I',"Creating remote job working directory:\n");
				gw_job_print(job,"TM",'I',"\tTarget url: %s.\n",job->history->rdir);

				gw_tm_mad_rmdir(tm_mad, job->id, job->history->rdir);
				free(job_id);				
			}		
			break;

		case GW_TM_STATE_EPILOG:
			if (strcmp(result, "FAILURE") == 0)
			{
				gw_job_print(job,"TM",'E',"Could not initialize transfer, %s.\n", info);
				
				gw_am_trigger(gw_tm.dm_am, "GW_DM_EPILOG_FAILED", (void *) job_id);
				job->tm_state = GW_TM_STATE_INIT;				
			}
			else
			{
				gw_tm_epilog_stage_out(job);
				free(job_id);
			}
			break;
						
		case GW_TM_STATE_CHECKPOINT:
			if (strcmp(result, "FAILURE") == 0)
			{
				gw_job_print(job,"TM",'E',"Could not initialize transfer (%s), checkpointing aborted.\n", info);
				job->tm_state = GW_TM_STATE_INIT;
			}
			else
				gw_tm_checkpoint_cp(job);
			free(job_id);		
			break;
		
		default:
			gw_job_print(job,"TM",'E',"Unexpected transfer MAD callback in START command.\n");
			free(job_id);
			break;
	}
}

/* ------------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------------- */

void gw_tm_rmdir_action_cb(gw_job_t *job, gw_tm_mad_t *tm_mad, char * result, char *info)
{
	switch (job->tm_state)
	{	
		case GW_TM_STATE_PROLOG:		
			gw_tm_mad_mkdir(tm_mad, job->id, job->history->rdir);
			break;
			
		case GW_TM_STATE_EPILOG:
		case GW_TM_STATE_EPILOG_FAILED:
		case GW_TM_STATE_PROLOG_FAILED:
		
			if (strcmp(result, "FAILURE") == 0)		
				gw_job_print(job,"TM",'E',"\tCould not remove remote dir (%s), you may need to remove it manually.\n",info);
			else        				
				gw_job_print(job,"TM",'I',"\tRemote job directory removed.\n");
						
			gw_tm_mad_end(job->history->tm_mad, job->id);
			break;
						
		default:
			gw_job_print(job,"TM",'E',"Unexpected transfer MAD callback in RMDIR command.\n");
			break;	

	}					
}

/* ------------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------------- */

void gw_tm_mkdir_action_cb(gw_job_t *job, gw_tm_mad_t *tm_mad, char * result, char *info)
{
	switch (job->tm_state)
	{	
		case GW_TM_STATE_PROLOG:
			if (strcmp(result, "FAILURE") == 0)
			{
				gw_job_print(job,"TM",'E',"\tCould not create remote job directory (%s).\n", info);
				
				job->tm_state = GW_TM_STATE_PROLOG_FAILED;
				gw_tm_mad_end(tm_mad, job->id);		
			}
			else
			{
				gw_job_print(job,"TM",'I',"\tRemote job directory created.\n");
				
				gw_tm_prolog_stage_in(job);
			}
			break;
			
		default:
			gw_job_print(job,"TM",'E',"Unexpected transfer MAD callback in MKDIR command.\n");
			break;
	}					
}

/* ------------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------------- */

void gw_tm_cp_action_cb(gw_job_t *job, int cp_xfr_id, char * result)
{
	gw_boolean_t failure;
	
	if ( strcmp(result, "FAILURE") == 0)
		failure = GW_TRUE;
	else
		failure = GW_FALSE;	
	
	switch (job->tm_state)
	{
		case GW_TM_STATE_PROLOG:						
			gw_tm_prolog_cp_cb(job, cp_xfr_id, failure);									
			break;
			
		case GW_TM_STATE_EPILOG:		
			gw_tm_epilog_cp_cb(job, cp_xfr_id, failure);
			break;
			
		case GW_TM_STATE_CHECKPOINT:		
			gw_tm_checkpoint_cp_cb(job, cp_xfr_id, failure);
			break;		
			
		case GW_TM_STATE_CHECKPOINT_CANCEL:
			break;
			
		default:
			gw_job_print(job,"TM",'E',"Unexpected transfer MAD callback in CP command.\n");
			break;		
	}
}	

/* ------------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------------- */

void gw_tm_end_action_cb( gw_job_t *job, gw_tm_mad_t *tm_mad, int *job_id, char * result, char *info )
{
	if (strcmp(result, "FAILURE") == 0)
	{
		gw_job_print(job,"TM",'W',"Could not free transfer (%s).\n");		
	}
	    
	switch (job->tm_state)
	{
		case GW_TM_STATE_PROLOG:
			gw_am_trigger(gw_tm.dm_am, "GW_DM_PROLOG_DONE", (void *) job_id);
			job->tm_state = GW_TM_STATE_INIT;
			break;
			
		case GW_TM_STATE_PROLOG_FAILED:
			gw_am_trigger(gw_tm.dm_am, "GW_DM_PROLOG_FAILED", (void *) job_id);
			job->tm_state = GW_TM_STATE_INIT;
			break;
			
		case GW_TM_STATE_EPILOG:
			gw_am_trigger(gw_tm.dm_am, "GW_DM_EPILOG_DONE", (void *) job_id);
			job->tm_state = GW_TM_STATE_INIT;		
			break;
			
		case GW_TM_STATE_EPILOG_FAILED:
			gw_am_trigger(gw_tm.dm_am, "GW_DM_EPILOG_FAILED", (void *) job_id);
			job->tm_state = GW_TM_STATE_INIT;				
			break;
			
		case GW_TM_STATE_CHECKPOINT_CANCEL:
			gw_job_print(job,"TM",'W',"Copy of checkpoint files cancelled.\n");
			
		  	gw_tm_mad_start(job->history->tm_mad, job->id);
		  	
		  	if ((job->job_state == GW_JOB_STATE_PROLOG) ||
			  	(job->job_state == GW_JOB_STATE_MIGR_PROLOG))			  	
				job->tm_state = GW_TM_STATE_PROLOG;			
			else
				job->tm_state = GW_TM_STATE_EPILOG;			
							
			free(job_id);
			break;
			
		case GW_TM_STATE_INIT:
			free(job_id);
			break;
		
		default:
			gw_job_print(job,"TM",'E',"Unexpected transfer MAD callback in END command.\n");
			free(job_id);
			break;
	}
}
