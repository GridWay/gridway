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

void gw_dm_reschedule (void *_job_id)
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
			gw_log_print("DM",'E',"Job %i does not exist (RE-SCHEDULE).\n",job_id);

			gw_am_trigger(gw_dm.rm_am,"GW_RM_RESCHEDULE_FAILED", _job_id);
			return;
		}
	}
	else
		return;

	/* ----------------------------------------------------------- */  
    /* 1.- re-schedule job                                         */
    /* ----------------------------------------------------------- */  


    switch (job->job_state)
    {
		case GW_JOB_STATE_WRAPPER:
	        
	        job->reschedule      = GW_TRUE;
            job->history->reason = GW_REASON_USER_REQUESTED;
            	        
            gw_log_print("DM",'I',"Job %i will be re-scheduled.\n", job_id);        
            
            gw_am_trigger(gw_dm.rm_am,"GW_RM_RESCHEDULE_SUCCESS",_job_id);
            
            gw_dm_mad_job_schedule(&gw_dm.dm_mad[0],
                                   job_id,
                                   job->array_id,
                                   job->user_id,
                                   GW_REASON_USER_REQUESTED);
            break;
		
		case GW_JOB_STATE_FAILED:
		
            gw_job_set_state(job, GW_JOB_STATE_PENDING, GW_FALSE);
            
            job->tm_state = GW_TM_STATE_INIT;
            job->em_state = GW_EM_STATE_INIT;
            
            job->history->reason = GW_REASON_USER_REQUESTED;
            job->restarted++;

            gw_log_print("DM",'I',"Job %i will be re-scheduled.\n", job_id);
            
            gw_am_trigger(gw_dm.rm_am,"GW_RM_RESCHEDULE_SUCCESS",  _job_id);
            
            gw_dm_mad_job_schedule(&gw_dm.dm_mad[0],
                                   job_id,
                                   job->array_id,
                                   job->user_id,
                                   GW_REASON_NONE);            
			break;
			
        default:
                                
            gw_log_print("DM",'I',"Job %i can not be re-scheduled in current state.\n", job_id);
            
            gw_am_trigger(gw_dm.rm_am,"GW_RM_RESCHEDULE_FAILED", _job_id);            
            break;
    }

	pthread_mutex_unlock(&(job->mutex));	
}	

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
