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
    int *       id;
    gw_job_t *  job;
    gw_host_t * host;
    int         rc;
    int         not_dispatch;
     
    job  = gw_job_pool_get(job_id, GW_TRUE);
                        
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
    
    /* ---- Check dispatch options ---- */
    
    pthread_mutex_lock(&(gw_dm.mutex));
    
    if ( gw_conf.jobs_per_sched <= 0 )
    	not_dispatch = 0;
    else
    	not_dispatch = gw_dm.dispatched_jobs == gw_conf.jobs_per_sched;

    pthread_mutex_unlock(&(gw_dm.mutex));
        	
    if ( host->running_jobs > 0 )
    	not_dispatch = not_dispatch || (host->running_jobs == gw_conf.jobs_per_host);
    	
    if ( gw_conf.jobs_per_user > 0 )
    	not_dispatch = not_dispatch || (gw_user_pool_max_running_jobs(job->user_id) == GW_TRUE);
    
    if ( not_dispatch )
    {
    	    pthread_mutex_unlock(&(host->mutex));
	        pthread_mutex_unlock(&(job->mutex));
    	    return 0;
    }
    
    /* -------------------------------- */
    
    if (job->job_state == GW_JOB_STATE_PENDING)
    {
        gw_log_print("DM",'I',"Dispatching job %i to %s (%s).\n",job->id, 
        	host->hostname, queue_name);
	    	
        rc = gw_job_history_add(&(job->history), 
                host,
                rank,
                queue_name,
                host->fork_name,
                host->lrms_name,
                job->owner,
                job->template.job_home,
                job->id,
                job->user_id,
                GW_FALSE);
                
		if ( rc == -1 )
		{
	        gw_log_print("DM",'E',"Could not add history record for job %i.\n",
	                job_id);
	                
            pthread_mutex_unlock(&(host->mutex));
	        pthread_mutex_unlock(&(job->mutex));
	        return -1;		
		}
		
    	if (job->reschedule == GW_TRUE)
    	{
	    	job->restarted++;
	    	job->reschedule = GW_FALSE;
    	}
    	
    	/* ----- Update Host, User & DM job counters ----- */
    	
        host->used_slots++;
		host->running_jobs++;
		
	    pthread_mutex_lock(&(gw_dm.mutex));
    	
    	gw_dm.dispatched_jobs++;
	    
	    pthread_mutex_unlock(&(gw_dm.mutex));
	    
	    gw_user_pool_inc_running_jobs(job->user_id, 1);
		
		/* ----------------------------------------------- */		
        
        id  = (int *) malloc(sizeof(int));
        *id =  job->id;

        gw_am_trigger(&(gw_dm.am), "GW_DM_STATE_PROLOG", (void *) id);

        pthread_mutex_unlock(&(host->mutex));        
        pthread_mutex_unlock(&(job->mutex));
    }
    else if (job->job_state == GW_JOB_STATE_WRAPPER)
    {    	
        gw_log_print("DM",'I',"Migrating job %i to %s (%s).\n",
                job->id, host->hostname, queue_name);

        rc = gw_job_history_add(&(job->history), 
                host,
                rank,
                queue_name,
                host->fork_name,
                host->lrms_name,
                job->owner,
                job->template.job_home,
                job->id,
                job->user_id,
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

    	/* ----- Update Host, User & DM job counters ----- */
    					
		host->used_slots++;
		host->running_jobs++;

	    pthread_mutex_lock(&(gw_dm.mutex));
    	
    	gw_dm.dispatched_jobs++;
	    
	    pthread_mutex_unlock(&(gw_dm.mutex));
		
		/* ----------------------------------------------- */

        id  = (int *) malloc(sizeof(int));
        *id =  job->id;

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

int gw_dm_dispatch_tasks (int    array_id,
                          int    ntasks, 
                          int    host_id,
                          char * queue_name, 
                          int    rank)
{
    int          number_of_tasks;
    int          job_id;
    int          task_id;
    int *        id;
    gw_job_t *   job;
	gw_array_t * array;
	gw_host_t *  host;
	int          rc;
    int          not_dispatch;
    
    array = gw_array_pool_get_array(array_id, GW_TRUE);

    if (array == NULL)
    {
        gw_log_print("DM",'E',"Array %i does not exist (DISPATCH).\n",array_id);
		return -1;
    }
	
    number_of_tasks   = array->number_of_tasks;


    for (task_id= 0;(task_id<number_of_tasks) && (ntasks>0); task_id++)
    {
        job_id = array->job_ids[task_id];
                       
        job = gw_job_pool_get(job_id, GW_TRUE);
        
        if (job == NULL)
            gw_log_print("DM",'E',"Can't dispatch tasks of array %i, job %i doesn't exist.\n",
                array->array_id, job_id);
        else
        {
			host = gw_host_pool_get_host(host_id, GW_TRUE);

		    if (host == NULL)
		    {
		        gw_log_print("DM",'E',"Can't Dispatch array %i, host %i not found.\n",
                     array_id, host_id);
                     
				pthread_mutex_unlock(&(job->mutex));                     
                pthread_mutex_unlock(&(array->mutex));
		        return -1;
			}        	
			
		    /* ---- Check dispatch options ---- */

		    pthread_mutex_lock(&(gw_dm.mutex));
	    
		    if ( gw_conf.jobs_per_sched <= 0 )
		    	not_dispatch = 0;
		    else
		    	not_dispatch = gw_dm.dispatched_jobs == gw_conf.jobs_per_sched;

		    pthread_mutex_unlock(&(gw_dm.mutex));
        	
		    if ( host->running_jobs > 0 )
		    	not_dispatch = not_dispatch || (host->running_jobs == gw_conf.jobs_per_host);
    	
		    if ( gw_conf.jobs_per_user > 0 )
		    	not_dispatch = not_dispatch || (gw_user_pool_max_running_jobs(job->user_id) == GW_TRUE);
    
		    if ( not_dispatch )
		    {
			    pthread_mutex_unlock(&(host->mutex));		    	
	            pthread_mutex_unlock(&(job->mutex));
			    pthread_mutex_unlock(&(array->mutex));
			    
    	    	return 0;
		    }

			/* -------------------------------- */ 
			  	
            if (  (job->job_state  == GW_JOB_STATE_PENDING)
               && (job->reschedule == GW_FALSE)
               && (job->history    == NULL))
            {
			    gw_log_print("DM",'I',"Dispatching task (%i) of array %i to %s (%s).\n",
				        task_id,array->array_id, host->hostname, queue_name);
				        
		        rc = gw_job_history_add(&(job->history), 
				        host,
					    rank,
					    queue_name,
					    host->fork_name,
					    host->lrms_name,
					    job->owner,
					    job->template.job_home,
					    job->id,
					    job->user_id,
                        GW_FALSE);
					                
				if ( rc == -1 )                
			    {
			    	gw_log_print("DM",'E',"Can't add history record for job %i.\n",
                            job_id);
			                     
				    pthread_mutex_unlock(&(host->mutex));		    	
		            pthread_mutex_unlock(&(job->mutex));
				    pthread_mutex_unlock(&(array->mutex));				    
				    
			        return -1;
				}
				
		    	/* ----- Update Host, User & DM job counters ----- */
    					
				host->used_slots++;
				host->running_jobs++;

	    		pthread_mutex_lock(&(gw_dm.mutex));
    	
    			gw_dm.dispatched_jobs++;
	    
	    		pthread_mutex_unlock(&(gw_dm.mutex));
		
			    gw_user_pool_inc_running_jobs(job->user_id, 1);
			    
				/* ----------------------------------------------- */
				
		        id  = (int *) malloc(sizeof(int));
		        *id =  job->id;

		        gw_am_trigger(&(gw_dm.am), "GW_DM_STATE_PROLOG", (void *) id);
        
	            ntasks--;   
            }
                
            pthread_mutex_unlock(&(host->mutex));	
            pthread_mutex_unlock(&(job->mutex));
        }
    }

    pthread_mutex_unlock(&(array->mutex));
        
    return 0;
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
