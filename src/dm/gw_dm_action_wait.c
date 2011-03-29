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

#include "gw_dm.h"
#include "gw_log.h"
#include <pthread.h>
#include <stdlib.h>

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_dm_wait (void *_job_id)
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
			gw_log_print("DM",'E',"Job %i does not exist (WAIT).\n",job_id);			
				
            gw_am_trigger(gw_dm.rm_am,"GW_RM_WAIT_FAILED", _job_id);
			return;
		}
	}
	else
		return;

	/* ----------------------------------------------------------- */  
    /* 1.- Wait for the job                                        */
    /* ----------------------------------------------------------- */  
	
	job->client_waiting++;
	
    switch (job->job_state)
    {
		case GW_JOB_STATE_ZOMBIE:
		case GW_JOB_STATE_FAILED:
            gw_am_trigger(gw_dm.rm_am,"GW_RM_WAIT_SUCCESS",  _job_id);
            break;
		
        default:
            free(_job_id);
            break;
    }

	pthread_mutex_unlock(&(job->mutex));	
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
