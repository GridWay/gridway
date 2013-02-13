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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <signal.h>

#include "gw_dm.h"
#include "gw_log.h"
#include "gw_conf.h"
#include "gw_host_pool.h"
#include "gw_user_pool.h"


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int gw_dm_dispatch_job (int job_id, int host_id, char *queue_name, int rank)
{
    int *id;
    gw_job_t *job;
    gw_host_t *host;
    int rc;
   
    job = gw_job_pool_get(job_id, GW_TRUE);
                        
    if (job == NULL)
    {
        gw_log_print("DM",'E',"Job %i does not exist (DISPATCH).\n",job_id);
        return -1;
    }

    host = gw_host_pool_get_host(host_id, GW_TRUE);

    if (host == NULL)
    {
        gw_log_print("DM",'E',"Can't Dispatch job %i, host %i not found.\n",
                job_id, host_id);
        pthread_mutex_unlock(&(job->mutex));
        return -1;
    }

    /* -------------------------------- */
    
    if (job->job_state == GW_JOB_STATE_PENDING)
    {
        gw_log_print("DM",'I',"Dispatching job %i to %s (%s).\n",job->id, 
                host->hostname, queue_name);

        rc = gw_job_history_add(&(job->history), host, rank, queue_name,
                host->fork_name, host->lrms_name, host->lrms_type,
                job->owner, job->template.job_home, job->id, job->user_id,
                GW_FALSE);
                
        if ( rc == -1 )
        {
            gw_log_print("DM",'E',"Could not add history record for job %i.\n",
                    job_id);
                
            pthread_mutex_unlock(&(host->mutex));
            pthread_mutex_unlock(&(job->mutex));
            return -1;
        }

        /* ----- Update Host & User counters ----- */

        gw_host_inc_slots_nb(host, job->template.np);

        gw_user_pool_inc_running_jobs(job->user_id, 1);
 
        /* --------------------------------------- */
        
        id = (int *) malloc(sizeof(int));
        *id = job->id;

        gw_am_trigger(&(gw_dm.am), "GW_DM_STATE_PROLOG", (void *) id);

        pthread_mutex_unlock(&(host->mutex));        
        pthread_mutex_unlock(&(job->mutex));
    }
    else if (job->job_state == GW_JOB_STATE_WRAPPER)
    {
        if (job->history != NULL
                && job->history->reason == GW_REASON_SUSPENSION_TIME
                && job->em_state == GW_EM_STATE_ACTIVE)
        {
            gw_log_print("EM",'W',
                    "Migration of job %i aborted (suspension timeout) because it is active.\n",
                    job->id);

            pthread_mutex_unlock(&(host->mutex));
            pthread_mutex_unlock(&(job->mutex));

            gw_dm_uncheck_job(job->id);

            return -1;
        }

        gw_log_print("DM",'I',"Migrating job %i to %s (%s).\n",
                job->id, host->hostname, queue_name);

        rc = gw_job_history_add(&(job->history), host, rank, queue_name,
                host->fork_name, host->lrms_name, host->lrms_type,
                job->owner, job->template.job_home, job->id, job->user_id, 
                GW_FALSE);
 
        if ( rc == -1 )
        {
            gw_log_print("DM",'E',"Could not add history record for job %i.\n",
                    job_id);
 
            pthread_mutex_unlock(&(host->mutex));
            pthread_mutex_unlock(&(job->mutex));

            return -1;
        }

        job->restarted++;
        job->reschedule = GW_FALSE;

        /* ----- Update Host ----- */
        
        gw_host_inc_slots_nb(host, job->template.np);
 
        /* ----------------------- */

        id = (int *) malloc(sizeof(int));
        *id = job->id;

        gw_am_trigger(&(gw_dm.am), "GW_DM_STATE_MIGR_CANCEL", (void *) id);

        pthread_mutex_unlock(&(host->mutex));
        pthread_mutex_unlock(&(job->mutex));        
    }
    else
    {
        gw_log_print("DM",'E',"Can't dispatch or migrate job %i in state %s.\n",
                job->id, gw_job_state_string(job->job_state));
                
        pthread_mutex_unlock(&(host->mutex));
        pthread_mutex_unlock(&(job->mutex));                
        return -1;
    }

    return 0;
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
