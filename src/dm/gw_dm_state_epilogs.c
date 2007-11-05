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
#include "gw_file_parser.h"

static void gw_dm_epilog_std_wrapper_files(gw_job_t * job, int * index);
static void gw_dm_epilog_std_files(gw_job_t * job, int * index);
static void gw_dm_epilog_output_files(gw_job_t * job, int * index);
static void gw_dm_epilog_restart_files(gw_job_t * job, int * index);

static int gw_dm_epilog_parse_wrapper_std(gw_job_t *job);

void gw_dm_epilog_std ( void *_job_id )
{
    gw_job_t * job;
    int        job_id;
    int        index;
	int        num_xfrs;
	
	/* ----------------------------------------------------------- */  
    /* 0.- Get job pointer                                         */
    /* ----------------------------------------------------------- */  
    
	if ( _job_id != NULL )
	{
		job_id = *( (int *) _job_id );

		job = gw_job_pool_get(job_id, GW_TRUE);

		if ( job == NULL )
		{
			gw_log_print("DM",'E',"Job %i does not exist (JOB_STATE_EPILOG_STD).\n",job_id);
			free(_job_id);
			return;
		}
	}
	else
		return;
	    
    /* ----------------------------------------------------------- */  
    /* 1.- Set state and times                                     */
    /* ----------------------------------------------------------- */  
    
	if (job->job_state != GW_JOB_STATE_EPILOG_STD)
	{
	    gw_job_set_state(job, GW_JOB_STATE_EPILOG_STD, GW_FALSE);
	    job->history->stats[EPILOG_START_TIME] = time(NULL);
    }
    
    /* ----------------------------------------------------------- */  
    /* 2.- Signal the Transfer Manager                             */
    /* ----------------------------------------------------------- */

	gw_xfr_destroy (&(job->xfrs));
	
	index    = 0;
	num_xfrs = 2;
	
	gw_xfr_init(&(job->xfrs), num_xfrs, job->template.number_of_retries);
	
	job->xfrs.failure_limit = -1;
	
	gw_dm_epilog_std_wrapper_files(job, &index);

	gw_am_trigger(gw_dm.tm_am, "GW_TM_EPILOG", _job_id);
	       
    pthread_mutex_unlock(&(job->mutex));
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_dm_migr_epilog ( void *_job_id )
{
    gw_job_t * job;
    int        job_id;
    int        index;
	int        num_xfrs;
	
	    
	/* ----------------------------------------------------------- */  
    /* 0.- Get job pointer                                         */
    /* ----------------------------------------------------------- */  
    
	if ( _job_id != NULL )
	{
		job_id = *( (int *) _job_id );

		job = gw_job_pool_get(job_id, GW_TRUE);

		if ( job == NULL )
		{
			gw_log_print("DM",'E',"Job %i does not exist (JOB_STATE_MIGR_EPILOG).\n",job_id);			

			free(_job_id);
			return;
		}
	}
	else
		return;
	    
    /* ----------------------------------------------------------- */  
    /* 1.- Set state and times                                     */
    /* ----------------------------------------------------------- */  
    
	if (job->job_state != GW_JOB_STATE_MIGR_EPILOG)
	{
	    gw_job_set_state(job, GW_JOB_STATE_MIGR_EPILOG, GW_FALSE);
        job->history->next->stats[EPILOG_START_TIME] = time(NULL);
    }

    /* ----------------------------------------------------------- */  
    /* 2.- Signal the Transfer Manager                             */
    /* ----------------------------------------------------------- */

	gw_xfr_destroy (&(job->xfrs));
	
	index    = 0;
	num_xfrs = 0
            + (job->template.stdout_file != NULL)
            + (job->template.stderr_file != NULL)
            + 2*(job->template.pre_wrapper != NULL)
            + 2*(job->template.monitor != NULL);
		
	gw_xfr_init(&(job->xfrs), num_xfrs, job->template.number_of_retries);

	job->xfrs.failure_limit = -1;
			
	gw_dm_epilog_std_files(job, &index);	
        
    gw_am_trigger(gw_dm.tm_am, "GW_TM_EPILOG", _job_id);
    
    pthread_mutex_unlock(&(job->mutex));
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_dm_stop_epilog ( void *_job_id )
{
    gw_job_t * job;
    int        job_id;
    int        index;
    int        num_xfrs;
	    
    /* ----------------------------------------------------------- */  
    /* 0.- Get job pointer                                         */
    /* ----------------------------------------------------------- */  
    
	if ( _job_id != NULL )
	{
		job_id = *( (int *) _job_id );

		job = gw_job_pool_get(job_id, GW_TRUE);

		if ( job == NULL )
		{
			gw_log_print("DM",'E',"Job %i does not exist (JOB_STATE_STOP_EPILOG).\n",job_id);			

			free(_job_id);
			return;
		}
	}
	else
		return;
	    
    /* ----------------------------------------------------------- */  
    /* 1.- Set state and times                                     */
    /* ----------------------------------------------------------- */  

	if (job->job_state != GW_JOB_STATE_STOP_EPILOG)
	{   
	    gw_job_set_state(job, GW_JOB_STATE_STOP_EPILOG, GW_FALSE);
	    job->history->stats[EPILOG_START_TIME] = time(NULL);
    }

    /* ----------------------------------------------------------- */  
    /* 2.- Signal the Transfer Manager                             */
    /* ----------------------------------------------------------- */

	gw_xfr_destroy (&(job->xfrs));
	
	index    = 0;
	num_xfrs = 0
            + (job->template.stdout_file != NULL)
            + (job->template.stderr_file != NULL)
            + 2*(job->template.pre_wrapper != NULL)
            + 2*(job->template.monitor != NULL);
	
	if (job->tm_state != GW_TM_STATE_CHECKPOINT)
		num_xfrs += job->template.num_restart_files;
	
	gw_xfr_init(&(job->xfrs), num_xfrs, job->template.number_of_retries);
	
	job->xfrs.failure_limit = -1;
	
	gw_dm_epilog_std_files(job, &index);

	if (job->tm_state != GW_TM_STATE_CHECKPOINT)
		gw_dm_epilog_restart_files(job, &index);	
       
    gw_am_trigger(gw_dm.tm_am, "GW_TM_EPILOG", _job_id);
    
    pthread_mutex_unlock(&(job->mutex));
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_dm_kill_epilog ( void *_job_id )
{
    gw_job_t *   job;
    int          job_id;
    int          index;
    int          num_xfrs;
    
    /* ----------------------------------------------------------- */  
    /* 0.- Get job pointer                                         */
    /* ----------------------------------------------------------- */  
    
	if ( _job_id != NULL )
	{
		job_id = *( (int *) _job_id );

		job = gw_job_pool_get(job_id, GW_TRUE);

		if ( job == NULL )
		{
			gw_log_print("DM",'E',"Job %i does not exist (JOB_STATE_KILL_EPILOG).\n",job_id);			

			free(_job_id);
			return;
		}
	}
	else
		return;
	    
    /* ----------------------------------------------------------- */  
    /* 1.- Set state and times                                     */
    /* ----------------------------------------------------------- */  
    
	if (job->job_state != GW_JOB_STATE_KILL_EPILOG)
	{
	    gw_job_set_state(job, GW_JOB_STATE_KILL_EPILOG, GW_FALSE);
        job->history->stats[EPILOG_START_TIME] = time(NULL);
    }
    
    /* ----------------------------------------------------------- */  
    /* 2.- Signal the Transfer Manager                             */
    /* ----------------------------------------------------------- */

	gw_xfr_destroy (&(job->xfrs));
	
	if ( gw_job_is_wrapper_based(job) )
	{
		index    = 0;	
		num_xfrs = 2;
		
		gw_xfr_init(&(job->xfrs), num_xfrs, job->template.number_of_retries);
		
		job->xfrs.failure_limit = -1;
			
		gw_dm_epilog_std_wrapper_files(job, &index);		
	}
        
    gw_am_trigger(gw_dm.tm_am, "GW_TM_EPILOG", _job_id);
    
    pthread_mutex_unlock(&(job->mutex));
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_dm_epilog_fail (void *_job_id)
{
    gw_job_t *   job;
    int          job_id;
    int          index;
    int          num_xfrs;
    
    /* ----------------------------------------------------------- */  
    /* 0.- Get job pointer                                         */
    /* ----------------------------------------------------------- */  
    
	if ( _job_id != NULL )
	{
		job_id = *( (int *) _job_id );

		job = gw_job_pool_get(job_id, GW_TRUE);

		if ( job == NULL )
		{
			gw_log_print("DM",'E',"Job %i does not exist (JOB_STATE_EPILOG_FAIL).\n",job_id);			

			free(_job_id);
			return;
		}
	}
	else
		return;
	    
    /* ----------------------------------------------------------- */  
    /* 1.- Set state and times                                     */
    /* ----------------------------------------------------------- */  
    
	if (job->job_state != GW_JOB_STATE_EPILOG_FAIL)
	{
	    gw_job_set_state(job, GW_JOB_STATE_EPILOG_FAIL, GW_FALSE);
        job->history->stats[EPILOG_START_TIME] = time(NULL);
    }

    /* ----------------------------------------------------------- */  
    /* 2.- Signal the Transfer Manager                             */
    /* ----------------------------------------------------------- */

	gw_xfr_destroy (&(job->xfrs));

    if ( gw_job_is_wrapper_based(job) )
	{
		index    = 0;
		num_xfrs = 2;
		
		gw_xfr_init(&(job->xfrs), num_xfrs, job->template.number_of_retries);
		
		job->xfrs.failure_limit = -1;
			
		gw_dm_epilog_std_wrapper_files(job, &index);		
	}
	        
    gw_am_trigger(gw_dm.tm_am, "GW_TM_EPILOG", _job_id);
    
    pthread_mutex_unlock(&(job->mutex));
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_dm_epilog (void *_job_id)
{
    gw_job_t * job;
    int        job_id;
    int        index;
    int        num_xfrs;
    
    /* ----------------------------------------------------------- */  
    /* 0.- Get job pointer                                         */
    /* ----------------------------------------------------------- */  
    
	if ( _job_id != NULL )
	{
		job_id = *( (int *) _job_id );

		job = gw_job_pool_get(job_id, GW_TRUE);

		if ( job == NULL )
		{
			gw_log_print("DM",'E',"Job %i does not exist (JOB_STATE_EPILOG).\n",job_id);			
			free(_job_id);
			return;
		}
	}
	else
		return;
	    
    /* ----------------------------------------------------------- */  
    /* 1.- Set state                                               */
    /* ----------------------------------------------------------- */  
    
    if (job->job_state != GW_JOB_STATE_EPILOG_STD && 
        job->job_state != GW_JOB_STATE_EPILOG)
        job->history->stats[EPILOG_START_TIME] = time(NULL);

    if (job->job_state != GW_JOB_STATE_EPILOG)
    	gw_job_set_state(job, GW_JOB_STATE_EPILOG, GW_FALSE);
    
    /* ----------------------------------------------------------- */  
    /* 2.- Signal the Transfer Manager                             */
    /* ----------------------------------------------------------- */

	gw_xfr_destroy (&(job->xfrs));
	
	index    = 0;
	num_xfrs = 0
            + (job->template.stdout_file != NULL)
            + (job->template.stderr_file != NULL)
            + 2*(job->template.pre_wrapper != NULL)
            + 2*(job->template.monitor != NULL)
            + job->template.num_output_files;
	
	gw_xfr_init(&(job->xfrs), num_xfrs, job->template.number_of_retries);
	
	job->xfrs.failure_limit = -1;
	
 	gw_dm_epilog_output_files(job, &index);
 	
	gw_dm_epilog_std_files(job, &index);
 	
    gw_am_trigger(gw_dm.tm_am, "GW_TM_EPILOG", _job_id);
    
    pthread_mutex_unlock(&(job->mutex));
}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_dm_epilog_restart (void *_job_id)
{
    gw_job_t * job;
    int        job_id;
    int        index;
    int        num_xfrs;
    
    /* ----------------------------------------------------------- */  
    /* 0.- Get job pointer                                         */
    /* ----------------------------------------------------------- */  
    
	if ( _job_id != NULL )
	{
		job_id = *( (int *) _job_id );

		job = gw_job_pool_get(job_id, GW_TRUE);

		if ( job == NULL )
		{
			gw_log_print("DM",'E',"Job %i does not exist (JOB_STATE_EPILOG_RESTART).\n",job_id);			

			free(_job_id);
			return;
		}
	}
	else
		return;
	    
    /* ----------------------------------------------------------- */  
    /* 1.- Set state                                               */
    /* ----------------------------------------------------------- */  
    
	if (job->job_state != GW_JOB_STATE_EPILOG_RESTART)
	    gw_job_set_state(job, GW_JOB_STATE_EPILOG_RESTART, GW_FALSE);

    /* ----------------------------------------------------------- */  
    /* 2.- Signal the Transfer Manager                             */
    /* ----------------------------------------------------------- */

	gw_xfr_destroy (&(job->xfrs));
	
	index    = 0;
	num_xfrs = 0
            + (job->template.stdout_file != NULL)
            + (job->template.stderr_file != NULL)
            + 2*(job->template.pre_wrapper != NULL)
            + 2*(job->template.monitor != NULL)
            + job->template.num_output_files;
	
	if (job->tm_state != GW_TM_STATE_CHECKPOINT)
		num_xfrs += job->template.num_restart_files;
			
	gw_xfr_init(&(job->xfrs), num_xfrs, job->template.number_of_retries);
	
	job->xfrs.failure_limit = -1;
	
 	gw_dm_epilog_output_files(job, &index);
 		
	gw_dm_epilog_std_files(job, &index);
 	
	if (job->tm_state != GW_TM_STATE_CHECKPOINT)
	 	gw_dm_epilog_restart_files(job, &index);
 	
    gw_am_trigger(gw_dm.tm_am, "GW_TM_EPILOG", _job_id);
    
    pthread_mutex_unlock(&(job->mutex));
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_dm_epilog_done_cb ( void *_job_id )
{
    gw_job_t * job;
    int        job_id;
    time_t     total;
    int        rt;
	
    /* ----------------------------------------------------------- */  
    /* 0.- Get job pointer                                         */
    /* ----------------------------------------------------------- */  
    
	if ( _job_id != NULL )
	{
		job_id = *( (int *) _job_id );

		job = gw_job_pool_get(job_id, GW_TRUE);

		if ( job == NULL )
		{
			gw_log_print("DM",'E',"Job %i does not exist (JOB_STATE_EPILOG_DONE_CB).\n",job_id);

			free(_job_id);
			return;
		}
	}
	else
		return;

    /* ----------------------------------------------------------- */  
    /* 1.- Set EPILOG time                                         */
    /* ----------------------------------------------------------- */
    
    switch (job->job_state)
    {
    	case GW_JOB_STATE_EPILOG:
		case GW_JOB_STATE_EPILOG_FAIL:    			
		case GW_JOB_STATE_EPILOG_RESTART:
		case GW_JOB_STATE_STOP_EPILOG:
    	case GW_JOB_STATE_KILL_EPILOG:		
		    job->history->stats[EPILOG_EXIT_TIME] = time(NULL);
		    job->history->stats[EXIT_TIME]        = time(NULL);
		    total = gw_job_history_get_epilog_time(job->history);
		    
		    gw_job_print(job,"DM",'I',"Epilog done:\n");
		    gw_job_print(job,"DM",'I',"\tTotal time      : %i\n", total);
		    break;
		    
    	case GW_JOB_STATE_MIGR_EPILOG:
		    job->history->next->stats[EPILOG_EXIT_TIME] = time(NULL);
		    job->history->next->stats[EXIT_TIME]        = time(NULL);
            job->history->stats[MIGRATION_EXIT_TIME]    = time(NULL);
            total = gw_job_history_get_migration_time(job->history);

		    gw_job_print(job,"DM",'I',"Epilog done, job migrated.\n");
		    gw_job_print(job,"DM",'I',"\tMigration Time  : %i\n", total);		    
			break;
		
		case GW_JOB_STATE_EPILOG_STD:
			break;
			
    	default:
			gw_log_print("DM",'E',"Epilog done callback in wrong job (%i) state.\n", job_id);
			
			free(_job_id);
		    pthread_mutex_unlock(&(job->mutex));
		    return;   	
    }	

    /* ----------------------------------------------------------- */  
    /* 2.- State transition                                        */
    /* ----------------------------------------------------------- */
    
    switch (job->job_state)
    {
    	case GW_JOB_STATE_EPILOG:
    	case GW_JOB_STATE_KILL_EPILOG:    	    		
    		gw_am_trigger(&(gw_dm.am), "GW_DM_STATE_ZOMBIE", _job_id);
			break;
    		
    	case GW_JOB_STATE_MIGR_EPILOG:		    
			gw_am_trigger(&(gw_dm.am), "GW_DM_STATE_WRAPPER", _job_id);

		    gw_host_dec_rjobs(job->history->next->host);		    
    		break;
    	
    	case GW_JOB_STATE_STOP_EPILOG: 			
			gw_am_trigger(&(gw_dm.am), "GW_DM_STATE_STOPPED", _job_id);
    		break;
    				
		case GW_JOB_STATE_EPILOG_FAIL:		
			if (job->template.reschedule_on_failure == GW_TRUE)
				gw_am_trigger(&(gw_dm.am), "GW_DM_STATE_PENDING", _job_id);
		    else
				gw_am_trigger(&(gw_dm.am), "GW_DM_STATE_FAILED", _job_id);
			break;
			
 		case GW_JOB_STATE_EPILOG_RESTART:
 			switch (job->history->reason)
 			{
 				case GW_REASON_PERFORMANCE:
 				case GW_REASON_SELF_MIGRATION:
	 				gw_am_trigger(&(gw_dm.am), "GW_DM_STATE_PENDING", _job_id);
	 				break;
	 				
				default:
					if (job->template.reschedule_on_failure == GW_TRUE)
						gw_am_trigger(&(gw_dm.am), "GW_DM_STATE_PENDING", _job_id);
			    	else
						gw_am_trigger(&(gw_dm.am), "GW_DM_STATE_FAILED", _job_id);
					break;
 			}
			break;
    	
    	case GW_JOB_STATE_EPILOG_STD:    	
	    	rt = gw_dm_epilog_parse_wrapper_std(job);
	    	
	    	if (rt == 0)
				gw_am_trigger(&(gw_dm.am), "GW_DM_STATE_EPILOG", _job_id);
	    	else 
				gw_am_trigger(&(gw_dm.am), "GW_DM_STATE_EPILOG_RESTART", _job_id);
    		break;
    		
    	default:
    		break;    	
    }
    		
    pthread_mutex_unlock(&(job->mutex));
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_dm_epilog_failed_cb ( void *_job_id )
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
			gw_log_print("DM",'E',"Job %i does not exist (JOB_STATE_EPILOG_FAILED_CB).\n",job_id);

			free(_job_id);
			return;
		}
	}
	else
		return;

    /* ----------------------------------------------------------- */  
    /* 1.- Set EPILOG time                                         */
    /* ----------------------------------------------------------- */
    
    switch (job->job_state)
    {
    	case GW_JOB_STATE_EPILOG:
		case GW_JOB_STATE_EPILOG_FAIL:    			
		case GW_JOB_STATE_EPILOG_RESTART:
		case GW_JOB_STATE_STOP_EPILOG:
    	case GW_JOB_STATE_KILL_EPILOG:		
		    job->history->stats[EPILOG_EXIT_TIME] = time(NULL);
		    job->history->stats[EXIT_TIME]        = time(NULL);
		    total = gw_job_history_get_epilog_time(job->history);
		    
		    gw_job_print(job,"DM",'E',"Epilog failed:\n");
		    gw_job_print(job,"DM",'E',"\tTotal time      : %i\n", total);
		    break;
		    
    	case GW_JOB_STATE_MIGR_EPILOG:
		    job->history->next->stats[EPILOG_EXIT_TIME] = time(NULL);
		    job->history->next->stats[EXIT_TIME]        = time(NULL);
            job->history->stats[MIGRATION_EXIT_TIME]    = time(NULL);
            total = gw_job_history_get_migration_time(job->history);

		    gw_job_print(job,"DM",'E',"Epilog failed, job migrated\n");
		    gw_job_print(job,"DM",'E',"\tMigration Time  : %i\n", total);
			break;
		
		case GW_JOB_STATE_EPILOG_STD:
			break;
			
    	default:
			gw_log_print("DM",'E',"Epilog failed callback in wrong job (%i) state.\n", job_id);    	

			free(_job_id);
		    pthread_mutex_unlock(&(job->mutex));
		    return;   	
    }

    /* ----------------------------------------------------------- */  
    /* 2.- State transition                                        */
    /* ----------------------------------------------------------- */

    switch (job->job_state)
    {
    	case GW_JOB_STATE_EPILOG:			
	case GW_JOB_STATE_EPILOG_FAIL:		
            job->history->reason = GW_REASON_EXECUTION_ERROR;

            if (job->template.reschedule_on_failure == GW_TRUE)
		gw_am_trigger(&(gw_dm.am), "GW_DM_STATE_PENDING", _job_id);
	    else
		gw_am_trigger(&(gw_dm.am), "GW_DM_STATE_FAILED", _job_id);
            break;

    	case GW_JOB_STATE_KILL_EPILOG:    	    		
    		gw_am_trigger(&(gw_dm.am), "GW_DM_STATE_ZOMBIE", _job_id);
			break;
    		
    	case GW_JOB_STATE_MIGR_EPILOG:   	    				    
			gw_am_trigger(&(gw_dm.am), "GW_DM_STATE_WRAPPER", _job_id);

		    gw_host_dec_rjobs(job->history->next->host);		    
    		break;
    	
    	case GW_JOB_STATE_STOP_EPILOG: 			
			gw_am_trigger(&(gw_dm.am), "GW_DM_STATE_STOPPED", _job_id);
    		break;
    				
 		case GW_JOB_STATE_EPILOG_RESTART:
 			switch (job->history->reason)
 			{
 				case GW_REASON_PERFORMANCE:
 				case GW_REASON_SELF_MIGRATION: 				
	 				gw_am_trigger(&(gw_dm.am), "GW_DM_STATE_PENDING", _job_id);
	 				break;
	 				
				default:
					if (job->template.reschedule_on_failure == GW_TRUE)
						gw_am_trigger(&(gw_dm.am), "GW_DM_STATE_PENDING", _job_id);
			    	else
						gw_am_trigger(&(gw_dm.am), "GW_DM_STATE_FAILED", _job_id);
					break;					
 			} 		
			break;    	
    	
    	case GW_JOB_STATE_EPILOG_STD:
	    	job->history->reason = GW_REASON_EXECUTION_ERROR;
			gw_am_trigger(&(gw_dm.am), "GW_DM_STATE_EPILOG_RESTART", _job_id);
    		break;
    		
    	default:
    		break;    	
    }    

    pthread_mutex_unlock(&(job->mutex));
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_dm_epilog_std_wrapper_files(gw_job_t * job, int * index)
{	
    int  i;
    char url[512];
    
    i = *index;

    job->xfrs.failure_limit = i;

    snprintf(url,sizeof(char)*512,"file://%s/stdout.wrapper.%i",
            job->directory,job->restarted);
    
    job->xfrs.xfrs[i].src_url       = strdup("stdout.wrapper");
    job->xfrs.xfrs[i].dst_url       = strdup(url);
    job->xfrs.xfrs[i++].alt_src_url = NULL;

    snprintf(url,sizeof(char)*512,"file://%s/stderr.wrapper.%i",
            job->directory,job->restarted);
        
    job->xfrs.xfrs[i].src_url       = strdup("stderr.wrapper");
    job->xfrs.xfrs[i].dst_url       = strdup(url);
    job->xfrs.xfrs[i++].alt_src_url = NULL;
    	
    *index = i;
}

/* -------------------------------------------------------------------------- */

void gw_dm_epilog_std_files(gw_job_t * job, int * index)
{
    int  i;
    char url[512];
    
    i = *index;
	
   	/* Execution stdout & stderr */
   	    
    if (job->template.stdout_file != NULL)
    {
    	job->xfrs.failure_limit = i;
    	
       	job->xfrs.xfrs[i].src_url = strdup("stdout.execution");
        job->xfrs.xfrs[i].dst_url = strdup(job->template.stdout_file);
        job->xfrs.xfrs[i++].alt_src_url = NULL;
    }

    if (job->template.stderr_file != NULL)
    {
    	job->xfrs.failure_limit = i;
    	
        job->xfrs.xfrs[i].src_url = strdup("stderr.execution");
        job->xfrs.xfrs[i].dst_url = strdup(job->template.stderr_file);
        job->xfrs.xfrs[i++].alt_src_url = NULL;        
    }
    
   	/* Pre-wrapper stdout & stderr */
    
    if ( job->template.pre_wrapper != NULL )
    {
        snprintf(url,sizeof(char)*512,"file://%s/stdout.pre_wrapper.%i",
            job->directory,job->restarted);        
            
        job->xfrs.xfrs[i].src_url       = strdup("stdout.pre_wrapper");
        job->xfrs.xfrs[i].dst_url       = strdup(url);
        job->xfrs.xfrs[i++].alt_src_url = NULL;
        
        snprintf(url, sizeof(char)*512, "file://%s/stderr.pre_wrapper.%i",
                job->directory,job->restarted);    
                
        job->xfrs.xfrs[i].src_url       = strdup("stderr.pre_wrapper");
        job->xfrs.xfrs[i].dst_url       = strdup(url);
        job->xfrs.xfrs[i++].alt_src_url = NULL;
    }
    
   	/* Monitor stdout & stderr */
    
    if ( job->template.monitor != NULL )
    {
        snprintf(url,sizeof(char)*512,"file://%s/stdout.monitor.%i",
            job->directory,job->restarted);        
            
        job->xfrs.xfrs[i].src_url       = strdup("stdout.monitor");
        job->xfrs.xfrs[i].dst_url       = strdup(url);
        job->xfrs.xfrs[i++].alt_src_url = NULL;
        
        snprintf(url, sizeof(char)*512, "file://%s/stderr.monitor.%i",
                job->directory,job->restarted);    
                
        job->xfrs.xfrs[i].src_url       = strdup("stderr.monitor");
        job->xfrs.xfrs[i].dst_url       = strdup(url);
        job->xfrs.xfrs[i++].alt_src_url = NULL;
    }    

	*index = i;
}

/* -------------------------------------------------------------------------- */

void gw_dm_epilog_output_files(gw_job_t * job, int * index)
{
    int  i;
	
    for (i = 0; i< job->template.num_output_files ; i++)
    {
    	job->xfrs.xfrs[i+(*index)].src_url = strdup(job->template.output_files[i][GW_LOCAL_FILE]);
            
        if ( job->template.output_files[i][GW_REMOTE_FILE] != NULL )
        	job->xfrs.xfrs[i+(*index)].dst_url = strdup(job->template.output_files[i][GW_REMOTE_FILE]);
		else
            job->xfrs.xfrs[i+(*index)].dst_url = NULL;
                
        job->xfrs.xfrs[i+(*index)].alt_src_url = NULL;
	}
	
	job->xfrs.failure_limit = job->template.num_output_files - 1;
	
	*index = i + (*index);
}

/* -------------------------------------------------------------------------- */

void gw_dm_epilog_restart_files(gw_job_t * job, int * index)
{
    int  i;
    char url[512];
	
    for (i = 0; i< job->template.num_restart_files ; i++)
    {
		job->xfrs.xfrs[i+(*index)].src_url = strdup(job->template.restart_files[i]);

        snprintf(url,sizeof(char)*512,"%s/%s",
                 job->template.checkpoint_url,
                 job->template.restart_files[i]);
                    
        job->xfrs.xfrs[i+(*index)].dst_url     = strdup(url);
        job->xfrs.xfrs[i+(*index)].alt_src_url = NULL;
	}
	
	*index = i + (*index);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int gw_dm_epilog_parse_wrapper_std(gw_job_t *job)
{
    char         stdout_wrapper[512];
    int          rc, rt;
    char *       exit_code;
    char *       new_reqs;
    char *       new_rank;
    
    snprintf(stdout_wrapper,sizeof(char)*512,"%s/stdout.wrapper.%i",
                        job->directory, job->restarted);
	
    rc = gw_parse_file(stdout_wrapper, EXIT_STATUS, &exit_code);
	
    if ( ( rc != -1) && ( exit_code != NULL ) )
    {
    	switch ( exit_code[0] )
        {
        	case 'S':
            	gw_job_print(job,"DM",'I',"The application has requested a self-migration.\n");
				
				gw_parse_file(stdout_wrapper, "NEW_REQS", &new_reqs);
		            
		        if ( new_reqs != NULL )
		        {
					gw_parse_file(stdout_wrapper, "NEW_RANK", &new_rank);
		            	
		            if ( new_rank != NULL)
		            {
		            	if ( job->template.requirements != NULL )
		            		free(job->template.requirements);
		            		
		            	if ( job->template.rank != NULL )
		            		free(job->template.rank);
		            			
		            	job->template.requirements = new_reqs;
		            	job->template.rank         = new_rank;		            		
		            }
		            else
		            {
		            	gw_job_print(job,"DM",'E',"Could not find new rank.\n");
		            	free(new_reqs);
		            }		            
				}
		        else
	            	gw_job_print(job, "DM",'E',"Could not find new requirements.\n");
	
                job->history->reason = GW_REASON_SELF_MIGRATION;
                
                rt = 1;
				break;
                    
			case 'P' :
            	gw_job_print(job,"DM",'I',"The application has detected a performance slowdown.\n");
                        
                job->history->reason = GW_REASON_PERFORMANCE;
                rt = 1;

                break;
	                    
			default:
            	job->exit_code = atoi(exit_code);
            	rt = 0;
            	break;
		}
                
        free (exit_code);
	}
	else
    {
    	if ( rc == -1 )
        	gw_job_print(job,"DM",'E',"Unable to open wrapper stdout %s.\n",strerror(errno));
		else
        	gw_job_print(job,"DM",'E',"Unable to find exit code, assuming that the job failed or was cancelled.\n");
	        	
      	job->history->reason = GW_REASON_EXECUTION_ERROR;
	      	
        rt = -1;
	} 
	
	return rt;
}
