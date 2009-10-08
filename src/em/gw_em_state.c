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


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


void gw_em_pending(void *_job_id)
{
    int           job_id;
    gw_job_t      *job;
    gw_em_state_t current_em_state;
       
    if ( _job_id != NULL )
    {
    	job_id = *( (int *) _job_id );    	
    	job    = gw_job_pool_get(job_id, GW_TRUE);
    	
    	if ( job == NULL )
    		return;
    }
    else
    	return;

    /* ------------------------------------------------ */
        
    current_em_state = job->em_state;

    /* ----------------------------------------------- */
    /* Re-try the cancelation of current job           */
    /* ----------------------------------------------- */
    
    if (((job->job_state == GW_JOB_STATE_MIGR_CANCEL) ||
	     (job->job_state == GW_JOB_STATE_STOP_CANCEL) ||
	     (job->job_state == GW_JOB_STATE_KILL_CANCEL) ) &&
	     (!(issubmitted (current_em_state))))
	    gw_am_trigger(&(gw_em.am), "GW_EM_CANCEL", _job_id); 
    else
      	free(_job_id);
      	    
    if ( current_em_state == GW_EM_STATE_PENDING )
    {
    	gw_job_print (job,"EM",'I',"Execution state is PENDING.\n");
        pthread_mutex_unlock(&(job->mutex));
        return;
    }

    job->em_state = GW_EM_STATE_PENDING;

    /* ------------------------------------------------ */
    
    gw_job_print (job,"EM",'I',"New execution state is PENDING.\n");
          	
    pthread_mutex_unlock(&(job->mutex));    
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_em_active(void *_job_id)
{
    gw_job_t      *job;
    gw_em_state_t current_em_state;
    int           job_id;     

    if ( _job_id != NULL )
    {
    	job_id = *( (int *) _job_id );
    	job    = gw_job_pool_get(job_id, GW_TRUE);
    	
    	if ( job == NULL )
    		return;
    }
    else
    	return;

    /* ------------------------------------------------ */

    current_em_state = job->em_state;

    /* ----------------------------------------------- */
    /* Re-try the cancelation of current job           */
    /* ----------------------------------------------- */
    
    if (((job->job_state == GW_JOB_STATE_MIGR_CANCEL) ||
	     (job->job_state == GW_JOB_STATE_STOP_CANCEL) ||
	     (job->job_state == GW_JOB_STATE_KILL_CANCEL) ) &&
	     (!(issubmitted (current_em_state))))
	    gw_am_trigger(&(gw_em.am), "GW_EM_CANCEL", _job_id); 
    else
      	free(_job_id);
      	    
    if ( current_em_state == GW_EM_STATE_ACTIVE )
    {
    	gw_job_print (job,"EM",'I',"Execution state is ACTIVE.\n");
        pthread_mutex_unlock(&(job->mutex));      
        return;
    }        
    
    job->history->stats[SUSPENSION_TIME] += time(NULL) 
                                    - job->history->stats[LAST_SUSPENSION_TIME];                                        
    job->history->stats[LAST_ACTIVE_TIME] = time(NULL);

    job->em_state = GW_EM_STATE_ACTIVE;
    
    if (job->last_checkpoint_time == 0)
    	job->last_checkpoint_time = time(NULL);
    	
    /* ------------------------------------------------ */
        
    gw_job_print (job,"EM",'I',"New execution state is ACTIVE.\n");
        
    pthread_mutex_unlock(&(job->mutex));       
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_em_suspended(void *_job_id)
{
    gw_job_t      *job;
    gw_em_state_t current_em_state;
    int           job_id;

    if ( _job_id != NULL )
    {
    	job_id = *( (int *) _job_id );
    	job    = gw_job_pool_get(job_id, GW_TRUE);
    	
    	if ( job == NULL )
    		return;
    }
    else
    	return;
    
    /* ------------------------------------------------ */
    
    current_em_state = job->em_state;

    /* ----------------------------------------------- */
    /* Re-try the cancelation of current job           */
    /* ----------------------------------------------- */
    
    if (((job->job_state == GW_JOB_STATE_MIGR_CANCEL) ||
	     (job->job_state == GW_JOB_STATE_STOP_CANCEL) ||
	     (job->job_state == GW_JOB_STATE_KILL_CANCEL) ) &&
	     (!(issubmitted (current_em_state))))
	    gw_am_trigger(&(gw_em.am), "GW_EM_CANCEL", _job_id); 
    else
      	free(_job_id);
      	    
    if ( current_em_state == GW_EM_STATE_SUSPENDED )
    {
        gw_job_print (job,"EM",'I',"Execution state is SUSPENDED.\n");
        pthread_mutex_unlock(&(job->mutex));
        return;
    }    
    
    if ( current_em_state == GW_EM_STATE_ACTIVE )
        job->history->stats[ACTIVE_TIME] += time(NULL) 
                - job->history->stats[LAST_ACTIVE_TIME];    

    if ( current_em_state != GW_EM_STATE_PENDING )        
        job->history->stats[LAST_SUSPENSION_TIME] = time(NULL);

    job->em_state = GW_EM_STATE_SUSPENDED;
    
    /* ------------------------------------------------ */
            
    gw_job_print (job,"EM",'I',"New execution state is SUSPENDED.\n");
    
    pthread_mutex_unlock(&(job->mutex));
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_em_done(void *_job_id)
{
    gw_job_t      *job;
    int           job_id;
    gw_em_state_t  current_em_state;
    gw_job_state_t current_state;

    if ( _job_id != NULL )
    {
    	job_id = *( (int *) _job_id );
    	    	
    	job    = gw_job_pool_get(job_id, GW_TRUE);
    	
    	if ( job == NULL )
    		return;
    }
    else
    	return;
   
    /* ------------------------------------------------ */
        
    current_em_state = job->em_state;
    
    if ( current_em_state == GW_EM_STATE_DONE )
    {
        pthread_mutex_unlock(&(job->mutex));
        return;    
    }
            
    current_state = job->job_state;
    
    if ( current_state == GW_JOB_STATE_MIGR_CANCEL )
    {
        if ( current_em_state == GW_EM_STATE_ACTIVE )
            job->history->next->stats[ACTIVE_TIME] += time(NULL) 
                                  - job->history->next->stats[LAST_ACTIVE_TIME];
        else     
            job->history->next->stats[SUSPENSION_TIME] += time(NULL) 
                              - job->history->next->stats[LAST_SUSPENSION_TIME];
    }
    else
    {
        if ( current_em_state == GW_EM_STATE_ACTIVE )
            job->history->stats[ACTIVE_TIME]+= time(NULL) 
                                        - job->history->stats[LAST_ACTIVE_TIME];
        else
            job->history->stats[SUSPENSION_TIME] += time(NULL) -
                                      job->history->stats[LAST_SUSPENSION_TIME];
    }

    job->em_state = GW_EM_STATE_DONE;    
    
    /* -------------------------------------------------------------------- */
            
    gw_job_print (job,"EM",'I',"New execution state is DONE.\n");
                 
    /* -------------------------------------------------------------------- */

    pthread_mutex_unlock(&(job->mutex));
    
    gw_am_trigger(gw_em.dm_am, "GW_DM_WRAPPER_DONE", _job_id );
    

}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_em_failed(void *_job_id)
{
    gw_job_t      *job;
    gw_em_state_t current_em_state;
    
    int    tries, num_retries, job_id;


    if ( _job_id != NULL )
    {
    	job_id = *( (int *) _job_id );
    	    	
    	job    = gw_job_pool_get(job_id, GW_TRUE);
    	
    	if ( job == NULL )
    		return;
    }
    else
    	return;
    
        
    current_em_state = job->em_state;

    /* -------------------------------------------------------------------- */
    
    if (  (current_em_state == GW_EM_STATE_SUSPENDED)
        ||(current_em_state == GW_EM_STATE_PENDING) )
        job->history->stats[SUSPENSION_TIME] += time(NULL) 
                                    - job->history->stats[LAST_SUSPENSION_TIME];
        
    if ( current_em_state == GW_EM_STATE_ACTIVE )
        job->history->stats[ACTIVE_TIME] += time(NULL)
                                        - job->history->stats[LAST_ACTIVE_TIME];

    job->em_state = GW_EM_STATE_FAILED;
    
    tries       = job->history->tries;
    num_retries = job->template.number_of_retries;

    /* -------------------------------------------------------------------- */
    
    gw_job_print (job,"EM",'I',"New execution state is FAILED.\n");
        
    /* -------------------------------------------------------------------- */        
    
    if ( tries >= num_retries )
    {   
        job->history->counter = -1;
            	             
        gw_job_print(job,"EM",'E',"Job failed, no retries left.\n");
        gw_log_print("EM",'E',"Job %i failed, no retries left.\n", job_id);
                
 		gw_am_trigger(gw_em.dm_am, "GW_DM_WRAPPER_FAILED", _job_id);
    }
    else
    {       
    	job->history->counter = tries;
    					    	
        gw_job_print (job,"EM",'W',"Retrying execution in ~%i seconds, (%i retries left).\n", 
            tries * GW_EM_TIMER_PERIOD,
            num_retries - tries);
                      
        gw_log_print ("EM",'W',"Job %i failed, retrying execution in ~%i seconds, (%i retries left).\n", 
            job_id,
            tries * GW_EM_TIMER_PERIOD,
            num_retries - tries);
            
        free(_job_id);
    }
    
    pthread_mutex_unlock(&(job->mutex));
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
