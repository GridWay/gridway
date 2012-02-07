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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>

#include "gw_em.h"
#include "gw_em_rsl.h"
#include "gw_log.h"
#include "gw_conf.h"


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_em_submit(void *_job_id)
{
    int           job_id;
    gw_job_t      *job;
    char          *rsl=NULL;
    char          *contact;
    gw_job_state_t state;
    char          rsl_filename[2048];
    FILE          *fd;
    time_t        now;

    /* ----------------------------------------------------------- */  
    /* 0.- Get job pointer, check if it exits and lock mutex       */
    /* ----------------------------------------------------------- */  

    if ( _job_id != NULL )
    {
        job_id = *( (int *) _job_id );
        job = gw_job_pool_get(job_id, GW_TRUE);

        if ( job == NULL )
        {
            gw_log_print("EM",'E',"Job %s no longer exists (PENDING).\n",
                    job_id);
            return;
        }
    }
    else
        return;

    if (job->history == NULL) 
    {
        gw_log_print("EM",'E',"History of job %s doesn't exists\n",
                job_id);
        free(_job_id);
        pthread_mutex_unlock(&(job->mutex));
        return;
    }

    state = job->job_state;

    /* ----------------------------------------------------------- */  
    /* 1.- Get execution MAD for this host                         */
    /* ----------------------------------------------------------- */  

    job->em_state = GW_EM_STATE_INIT;
    job->history->counter = -1;    

    if ( job->job_state == GW_JOB_STATE_PRE_WRAPPER )
    {
        contact = job->history->em_fork_rc;
        rsl     = (char *) job->history->em_mad->pre_wrapper_rsl((void *) job);    
    }
    else
    {
        contact = job->history->em_rc;
        rsl     = (char *) job->history->em_mad->wrapper_rsl((void *) job);
    }

    if ( rsl == NULL )
    {
        job->em_state = GW_EM_STATE_FAILED;
        
        gw_log_print("EM",'E',"Job %i failed, could not generate RSL.\n", job_id);        
        gw_am_trigger(gw_em.dm_am, "GW_DM_WRAPPER_FAILED", _job_id);
        
        pthread_mutex_unlock(&(job->mutex));
        
        return;
    }
    
    sprintf(rsl_filename, "%s/job.rsl.%i", job->directory,job->restarted);
    
    fd = fopen(rsl_filename,"w");
    if (fd != NULL )
    {
        gw_job_print(job,"EM",'I',"Submitting wrapper to %s, RSL used is in %s.\n",contact,rsl_filename);
        fprintf(fd,"%s",rsl);
        fclose(fd);
    }
    else
    {
        job->em_state = GW_EM_STATE_FAILED;
        
        gw_log_print("EM",'E',"Job %i failed, could not open RSL file.\n", job_id);
        gw_job_print(job,"EM",'E',"Job failed, could not open RSL file %s.\n",rsl_filename);

        gw_am_trigger(gw_em.dm_am, "GW_DM_WRAPPER_FAILED", _job_id);
        
        pthread_mutex_unlock(&(job->mutex));
        
        return;
    }

    /* -------------------------------------------------------------------- */

    now = time(NULL);

    job->next_poll_time = now + gw_conf.poll_interval/2 
            + gw_rand(gw_conf.poll_interval);            /* randomize polls */

    gw_job_print(job,"EM",'I',"Job will be polled in %d seconds.\n",
                 job->next_poll_time-now);

    job->last_checkpoint_time = 0;

    job->history->stats[LAST_SUSPENSION_TIME] = now;
    job->history->stats[SUSPENSION_TIME]      = 0;
    job->history->stats[ACTIVE_TIME]          = 0;

    job->history->tries++;
        
    /* -------------------------------------------------------------------- */
    
    pthread_mutex_unlock(&(job->mutex));
    
    gw_em_mad_submit(job->history->em_mad, job_id, contact, rsl_filename);

    /* -------------------------------------------------------------------- */
    
    free(_job_id);
    
    free(rsl);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_em_cancel(void *_job_id)
{
    int           job_id;
    gw_job_t      *job;
    gw_em_mad_t   *mad;
    gw_em_state_t current_em_state;

    if ( _job_id != NULL )
    {
        job_id = *( (int *) _job_id );
        free(_job_id);

        job = gw_job_pool_get(job_id, GW_TRUE);

        if ( job == NULL )
        {
            gw_log_print("EM",'E',"Job %s no longer exists (CANCEL).\n", job_id);
            return;
        }
    }
    else
        return;

    if (job->history->tries >= job->template.number_of_retries)
    {
        gw_log_print("EM",'I',"Max number of cancel retries for job %i reached, considering it done\n", job_id);
        gw_job_print(job, "EM",'I',"Max number of cancel retries reached, considering it done\n");

        job->history->tries = 0;
        gw_am_trigger(&(gw_em.am),"GW_EM_STATE_DONE", _job_id);
    }

    /* -------------------------------------------------------------------- */
            
    current_em_state = job->em_state;
        
    if ( issubmitted(current_em_state) )
    {
        gw_log_print ("EM",'I',"Cancelling job %i.\n", job_id);
        gw_job_print (job,"EM",'I',"Cancelling job.\n");
        
        mad = job->history->em_mad;

        /* When in Migration Cancel, the previous MAD should be used */
        if (job->job_state == GW_JOB_STATE_MIGR_CANCEL)
        {
            if (job->history->next == NULL) 
            {
                gw_log_print("EM",'E',"Previous history record of job %i no longer exists\n", job_id);
                pthread_mutex_unlock(&(job->mutex));                        
                return;
            } 
            else
                mad = job->history->next->em_mad;
        }
        
        gw_em_mad_cancel(mad, job_id);
    }
    else
        gw_log_print ("EM",'W',"Ignoring cancel request for job %i, will retry.\n",
                job_id);    

    job->history->tries++;
    
    /* -------------------------------------------------------------------- */
            
    pthread_mutex_unlock(&(job->mutex));        
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


void gw_em_timer()
{
    int i;
    gw_job_t *job;
    time_t now;
    static int mark = 0;
    int *_job_id;
    gw_em_mad_t *mad;
    
    mark = mark + GW_EM_TIMER_PERIOD;
    if ( mark >= 300 )
    {
        gw_log_print("EM",'I',"-- MARK --\n");
        mark = 0;
    }    
    
    now = time(NULL);

    for (i= 0; i< gw_conf.number_of_jobs ; i++)
    {
        job = gw_job_pool_get(i, GW_TRUE);
        
        if ( job != NULL )
        {   
            if ( job->history == NULL )
            {
                pthread_mutex_unlock(&(job->mutex));
                continue;
            }
 
            if ( (job->job_state == GW_JOB_STATE_PRE_WRAPPER) 
                || (job->job_state == GW_JOB_STATE_WRAPPER)
                || (job->job_state == GW_JOB_STATE_MIGR_CANCEL)
                || (job->job_state == GW_JOB_STATE_STOP_CANCEL)
                || (job->job_state == GW_JOB_STATE_KILL_CANCEL))                                  
            {
                if (issubmitted(job->em_state))
                {
                    if ( now >= job->next_poll_time )
                    {
                        gw_log_print("EM",'I',"Checking execution state of job %i.\n", i);
                        gw_job_print(job,"EM",'I',"Checking execution state.\n");
                            
                        mad = job->history->em_mad;

                        /* Warning! When in Migration Cancel, the previous MAD should be used */
                        if (job->job_state == GW_JOB_STATE_MIGR_CANCEL)
                        {
                            if (job->history->next == NULL) 
                            {
                                gw_log_print("EM",'E',"Previous history record of job %i no longer exists\n", i);
                                pthread_mutex_unlock(&(job->mutex));                        
                                continue;
                            } 
                            else
                                mad = job->history->next->em_mad;
                        }                            
                                                     
                        gw_em_mad_poll(mad, i);

                        /* Wait for next poll (will be updated depending on success or failure) */
                        job->next_poll_time += gw_conf.poll_interval;
                    }
                }
                else if ((job->em_state == GW_EM_STATE_FAILED)
                        && (job->history->counter != -1))
                {
                    job->history->counter--;
 
                    if (job->history->counter == 0)
                    {
                        job->history->counter = -1;

                        _job_id    = (int *) malloc (sizeof(int));
                        *(_job_id) = i;

                        gw_am_trigger(&(gw_em.am),"GW_EM_SUBMIT", _job_id);
                    }
                }
            }

            pthread_mutex_unlock(&(job->mutex));            
        }
    }
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
