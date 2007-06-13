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

static void gw_dm_prolog_set_files(gw_job_t * job);

void gw_dm_prolog ( void *_job_id )
{
    gw_job_t * job;
    int        job_id;
	int        rc;
	
    /* ----------------------------------------------------------- */  
    /* 0.- Get job pointer                                         */
    /* ----------------------------------------------------------- */  
    
    if ( _job_id != NULL )
    {
        job_id = *( (int *) _job_id );

        job = gw_job_pool_get(job_id, GW_TRUE);

        if ( job == NULL )
        {
			gw_log_print("DM",'E',"Job %i does not exist (JOB_STATE_PROLOG).\n",job_id);

            free(_job_id);
            return;
        }
    }
    else
        return;
        
    /* ----------------------------------------------------------- */  
    /* 1.- Set state and times                                     */
    /* ----------------------------------------------------------- */  
    
    gw_job_set_state(job, GW_JOB_STATE_PROLOG, GW_FALSE);
    
    job->history->stats[START_TIME]        = time(NULL);
    job->history->stats[PROLOG_START_TIME] = time(NULL);
    
    /* ----------------------------------------------------------- */  
    /* 2.- Signal the Transfer Manager & set files                 */
    /* ----------------------------------------------------------- */

	rc = gw_job_environment(job);
	
	if ( rc == 0 )
	{
		gw_dm_prolog_set_files(job);
	
    	gw_am_trigger(gw_dm.tm_am, "GW_TM_PROLOG", _job_id);
	}
	else
    	gw_am_trigger(&(gw_dm.am), "GW_DM_PROLOG_FAILED", _job_id);
    
    pthread_mutex_unlock(&(job->mutex));
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_dm_migr_prolog ( void *_job_id )
{
    gw_job_t * job;
    int        job_id;
    int        rc;
    
    /* ----------------------------------------------------------- */  
    /* 0.- Get job pointer                                         */
    /* ----------------------------------------------------------- */  
    
    if ( _job_id != NULL )
    {
        job_id = *( (int *) _job_id );

        job = gw_job_pool_get(job_id, GW_TRUE);

        if ( job == NULL )
        {
			gw_log_print("DM",'E',"Job %i does not exist (JOB_STATE_MGIR_PROLOG).\n",job_id);

            free(_job_id);
            return;
        }
    }
    else
        return;
        
    /* ----------------------------------------------------------- */  
    /* 1.- Set state and times                                     */
    /* ----------------------------------------------------------- */  
    
    gw_job_set_state(job, GW_JOB_STATE_MIGR_PROLOG, GW_FALSE);
    
    job->history->stats[START_TIME]        = time(NULL);
    job->history->stats[PROLOG_START_TIME] = time(NULL);
    
    /* ----------------------------------------------------------- */  
    /* 2.- Signal the Transfer Manager                             */
    /* ----------------------------------------------------------- */

	rc = gw_job_environment(job);
	
	if ( rc == 0 )
	{
		gw_dm_prolog_set_files(job);
	
    	gw_am_trigger(gw_dm.tm_am, "GW_TM_PROLOG", _job_id);
	}
	else
    	gw_am_trigger(&(gw_dm.am), "GW_DM_PROLOG_FAILED", _job_id);
        
    pthread_mutex_unlock(&(job->mutex));
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_dm_prolog_done_cb ( void *_job_id )
{
    gw_job_t * job;
    int        job_id;
    time_t     total;
    
    /* ----------------------------------------------------------- */  
    /* 0.- Get job pointer                                         */
    /* ----------------------------------------------------------- */  
    
    if ( _job_id != NULL )
    {
        job_id = *( (int *) _job_id );

        job = gw_job_pool_get(job_id, GW_TRUE);

        if ( job == NULL )
        {
			gw_log_print("DM",'E',"Job %i does not exist (JOB_STATE_PROLOG_DONE_CB).\n",job_id);
        	
            free(_job_id);
            return;
        }
    }
    else
        return;

    /* ----------------------------------------------------------- */  
    /* 1.- Set PROLOG time                                         */
    /* ----------------------------------------------------------- */  
    
    job->history->stats[PROLOG_EXIT_TIME] = time(NULL);
    total = gw_job_history_get_prolog_time(job->history);
    
    gw_job_print(job,"DM",'I',"Prolog done:\n");
    gw_job_print(job,"DM",'I',"\tTotal time      : %i\n", total);

    /* ----------------------------------------------------------- */  
    /* 2.- State transtition                                       */
    /* ----------------------------------------------------------- */  

    switch (job->job_state)
    {
        case GW_JOB_STATE_PROLOG:
            if ( job->template.pre_wrapper != NULL )
                gw_am_trigger(&(gw_dm.am), "GW_DM_STATE_PRE_WRAPPER", _job_id);
            else
                gw_am_trigger(&(gw_dm.am), "GW_DM_STATE_WRAPPER", _job_id);
            break;
            
        case GW_JOB_STATE_MIGR_PROLOG:
                gw_am_trigger(&(gw_dm.am), "GW_DM_STATE_MIGR_EPILOG", _job_id);        
            break;
            
        default:        
			gw_log_print("DM",'E',"Prolog done callback in wrong job (%i) state.\n", job_id);        

            free(_job_id);        
            break;
    }
            
    pthread_mutex_unlock(&(job->mutex));
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_dm_prolog_failed_cb ( void *_job_id )
{
    gw_job_t * job;
    int        job_id;
    time_t     total;
    
    /* ----------------------------------------------------------- */  
    /* 0.- Get job pointer                                         */
    /* ----------------------------------------------------------- */  
    
    if ( _job_id != NULL )
    {
        job_id = *( (int *) _job_id );

        job = gw_job_pool_get(job_id, GW_TRUE);

        if ( job == NULL )
        {
			gw_log_print("DM",'E',"Job %i does not exist (JOB_STATE_PROLOG_FAILED_CB).\n",job_id);

            free(_job_id);
            return;
        }
    }
    else
        return;

    /* ----------------------------------------------------------- */  
    /* 1.- Set PROLOG time                                         */
    /* ----------------------------------------------------------- */  
    
    job->history->stats[PROLOG_EXIT_TIME] = time(NULL);
    job->history->stats[EXIT_TIME]        = time(NULL);

    total = gw_job_history_get_prolog_time(job->history);
    
    gw_job_print(job,"DM",'I',"Prolog failed:\n");
    gw_job_print(job,"DM",'I',"\tTotal time      : %i\n", total);
    
    if (job->job_state == GW_JOB_STATE_MIGR_PROLOG)
   	{
   		job->history->stats[MIGRATION_EXIT_TIME] = time(NULL);
   		
		if (job->history->next != NULL)
		{
	   		job->history->next->stats[EXIT_TIME] = time(NULL);

            gw_host_dec_rjobs(job->history->next->host);
		}   		
   	}
   	
    /* ----------------------------------------------------------- */  
    /* 2.- State transtition                                       */
    /* ----------------------------------------------------------- */  
	
	if (job->history != NULL)
	{
		job->history->reason = GW_REASON_EXECUTION_ERROR;
        gw_host_dec_uslots(job->history->host, job->template.np);
	}
	
	if (job->template.reschedule_on_failure == GW_TRUE)
		gw_am_trigger(&(gw_dm.am), "GW_DM_STATE_PENDING", _job_id);
    else
		gw_am_trigger(&(gw_dm.am), "GW_DM_STATE_FAILED", _job_id);
			
    pthread_mutex_unlock(&(job->mutex));
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_dm_prolog_set_files(gw_job_t * job)
{
    int  num_xfrs;
    int  i, j;
    char url[512], alt_url[512];
    int  wbe; /* Wrapper-based execution */

    wbe = gw_job_is_wrapper_based(job);

    /* ----------------------------------------------------------- */  
    /* 1.- Set xfr array                                           */
    /* ----------------------------------------------------------- */

    if ( wbe )
        num_xfrs = job->template.num_input_files + 1 /* job.env */
                 + ( job->template.executable != NULL )
                 + ( job->template.stdin_file != NULL )
                 + ( job->template.pre_wrapper != NULL )
                 + ( job->template.wrapper != NULL )
                 + ( job->template.monitor != NULL );
    else
        num_xfrs = job->template.num_input_files
		         + ( job->template.executable != NULL )
		         + ( job->template.stdin_file != NULL )
		         + ( job->template.pre_wrapper != NULL );	
	
	if ( ( job->restarted != 0 ) && 
	     ( job->template.num_restart_files > 0 ) )
	     num_xfrs += job->template.num_restart_files;

	gw_xfr_destroy (&(job->xfrs));
	gw_xfr_init    (&(job->xfrs), num_xfrs, job->template.number_of_retries);
	
	/* ----------------------------------------------------------- */  
    /* 2.- Set input files                                         */
    /* ----------------------------------------------------------- */  

	for (i = 0; i< job->template.num_input_files ; i++)
	{
		job->xfrs.xfrs[i].src_url = 
			strdup(job->template.input_files[i][GW_LOCAL_FILE]);
			
		if (job->template.input_files[i][GW_REMOTE_FILE] != NULL )
			job->xfrs.xfrs[i].dst_url = 
					strdup(job->template.input_files[i][GW_REMOTE_FILE]);
		else
			job->xfrs.xfrs[i].dst_url = NULL;
			
		job->xfrs.xfrs[i].alt_src_url = NULL;
		
		job->xfrs.xfrs[i].mode        = '-';
	}
	
    /* ----------------------------------------------------------- */  
    /* 3.- Environment                                             */
    /* ----------------------------------------------------------- */  		
	
	if ( wbe )
	{
		snprintf(url,sizeof(char)*512,"file://%s/job.env",job->directory);
	
		job->xfrs.xfrs[i].src_url = strdup(url);
		job->xfrs.xfrs[i].dst_url = NULL;
		job->xfrs.xfrs[i].mode    = '-';
		
		job->xfrs.xfrs[i++].alt_src_url = NULL;
	}
	
    /* ----------------------------------------------------------- */  
    /* 3.- Executable file                                         */
    /* ----------------------------------------------------------- */  		
	
	if ( job->template.executable != NULL )
	{
		job->xfrs.xfrs[i].src_url = strdup(job->template.executable);
		job->xfrs.xfrs[i].dst_url = NULL;
		job->xfrs.xfrs[i].mode    = 'X';
		
		job->xfrs.xfrs[i++].alt_src_url = NULL;
	}
	
	/* ----------------------------------------------------------- */  
    /* 4.- Standard input file                                     */
    /* ----------------------------------------------------------- */  		
	
	if ( job->template.stdin_file != NULL )
	{
		job->xfrs.xfrs[i].src_url = strdup(job->template.stdin_file);
		job->xfrs.xfrs[i].dst_url = strdup("stdin.execution");
		job->xfrs.xfrs[i].mode    = '-';
		
		job->xfrs.xfrs[i++].alt_src_url = NULL;
	}

	/* ----------------------------------------------------------- */  
    /* 5.- Pre-wrapper executable                                  */
    /* ----------------------------------------------------------- */  		
	
	if ( job->template.pre_wrapper != NULL )
	{
		job->xfrs.xfrs[i].src_url = strdup(job->template.pre_wrapper);
		job->xfrs.xfrs[i].dst_url = NULL;
		job->xfrs.xfrs[i].mode    = 'X';
		
		job->xfrs.xfrs[i++].alt_src_url = NULL;
	}

	/* ----------------------------------------------------------- */  
    /* 6.- Wrapper executable                                      */
    /* ----------------------------------------------------------- */  		
	
	if ( wbe )
	{
	    snprintf(url,sizeof(char)*512,"file://%s",job->template.wrapper);
	    
		job->xfrs.xfrs[i].src_url = strdup(url);
		job->xfrs.xfrs[i].dst_url = strdup(".wrapper");		
		job->xfrs.xfrs[i].mode    = 'X';
		
		job->xfrs.xfrs[i++].alt_src_url = NULL;	    
	}

	/* ----------------------------------------------------------- */  
    /* 7.- Monitor file                                            */
    /* ----------------------------------------------------------- */  		
	
	if ( wbe && job->template.monitor != NULL )
	{
	    snprintf(url,sizeof(char)*512,"file://%s", job->template.monitor);
	    
		job->xfrs.xfrs[i].src_url = strdup(url);
		job->xfrs.xfrs[i].dst_url = strdup(".monitor");
		job->xfrs.xfrs[i].mode    = 'X';
		
		job->xfrs.xfrs[i++].alt_src_url = NULL;	    
	}
    
	/* ----------------------------------------------------------- */  
    /* 8.-Re-start files                                           */
    /*    From last execution host (default)                       */
    /*    From checkpointing server                                */
    /* ----------------------------------------------------------- */  		
	
    job->xfrs.failure_limit = i;
    
	if ( ( job->restarted != 0 ) && ( job->template.num_restart_files > 0 ) )
	{
		for (j = 0; j< job->template.num_restart_files ; j++)
		{
			snprintf(url, sizeof(char)*512, "%s%s",
							job->history->next->rdir,
							job->template.restart_files[j]);

			snprintf(alt_url,sizeof(char)*512,"%s%s",
							job->template.checkpoint_url,
							job->template.restart_files[j]);

			if ( job->job_state == GW_JOB_STATE_MIGR_PROLOG )
			{
				job->xfrs.xfrs[i+j].src_url     = strdup(url);
				job->xfrs.xfrs[i+j].alt_src_url = strdup(alt_url);				
			}
			else
			{
				job->xfrs.xfrs[i+j].src_url     = strdup(alt_url);
				job->xfrs.xfrs[i+j].alt_src_url = NULL;								
			}
			
				job->xfrs.xfrs[i+j].dst_url = NULL;

			job->xfrs.xfrs[i+j].mode    = '-';
		}
	}
}
