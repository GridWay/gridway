/* -------------------------------------------------------------------------- */
/* Copyright 2002-2010, GridWay Project Leads (GridWay.org)                   */
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


#include "gw_scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

/* ------------------------------------------------------------------------- */
/* FUNCTION DEFINICION                                                       */
/* ------------------------------------------------------------------------- */

static void gw_sched (gw_scheduler_t * sched,
                      void *           user_arg);

static void gw_sched_dispatch (gw_scheduler_t * sched, 
                               gw_boolean_t     reschedule, 
                               int *            dispatched);
                             
int main(int argc, char **argv)
{
     gw_scheduler_loop(gw_sched, (void *) NULL);
     
     return 0;
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

static void gw_sched (gw_scheduler_t * sched,
                         void *           user_arg)
{
    int dispatched = 0;
    
    gw_sched_dispatch (sched, GW_FALSE, &dispatched);
    
    gw_sched_dispatch (sched, GW_TRUE, &dispatched);
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

static void gw_sched_dispatch (gw_scheduler_t * sched, 
                               gw_boolean_t     reschedule, 
                               int *            dispatched)
{
    int j,i;
    
    int max_dsp;
    int max_user;
    int max_host;
    int free_slots;
    
    int running_user;
    int running_host;
    
    gw_sch_job_t  * job;
    gw_sch_user_t * user;
    gw_sch_host_t * host;
    
    max_dsp  = sched->sch_conf.max_dispatch;
    if (max_dsp == 0)
        max_dsp = sched->num_jobs;
        
    max_user = sched->sch_conf.max_user;
    max_host = sched->sch_conf.max_resource;
    
    for (i=0; (i<sched->num_jobs) && (*dispatched < max_dsp); i++)
    {
        user = &(sched->users[sched->jobs[i].ua_id]);
        job  = &(sched->jobs[i]);
                
        running_user = user->running_jobs + user->dispatched;
        
        if ( max_user && running_user >= max_user )
        {
#ifdef GWSCHEDDEBUG
            gw_scheduler_print('D',"Max. number of running jobs for user %s reached\n",
                user->name);
#endif
            continue;   
        }

        if (((reschedule == GW_FALSE) && (job->reason != GW_REASON_NONE)) ||
            ((reschedule == GW_TRUE ) && (job->reason == GW_REASON_NONE)))
            continue;

        for ( j=0 ; j < job->num_mhosts ; j++ )
        {
            host = &(sched->hosts[job->mhosts[j].ha_id]);
                        
            running_host = host->used_slots + host->dispatched;
            
            if (max_host && running_host >= max_host)   
            {
#ifdef GWSCHEDDEBUG
                gw_scheduler_print('D',"Max. number of running jobs for host %s reached\n",
                    host->name);
#endif
                continue;   
            }
            
            free_slots = job->mhosts[j].slots - host->dispatched;
            
            if ( free_slots >= job->np )
            {
                *dispatched = *dispatched + 1;
                
                host->dispatched++;
                user->dispatched++;  
                                            
                printf("SCHEDULE_JOB %i SUCCESS %i:%s:%i\n",
                            job->jid,
                            host->hid,
                            job->mhosts[j].qname,
                            job->mhosts[j].rank);

                gw_scheduler_print('I',"Job %-4i scheduled to host %s (queue: %s)\n",
                            job->jid,
                            host->name,
                            job->mhosts[j].qname);
                                                                
                gw_scheduler_job_del(sched,job->jid,1);
                        
                i = i - 1; /* Next job will be i, not i+1 */
                                                                                    
                break;   
            }  
        }         
    }
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
