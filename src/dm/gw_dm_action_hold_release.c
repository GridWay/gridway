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
#include "gw_job.h"
#include "gw_log.h"
#include <pthread.h>
#include <stdlib.h>

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_dm_hold (void *_job_id)
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
			gw_log_print("DM",'E',"Job %i does not exist (HOLD).\n",job_id);
				
			gw_am_trigger(gw_dm.rm_am,"GW_RM_HOLD_FAILED", _job_id);
			return;
		}
	}
	else
		return;

	/* ----------------------------------------------------------- */  
    /* 1.- Hold the job                                            */
    /* ----------------------------------------------------------- */  

    switch (job->job_state)
    {
		case GW_JOB_STATE_INIT:
		case GW_JOB_STATE_PENDING:
	        
	        gw_job_set_state(job, GW_JOB_STATE_HOLD, GW_FALSE);
	        
            gw_log_print("DM",'I',"Job %i held.\n", job_id);        
            
            gw_am_trigger(gw_dm.rm_am,"GW_RM_HOLD_SUCCESS", _job_id);
            break;
		
        default:
                                
            gw_log_print("DM",'W',"Job %i can not be held in current state.\n",
                    job_id);
            
            gw_am_trigger(gw_dm.rm_am,"GW_RM_HOLD_FAILED", _job_id);            
            break;
    }

	pthread_mutex_unlock(&(job->mutex));	
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_dm_release (void *_job_id)
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
			gw_log_print("DM",'E',"Job %i does not exist (RELEASE).\n",job_id);

            gw_am_trigger(gw_dm.rm_am,"GW_RM_RELEASE_FAILED", _job_id);
			return;
		}
	}
	else
		return;

	/* ----------------------------------------------------------- */  
    /* 1.- Release the job                                         */
    /* ----------------------------------------------------------- */  

    switch (job->job_state)
    {
		case GW_JOB_STATE_HOLD:
	        
	        gw_job_set_state(job, GW_JOB_STATE_PENDING, GW_FALSE);
	        
            gw_log_print("DM",'I',"Job %i released.\n", job_id);        
            
            gw_am_trigger(gw_dm.rm_am,"GW_RM_RELEASE_SUCCESS",  _job_id);
            break;
		
        default:
                                
            gw_log_print("DM",'W',"Job %i can not be released in current state.\n", job_id);
            
            gw_am_trigger(gw_dm.rm_am,"GW_RM_RELEASE_FAILED", _job_id);            
            break;
    }

	pthread_mutex_unlock(&(job->mutex));	
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
