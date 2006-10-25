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


#include "gw_log.h"
#include "gw_conf.h"
#include "gw_job_pool.h"
#include "gw_tm_mad.h"
#include "gw_tm.h"

/* --------------------------------------------------------------- */
/* --------------------------------------------------------------- */
/* --------------------------------------------------------------- */

void gw_tm_prolog(void *_job_id)
{
    int        job_id;
    gw_job_t * job;
    
	if ( _job_id != NULL )
	{
		job_id = *( (int *) _job_id );
		free(_job_id);
		
		job = gw_job_pool_get(job_id, GW_TRUE);
	
		if ( job == NULL )
		{
			gw_log_print("TM",'E',"Job %i no longer exists (TM_PROLOG).\n", job_id);
			return;
		}
	}
	else
		return;
		
	if (job->history == NULL) 
    {
		gw_log_print("TM",'E',"History of job %i doesn't exist (TM_PROLOG).\n", job_id);
        pthread_mutex_unlock(&(job->mutex));				
		return;
    }		

	if ( job->tm_state == GW_TM_STATE_INIT )
	{
  		job->tm_state = GW_TM_STATE_PROLOG;
		gw_tm_mad_start(job->history->tm_mad, job->id);
	}
	else
	{
  		job->tm_state = GW_TM_STATE_CHECKPOINT_CANCEL;
	  	gw_tm_mad_end(job->history->tm_mad, job->id);  		
	}


    pthread_mutex_unlock(&(job->mutex));          
}

/* --------------------------------------------------------------- */
/* --------------------------------------------------------------- */
/* --------------------------------------------------------------- */

void gw_tm_epilog(void *_job_id)
{
    int        job_id;
    gw_job_t * job;
  
	if ( _job_id != NULL )
	{
		job_id = *( (int *) _job_id );
		
		job = gw_job_pool_get(job_id, GW_TRUE);
		
		free(_job_id);
		if ( job == NULL )
		{
			gw_log_print("TM",'E',"Job %i no longer exists (TM_EPILOG).\n", job_id);
			return;
		}		
	}
	else
		return;			

	if (job->history == NULL) 
    {
		gw_log_print("TM",'E',"History of job %i doesn't exists (TM_EPILOG).\n",
				     job_id);
        pthread_mutex_unlock(&(job->mutex));				
		return;
    }		
				
	switch (job->job_state)
	{		
		case GW_JOB_STATE_KILL_EPILOG:
		case GW_JOB_STATE_MIGR_EPILOG:
		case GW_JOB_STATE_EPILOG_FAIL:
		case GW_JOB_STATE_EPILOG:
		case GW_JOB_STATE_EPILOG_STD:
	    	if ( job->tm_state == GW_TM_STATE_INIT )
	    	{
			  	gw_tm_mad_start(job->history->tm_mad, job->id);
  				job->tm_state = GW_TM_STATE_EPILOG;
	    	}
			else
			{
				gw_job_print(job,"TM",'W',"Cancelling copy of checkpoint files.\n");
			  	gw_tm_mad_end(job->history->tm_mad, job->id);			  				
  				job->tm_state = GW_TM_STATE_CHECKPOINT_CANCEL;
			}
			break;
			
		case GW_JOB_STATE_STOP_EPILOG:
		case GW_JOB_STATE_EPILOG_RESTART:		
			if ( job->tm_state == GW_TM_STATE_INIT )
			{
				job->tm_state = GW_TM_STATE_EPILOG;	
			  	gw_tm_mad_start(job->history->tm_mad, job->id);
			}
			else
				gw_job_print(job,"TM",'I',"Copy of checkpoint files in progress. Starting epilog once it finish.\n");
			break;
		
		default:
			gw_log_print("TM",'E',"Epilog signal for job %i in wrong state.\n", job_id);
			break;
	}		
	
    pthread_mutex_unlock(&(job->mutex));
}

