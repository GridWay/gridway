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
#include <stdlib.h>
#include <errno.h>

#include "gw_job.h"
#include "gw_log.h"
#include "gw_dm.h"

void gw_dm_migr_cancel ( void *_job_id )
{
    gw_job_t * job;
    int        job_id;
    
	/* ----------------------------------------------------------- */  
    /* 0.- Get job pointer                                         */
    /* ----------------------------------------------------------- */  
    
	if ( _job_id != NULL )
	{
		job_id = *( (int *) _job_id );

		job = gw_job_pool_get(job_id, GW_TRUE);

		if ( job == NULL )
		{
			gw_log_print("DM",'E',"Job %i does not exist (JOB_STATE_MGR_CANCEL).\n",job_id);

			free(_job_id);
			return;
		}
	}
	else
		return;
	    
    /* ----------------------------------------------------------- */  
    /* 1.- Check we still need to migrate this job                 */
    /* ----------------------------------------------------------- */  

	if ( (job->job_state == GW_JOB_STATE_WRAPPER)
            && (job->em_state  != GW_EM_STATE_DONE))
	{
		job->history->stats[MIGRATION_START_TIME] = time(NULL);
        gw_job_set_state(job, GW_JOB_STATE_MIGR_CANCEL, GW_FALSE);

        gw_am_trigger(gw_dm.em_am, "GW_EM_CANCEL", _job_id);
	}
	else
	{
        gw_log_print("DM",'W',"Can't migrate %i to in current state.\n",job->id);
		free(_job_id);
	}	
	            
    pthread_mutex_unlock(&(job->mutex));
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
