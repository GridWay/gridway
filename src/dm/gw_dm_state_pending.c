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
#include "gw_user_pool.h"

void gw_dm_pending ( void *_job_id )
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
			gw_log_print("DM",'E',"Job %i does not exist (JOB_STATE_PENDING).\n",job_id);

			free(_job_id);
			return;
		}
	}
	else
		return;

    /* ----------------------------------------------------------- */  
    /* 1.- Set state                                               */
    /* ----------------------------------------------------------- */  

    gw_job_print(job,"DM",'I',"Rescheduling job.\n");
    gw_log_print("DM",'I',"Rescheduling job %d.\n", job->id);

   	if (job->history != NULL)
   		if (job->job_state != GW_JOB_STATE_EPILOG_RESTART )
		    job->history->reason = GW_REASON_EXECUTION_ERROR;
		    
    job->reschedule = GW_TRUE;
    gw_job_set_state (job, GW_JOB_STATE_PENDING, GW_FALSE);
    
	/* -------- Update Host & User running jobs -------- */
			            
    gw_user_pool_dec_running_jobs(job->user_id);

   	pthread_mutex_lock(&(job->history->host->mutex));
	    	
	job->history->host->running_jobs--;
			
	pthread_mutex_unlock(&(job->history->host->mutex));            
            
   /* ------------------------------------------------- */            
				        
    free(_job_id);
		    
    pthread_mutex_unlock(&(job->mutex));
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
