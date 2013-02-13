/* -------------------------------------------------------------------------- */
/* Copyright 2002-2013, GridWay Project Leads (GridWay.org)                   */
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
	
void gw_dm_stop (void *_job_id)
{
    gw_job_t *   job;
    int          job_id;
    
	/* ----------------------------------------------------------- */  
    /* 0.- Get job pointer                                         */
    /* ----------------------------------------------------------- */  
    
	if ( _job_id != NULL )
	{
		job_id = *( (int *) _job_id );

		job = gw_job_pool_get(job_id, GW_TRUE);

		if ( job == NULL )
		{
			gw_log_print("DM",'E',"Job %i does not exist (STOP).\n",job_id);

			gw_am_trigger(gw_dm.rm_am,"GW_RM_STOP_FAILED",  _job_id);
			return;
		}
	}
	else
		return;

	/* ----------------------------------------------------------- */  
    /* 1.- Stop the job                                            */
    /* ----------------------------------------------------------- */  
	
    switch (job->job_state)
    {
		case GW_JOB_STATE_STOPPED:
            
            gw_log_print("DM",'W',"Job %i already stopped.\n", job_id);
            
            gw_am_trigger(gw_dm.rm_am,"GW_RM_STOP_FAILED",  _job_id);
            break;
                        				
		case GW_JOB_STATE_WRAPPER:
		
	        if (job->history != NULL )
	            job->history->reason = GW_REASON_STOP_RESUME;

            gw_log_print("DM",'I',"Stopping job %i.\n", job_id);
            
			gw_job_set_state(job, GW_JOB_STATE_STOP_CANCEL, GW_FALSE);
			
			if ( job->reschedule == GW_TRUE )
			{
			    job->reschedule = GW_FALSE;
			    gw_dm_mad_job_del(&gw_dm.dm_mad[0],job->id);				
			}			
						
			gw_am_trigger(gw_dm.em_am, "GW_EM_CANCEL", _job_id);
			break;
			
        default:
                    
            gw_log_print("DM",'W',"Job %i can not be stopped in current state.\n", job_id);
            
            gw_am_trigger(gw_dm.rm_am,"GW_RM_STOP_FAILED",  _job_id);
            break;
    }

	pthread_mutex_unlock(&(job->mutex));
}	

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_dm_resume (void *_job_id)
{
    gw_job_t *   job;
    int          job_id;
    
	/* ----------------------------------------------------------- */  
    /* 0.- Get job pointer                                         */
    /* ----------------------------------------------------------- */  
    
	if ( _job_id != NULL )
	{
		job_id = *( (int *) _job_id );

		job = gw_job_pool_get(job_id, GW_TRUE);

		if ( job == NULL )
		{
			gw_log_print("DM",'E',"Job %i does not exist (RESUME).\n",job_id);
				
            gw_am_trigger(gw_dm.rm_am,"GW_RM_RESUME_FAILED", _job_id);
			return;
		}
	}
	else
		return;

	/* ----------------------------------------------------------- */  
    /* 1.- Resume the job                                          */
    /* ----------------------------------------------------------- */  

    switch (job->job_state)
    {
		case GW_JOB_STATE_STOPPED:
		
            job->restarted++;

	        gw_job_set_state(job, GW_JOB_STATE_PENDING, GW_FALSE);
	        job->tm_state  = GW_TM_STATE_INIT;
	        job->em_state  = GW_EM_STATE_INIT;
	        
            gw_log_print("DM",'I',"Job %i resumed.\n", job_id);        
            
            gw_am_trigger(gw_dm.rm_am,"GW_RM_RESUME_SUCCESS",  _job_id);
                        
            gw_dm_mad_job_schedule(&gw_dm.dm_mad[0],
                                   job_id,
                                   job->array_id,
                                   job->user_id,
                                   GW_REASON_NONE); 
            break;
		
        default:
                                
            gw_log_print("DM",'W',"Job %i can not be resumed in current state.\n", job_id);
            
            gw_am_trigger(gw_dm.rm_am,"GW_RM_RESUME_FAILED", _job_id);            
            break;
    }

	pthread_mutex_unlock(&(job->mutex));	
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
