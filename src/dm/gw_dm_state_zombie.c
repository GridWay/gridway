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
#include <stdlib.h>
#include <errno.h>

#include "gw_job.h"
#include "gw_log.h"
#include "gw_user_pool.h"
#include "gw_dm.h"

/* ----------------------------------------------------------------------------- */
/* ----------------------------------------------------------------------------- */
/* ----------------------------------------------------------------------------- */

void gw_dm_zombie ( void *_job_id )
{
    gw_job_t *   job;
    gw_array_t * array;
    int          job_id;
    int          task_id;
    int          array_id;
    int          rt;
    char         conf_filename[2048];
    time_t       prolog, epilog;

    /* ----------------------------------------------------------- */  
    /* 0.- Get job pointer                                         */
    /* ----------------------------------------------------------- */  
    
    if ( _job_id == NULL )
        return;

    job_id = *( (int *) _job_id );

    job = gw_job_pool_get(job_id, GW_TRUE);

    if ( job == NULL )
    {
        gw_log_print("DM",'E',"Job %i does not exist (JOB_STATE_ZOMBIE).\n",job_id);

        free(_job_id);
        return;
    }

    /* ----------------------------------------------------------- */  
    /* 0.- Update Job state                                        */
    /* ----------------------------------------------------------- */  
    
    switch (job->job_state)
    {
        case GW_JOB_STATE_EPILOG:
        
            gw_job_set_state(job, GW_JOB_STATE_ZOMBIE, GW_FALSE);
       
            gw_log_print("DM",'I',"Job %i done, with exit code %i.\n",job->id, job->exit_code);

            job->history->reason = GW_REASON_NONE;
            job->exit_time       = time(NULL);

            /* ------------- Print job history and send usage ------------ */

            gw_job_print(job,"DM",'I',"Job done, history:\n");            
            gw_job_print_history(job);
            gw_job_send_usage(job);
                                
            if ( job->client_waiting > 0 )
                gw_am_trigger(gw_dm.rm_am,"GW_RM_WAIT_SUCCESS", _job_id);
            else
            {
                if (gw_conf.dispose == GW_TRUE)
                    gw_am_trigger(&(gw_dm.am), "GW_DM_KILL", _job_id);
                else
                    free(_job_id);
            }

            /* -------- Update User & Host running jobs -------- */
            
            gw_user_pool_dec_running_jobs(job->user_id);
            
            gw_host_dec_rjobs(job->history->host);
            
            /* --------       Notify the scheduler      -------- */
                                  
            prolog = gw_job_history_get_prolog_time(job->history);
            epilog = gw_job_history_get_epilog_time(job->history);

            gw_dm_mad_job_success(&gw_dm.dm_mad[0],
                    job->history->host->host_id,
                    job->user_id,
                    (prolog + epilog),
                    job->history->stats[SUSPENSION_TIME],
                    job->history->stats[ACTIVE_TIME]);
                           
            pthread_mutex_unlock(&(job->mutex));

            /* -------- Update other jobs dependencies -------- */
            gw_job_pool_dep_check(job_id);

            break;
                    
        case GW_JOB_STATE_KILL_EPILOG:
            
            gw_job_set_state(job, GW_JOB_STATE_ZOMBIE, GW_FALSE);            

            job->exit_time = time(NULL);

            /* ------------- Print job history and send usage ------------ */
            
            gw_job_print(job,"DM",'I',"Job killed, history:\n");
            gw_job_print_history(job);
            gw_job_send_usage(job);

            /* ---------------- Free job & Notify RM ---------------- */
            
            array_id = job->array_id;
            task_id  = job->task_id;            

            /* -------- Update User & Host running jobs -------- */
           
            gw_user_pool_dec_running_jobs(job->user_id);

            gw_host_dec_rjobs(job->history->host);
            
            sprintf(conf_filename, "%s/job.conf", job->directory);
            unlink(conf_filename);    

            pthread_mutex_unlock(&(job->mutex));

            /* ------------------------------------------------- */            
            
            gw_job_pool_free(job_id);
            
            gw_log_print("DM",'I',"Job %i killed and freed.\n", job_id);        

            if (array_id != -1)
            {
                array = gw_array_pool_get_array(array_id,GW_TRUE);
            
                if ( array != NULL )
                {                        
                    rt = gw_array_del_task(array,task_id);
                    pthread_mutex_unlock(&(array->mutex));
                    if (rt == 0)
                    {
                        gw_array_pool_array_free(array_id);
                        gw_log_print("DM",'I',"Array %i freed\n",array_id);
                    }
                }
                else
                    gw_log_print("DM",'E',"Could not delete task %i from array %i.\n",
                            task_id, array_id);
            }
            
            gw_am_trigger(gw_dm.rm_am,"GW_RM_KILL_SUCCESS", _job_id);
            break;

        default:
            gw_log_print("DM",'E',"Zombie callback in wrong job (%i) state.\n", job_id);

            free(_job_id);
            
            pthread_mutex_unlock(&(job->mutex));
            break;
    }
}