/* --------------------------------------------------------------- */
/* --------------------------------------------------------------- */
/* --------------------------------------------------------------- */

void gw_tm_timer(void *_null)
{
    int           i,j;
    gw_job_t      *job;
    time_t        poll;
    time_t        poll_timeout;    
    static int    mark = 0;
	char *        src_url;
	char *        dst_url;
	char          mode;
	gw_xfrs_t *   xfrs;
	
	int           (*build_urls) (gw_job_t *   job, 
                                 char *       src, 
                                 char *       dst, 
                                 char **      src_url,
                                 char **      dst_url);
	
    mark = mark + GW_TM_TIMER_PERIOD;
    if ( mark >= 300 )
    {
        gw_log_print ("TM",'I',"-- MARK --\n");
        mark = 0;
    }

    for (i=0; i< gw_conf.number_of_jobs ; i++)
    {
        job = gw_job_pool_get(i, GW_TRUE);
        
        if ( job == NULL )
            continue;
                
        poll_timeout = job->template.checkpoint_interval;
        
        if ( poll_timeout > 0
                && job->job_state == GW_JOB_STATE_WRAPPER
                && job->tm_state  == GW_TM_STATE_INIT
                && job->template.num_restart_files != 0 )
        {
            poll = time(NULL) - job->last_checkpoint_time;
            
            if ( poll >= poll_timeout )
            {
                gw_log_print ("TM",'I',"Checkpoint timeout of job %i expired. Transfering restart files.\n", i);

                if (job->history == NULL) 
                {
                    gw_log_print("TM",'E',"History of job %i doesn't exist.\n",i);
                    pthread_mutex_unlock(&(job->mutex));                
                    continue;
                }        
                                                     
                gw_tm_mad_start(job->history->tm_mad, job->id);
                    
                job->last_checkpoint_time = time(NULL);
                job->tm_state = GW_TM_STATE_CHECKPOINT;
            }
        }
        else if (job->tm_state  == GW_TM_STATE_PROLOG
                   || job->tm_state == GW_TM_STATE_EPILOG 
                   || job->tm_state == GW_TM_STATE_CHECKPOINT )
        {
        	
        	switch (job->tm_state)
        	{
        		case GW_TM_STATE_PROLOG:
        		    xfrs       = &(job->xfrs);
        		    build_urls = &gw_tm_prolog_build_urls;
        		    break;
        		    
        		case GW_TM_STATE_EPILOG:        		    
        		    xfrs       = &(job->xfrs);
        		    build_urls = &gw_tm_epilog_build_urls;
        		    break;
        		    
        		case GW_TM_STATE_CHECKPOINT:        		    
        		    xfrs       = &(job->chk_xfrs);
        		    build_urls = &gw_tm_epilog_build_urls;
        		    break;
        		    
        		 default:
        		    break;
        	}
        		
       	    for (j = 0; j< xfrs->number_of_xfrs ; j++)
       	    {
       	        if ( xfrs->xfrs[j].counter != -1 )
       	        {
                    xfrs->xfrs[j].counter--;
                    
                    if ( xfrs->xfrs[j].counter == 0 )
                    {
                        xfrs->xfrs[j].counter = -1;
                        
                        if (job->tm_state == GW_TM_STATE_PROLOG)
                        {
                        	mode = xfrs->xfrs[j].mode;
                        }
                        else
                        {
                        	mode = '-';
                        }                        	
                        	
                        build_urls(job,
                                xfrs->xfrs[j].src_url, 
		                        xfrs->xfrs[j].dst_url, 
		                        &src_url,
		                        &dst_url);                        	
                   
			            gw_tm_mad_cp(job->history->tm_mad, 
			                job->id, 
			                j, 
			                mode,
			                src_url,
			                dst_url);

      			        free(src_url);
			            free(dst_url);                        
                    }
       	        }
       	    }
        }
        
        pthread_mutex_unlock(&(job->mutex));
    }
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
