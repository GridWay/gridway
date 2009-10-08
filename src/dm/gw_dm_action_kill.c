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

#include "gw_dm.h"
#include "gw_log.h"
#include <pthread.h>
#include <stdlib.h>

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_dm_kill (void *_job_id)
{
    gw_job_t *   job;
    int          job_id;
    int          rt;
    int          array_id;
    int          task_id;
    gw_array_t * array;
	char   		 conf_filename[2048];
	    
	/* ----------------------------------------------------------- */  
    /* 0.- Get job pointer                                         */
    /* ----------------------------------------------------------- */  
    
	if ( _job_id != NULL )
	{
		job_id = *( (int *) _job_id );

		job = gw_job_pool_get(job_id, GW_TRUE);

		if ( job == NULL )
		{
			gw_log_print("DM",'E',"Job %i does not exist (KILL).\n",job_id);

            gw_am_trigger(gw_dm.rm_am,"GW_RM_KILL_FAILED",  _job_id);
			return;
		}
	}
	else
		return;

	/* ----------------------------------------------------------- */  
    /* 1.- Kill the job                                            */
    /* ----------------------------------------------------------- */  
	
    switch (job->job_state)
    {
		case GW_JOB_STATE_INIT:
		case GW_JOB_STATE_PENDING:
		case GW_JOB_STATE_HOLD:
		case GW_JOB_STATE_STOPPED:
        
            job->exit_time = time(NULL);
		
		case GW_JOB_STATE_FAILED:
		case GW_JOB_STATE_ZOMBIE:
			
			array_id = job->array_id;
			task_id  = job->task_id;

			sprintf(conf_filename, "%s/job.conf", job->directory);	
			unlink(conf_filename);    
			
			pthread_mutex_unlock(&(job->mutex));
            gw_job_pool_free(job_id);
            
            gw_log_print("DM",'I',"Job %i killed and freed.\n", job_id);		

            if (array_id != -1)
            {
              array = gw_array_pool_get_array(array_id, GW_TRUE);
            
              if ( array != NULL )
              {                        
                rt = gw_array_del_task(array,task_id);
                pthread_mutex_unlock(&(array->mutex));
                
                if (rt == 0)
                {
                  gw_array_pool_array_free(array_id);
                  gw_log_print("DM",'I',"Array %i freed.\n",array_id);
                }
              }
              else
                gw_log_print("DM",'E',"Array %i does not exisit (KILL - task %i).\n",
                             array_id, task_id);
            }
            
            gw_am_trigger(gw_dm.rm_am,"GW_RM_KILL_SUCCESS",  _job_id);
            break;
		
		case GW_JOB_STATE_WRAPPER:

	        if (job->history != NULL )
	            job->history->reason = GW_REASON_KILL;
	            		
            gw_log_print("DM",'I',"Killing job %i.\n", job_id);
            
			gw_job_set_state(job, GW_JOB_STATE_KILL_CANCEL, GW_FALSE);
			    				    
			if ( job->reschedule == GW_TRUE )
			{
			    job->reschedule = GW_FALSE;
			    gw_dm_mad_job_del(&gw_dm.dm_mad[0],job->id);				
			}			
			
			gw_am_trigger(gw_dm.em_am, "GW_EM_CANCEL", _job_id);
			
            pthread_mutex_unlock(&(job->mutex));
			break;
			
        default:
            
            gw_log_print("DM",'W',"Job %i can not be killed in current state.\n", job_id);
            
            gw_am_trigger(gw_dm.rm_am,"GW_RM_KILL_FAILED",  _job_id);
            
            pthread_mutex_unlock(&(job->mutex));            
            break;
    }
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_dm_kill_hard (void *_job_id)
{
    gw_job_t *   job;
    int          job_id;
    int          rt;
    int          array_id;
    int          task_id;
    gw_array_t * array;
	char   		 conf_filename[2048];
	    
	/* ----------------------------------------------------------- */  
    /* 0.- Get job pointer                                         */
    /* ----------------------------------------------------------- */  
    
	if ( _job_id != NULL )
	{
		job_id = *( (int *) _job_id );

		job = gw_job_pool_get(job_id, GW_TRUE);

		if ( job == NULL )
		{
			gw_log_print("DM",'E',"Job %i does not exist (KILL_HARD).\n",job_id);

            gw_am_trigger(gw_dm.rm_am,"GW_RM_KILL_FAILED",_job_id);
			return;
		}
	}
	else
		return;
   
	/* ----------------------------------------------------------- */  
    /* 1.- Hard Kill the job                                       */
    /* ----------------------------------------------------------- */  
	
    switch (job->job_state)
    {
    	
		case GW_JOB_STATE_MIGR_PROLOG:
		case GW_JOB_STATE_MIGR_EPILOG:				
                        
            gw_host_dec_rjobs(job->history->next->host);
                        			
			job->history->next->stats[EXIT_TIME] = time(NULL);
						    	
    	case GW_JOB_STATE_PROLOG:
            
            gw_host_dec_uslots(job->history->host, job->template.np);
            			      
		case GW_JOB_STATE_EPILOG:
		case GW_JOB_STATE_EPILOG_STD:
		case GW_JOB_STATE_EPILOG_RESTART:
		case GW_JOB_STATE_EPILOG_FAIL:
		
			job->history->reason = GW_REASON_KILL;
			
		case GW_JOB_STATE_STOP_EPILOG:
		case GW_JOB_STATE_KILL_EPILOG:		
            
            gw_host_dec_rjobs(job->history->host);
                        						
			job->exit_time = time(NULL);
			job->history->stats[EXIT_TIME] = time(NULL);
						
	    	job->tm_state = GW_TM_STATE_HARD_KILL;

			if (job->history != NULL) 
			{
            	gw_log_print("DM",'I',"Cancelling prolog/epilog transfers of job %i.\n", job_id);
            	
				gw_tm_mad_end(job->history->tm_mad, job->id);
			}				    	
	    break;
    	
		case GW_JOB_STATE_PRE_WRAPPER:
		case GW_JOB_STATE_WRAPPER:

			job->history->reason = GW_REASON_KILL;
			
			gw_host_dec_slots(job->history->host, job->template.np);
			            
			job->exit_time = time(NULL);		
			job->history->stats[EXIT_TIME] = time(NULL);
						
			job->em_state = GW_EM_STATE_HARD_KILL;
		
			if (job->history != NULL) 
			{
            	gw_log_print("DM",'I',"Cancelling execution of job %i.\n", job_id);
            	
				gw_em_mad_cancel(job->history->em_mad, job_id);
			}			
		break;

		case GW_JOB_STATE_MIGR_CANCEL:
		
     		gw_host_dec_slots(job->history->next->host, job->template.np);
            
			job->history->next->stats[EXIT_TIME] = time(NULL);
			
			job->history->reason = GW_REASON_KILL;
			
   		case GW_JOB_STATE_STOP_CANCEL:
		case GW_JOB_STATE_KILL_CANCEL:
		
    		gw_host_dec_slots(job->history->host, job->template.np);
		            
			job->exit_time = time(NULL);		
			job->history->stats[EXIT_TIME] = time(NULL);
						
			job->em_state = GW_EM_STATE_HARD_KILL;
		break;
		
		case GW_JOB_STATE_INIT:
		case GW_JOB_STATE_PENDING:
		case GW_JOB_STATE_HOLD:
		case GW_JOB_STATE_STOPPED:
		
	        job->exit_time = time(NULL);
	     break;
            
		
		case GW_JOB_STATE_FAILED:
		case GW_JOB_STATE_ZOMBIE:
		
		break;
			
        default:
            
            gw_log_print("DM",'W',"Job %i can not be killed in current state.\n", job_id);
            
            gw_am_trigger(gw_dm.rm_am,"GW_RM_KILL_FAILED",  _job_id);
            
            pthread_mutex_unlock(&(job->mutex));            
            
        return;
    }
    
	array_id = job->array_id;
	task_id  = job->task_id;

	sprintf(conf_filename, "%s/job.conf", job->directory);	
	unlink(conf_filename);
							
	pthread_mutex_unlock(&(job->mutex));
    gw_job_pool_free(job_id);
            
    gw_log_print("DM",'I',"Job %i killed (hard) and freed.\n", job_id);		

    if (array_id != -1)
    {
    	array = gw_array_pool_get_array(array_id, GW_TRUE);
            
        if ( array != NULL )
        {                        
        	rt = gw_array_del_task(array,task_id);
            pthread_mutex_unlock(&(array->mutex));
                
            if (rt == 0)
            {
                gw_array_pool_array_free(array_id);
                gw_log_print("DM",'I',"Array %i freed.\n",array_id);
            }
         }
         else
             gw_log_print("DM",'E',"Array %i does not exisit (KILL - task %i).\n",
                          array_id, task_id);
     }
            
     gw_am_trigger(gw_dm.rm_am,"GW_RM_KILL_SUCCESS", _job_id);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
