/* -------------------------------------------------------------------------- */
/* Copyright 2002-2012, GridWay Project Leads (GridWay.org)                   */
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
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <errno.h>

#include "gw_dm.h"
#include "gw_conf.h"
#include "gw_log.h"

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_dm_uncheck_job (int job_id);
void gw_dm_check_job (gw_job_t *job);
void gw_dm_check_job_suspension ( gw_job_t *job );
void gw_dm_check_job_rescheduling ( gw_job_t *job );

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_dm_schedule (void *_null)
{
    int i;
    gw_job_t *job;
    static int mark = 0;
    	
	mark = mark + gw_conf.scheduling_interval;
	if ( mark >= 300 )
	{
	    gw_log_print("DM",'I',"-- MARK --\n");
    	mark = 0;
	}    

   	pthread_mutex_lock(&(gw_dm.mutex));
   	
   	if ( gw_dm.scheduling == GW_FALSE )
   	{
       	gw_dm.scheduling = GW_TRUE;
		
       	pthread_mutex_unlock(&(gw_dm.mutex));
   	}
   	else
   	{
       	pthread_mutex_unlock(&(gw_dm.mutex));   	
		return;
   	}

#ifdef GWDMDEBUG   	
    gw_log_print("DM",'D',"Checking rescheduling conditions of jobs.\n");
#endif
    
    /* Check rescheduling conditions and set "rescheduled" flag for each job/task */
    for (i = 0; i<gw_conf.number_of_jobs; i++)
    {
        job = gw_job_pool_get(i, GW_TRUE);
        
        if (job != NULL)
            gw_dm_check_job(job);
    }
#ifdef GWDMDEBUG
    gw_log_print("DM",'I',"Scheduling %i jobs (%i arrays).\n",
            gw_job_pool_get_num_jobs(), gw_array_pool_get_num_arrays());
#endif
    gw_dm_mad_schedule(&gw_dm.dm_mad[0]);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_dm_check_job (gw_job_t *job)
{
    gw_migration_reason_t reason;
    
    if ( job->history != NULL)
        reason = job->history->reason;
    else
        reason = GW_REASON_NONE;

    if ( job->job_state == GW_JOB_STATE_WRAPPER && reason == GW_REASON_NONE )
    {
        gw_dm_check_job_suspension(job);

        gw_dm_check_job_rescheduling(job);
    }
        
    pthread_mutex_unlock(&(job->mutex));
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_dm_check_job_suspension ( gw_job_t *job )
{
    int job_is_not_running;
    
    time_t the_time;
    time_t total_suspension_time;
    time_t suspension_timeout;
    time_t suspension_time;
    time_t last_suspension_time;
    
    gw_em_state_t em_state;
    
    em_state           = job->em_state;
    suspension_timeout = job->template.suspension_timeout;
    
    job_is_not_running = em_state == GW_EM_STATE_PENDING
            || em_state == GW_EM_STATE_SUSPENDED;
    
    if ( suspension_timeout > 0 && job_is_not_running )
    {
        the_time = time(NULL);
        last_suspension_time  = job->history->stats[LAST_SUSPENSION_TIME];
        suspension_time       = job->history->stats[SUSPENSION_TIME];
        total_suspension_time = suspension_time + the_time - last_suspension_time;
    
        if ( total_suspension_time > suspension_timeout )
        {
            gw_job_print(job,"DM",'W',"Max. suspension time exceeded.\n");
            gw_log_print("DM",'I',"Max. suspension time of job %i exceeded.\n",
                    job->id);

            job->reschedule      = GW_TRUE;
            job->history->reason = GW_REASON_SUSPENSION_TIME;
                        
            gw_dm_mad_job_failed(&gw_dm.dm_mad[0],
                                 job->history->host->host_id,
                                 job->user_id,
                                 GW_REASON_SUSPENSION_TIME);
            
            gw_dm_mad_job_schedule(&gw_dm.dm_mad[0],
                                   job->id,
                                   job->array_id,
                                   job->user_id,
                                   GW_REASON_SUSPENSION_TIME);
        }
    }
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_dm_check_job_rescheduling ( gw_job_t *job )
{
    time_t the_time;
    time_t rescheduling_timeout;
    time_t last_rescheduling_time;
    time_t exec_time;
            
    rescheduling_timeout   = job->template.rescheduling_interval;
    last_rescheduling_time = job->last_rescheduling_time;
    
    if (rescheduling_timeout > 0) 
    {        
        the_time= time(NULL);
    
        if ( last_rescheduling_time == 0 )
        {
            job->last_rescheduling_time = the_time - gw_conf.scheduling_interval;
        }
        else if ( the_time - last_rescheduling_time > rescheduling_timeout)
        {
            gw_job_print(job,"DM",'W',"Max. rescheduling time exceeded.\n");
            gw_log_print("DM",'I',"Max. rescheduling time of job %i exceeded.\n",
                    job->id);

            exec_time = gw_job_history_get_wrapper_time(job->history);

            if ( exec_time < job->template.rescheduling_threshold )
            {
                job->reschedule      = GW_TRUE;
                job->history->reason = GW_REASON_RESCHEDULING_TIMEOUT;

                gw_dm_mad_job_schedule(&gw_dm.dm_mad[0],
                                       job->id,
                                       job->array_id,
                                       job->user_id,
                                       GW_REASON_RESCHEDULING_TIMEOUT);
            }
            else
            {
                gw_log_print("DM",'W',"Not rescheduling job %i threshold reached.\n",
                             job->id);
            }
             
            /* Set the last rescheduling time */
            job->last_rescheduling_time = the_time;
        }
    }
}

/* ------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_dm_uncheck_job (int job_id)
{
    gw_job_t *job;
    
    job = gw_job_pool_get(job_id, GW_TRUE);
        
    if ( job != NULL)
    {
	    if ((job->history != NULL) && (job->job_state != GW_JOB_STATE_PENDING))
	    {
	        job->reschedule      = GW_FALSE;
	        job->history->reason = GW_REASON_NONE;
	        
	        gw_dm_mad_job_del(&gw_dm.dm_mad[0],job->id);
	    }
	    
	    pthread_mutex_unlock(&(job->mutex));
    }
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
