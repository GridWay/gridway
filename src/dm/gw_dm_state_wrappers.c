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
#include <stdlib.h>
#include <errno.h>

#include "gw_job.h"
#include "gw_log.h"
#include "gw_dm.h"

void gw_dm_pre_wrapper ( void *_job_id )
{
    gw_job_t * job;
    int        job_id;
    
	/* ----------------------------------------------------------- */  
    /* 0.- Get job pointer                                         */
    /* ----------------------------------------------------------- */  
    
	if ( _job_id != NULL )
	{
		job_id = *( (int *) _job_id );

		job = gw_job_pool_get(job_id, GW_TRUE);

		if ( job == NULL )
		{
			gw_log_print("DM",'E',"Job %i does not exist (JOB_STATE_PRE_WRAPPER).\n",job_id);

			free(_job_id);
			return;
		}
	}
	else
		return;
    
    /* ----------------------------------------------------------- */  
    /* 1.- Set state and times                                     */
    /* ----------------------------------------------------------- */  

    job->history->stats[PRE_WRAPPER_START_TIME] = time(NULL);
        
	gw_job_set_state(job, GW_JOB_STATE_PRE_WRAPPER, GW_FALSE);
    
    /* ----------------------------------------------------------- */  
    /* 2.- Signal the Execution Manager                            */
    /* ----------------------------------------------------------- */
        
    job->em_state = GW_EM_STATE_INIT;
        
    gw_am_trigger(gw_dm.em_am, "GW_EM_SUBMIT", _job_id);
    
    pthread_mutex_unlock(&(job->mutex));
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_dm_wrapper ( void *_job_id )
{
    gw_job_t * job;
    int        job_id;
    
	/* ----------------------------------------------------------- */  
    /* 0.- Get job pointer                                         */
    /* ----------------------------------------------------------- */  
    
	if ( _job_id != NULL )
	{
		job_id = *( (int *) _job_id );

		job = gw_job_pool_get(job_id, GW_TRUE);

		if ( job == NULL )
		{
			gw_log_print("DM",'E',"Job %i does not exist (JOB_STATE_WRAPPER).\n",job_id);

			free(_job_id);
			return;
		}
	}
	else
		return;
    
    /* ----------------------------------------------------------- */  
    /* 1.- Set state and times                                     */
    /* ----------------------------------------------------------- */  

    job->history->stats[WRAPPER_START_TIME] = time(NULL);

 	gw_job_set_state(job, GW_JOB_STATE_WRAPPER, GW_FALSE);
    
    /* ----------------------------------------------------------- */  
    /* 2.- Signal the Execution Manager                            */
    /* ----------------------------------------------------------- */
        
    job->em_state = GW_EM_STATE_INIT;
        
    gw_am_trigger(gw_dm.em_am, "GW_EM_SUBMIT", _job_id);
    
    pthread_mutex_unlock(&(job->mutex));
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_dm_wrapper_done_cb ( void *_job_id )
{
	gw_job_t *     job;
    int            job_id;
    time_t         total;
    time_t         active;
    time_t         suspension;

	/* ----------------------------------------------------------- */  
    /* 0.- Get job pointer                                         */
    /* ----------------------------------------------------------- */  
    
	if ( _job_id != NULL )
	{
		job_id = *( (int *) _job_id );

		job = gw_job_pool_get(job_id, GW_TRUE);

		if ( job == NULL )
		{
			gw_log_print("DM",'E',"Job %i does not exist (JOB_STATE_WRAPPER_DONE_CB).\n",job_id);
			
			free(_job_id);
			return;
		}
	}
	else
		return;
    
    /* ----------------------------------------------------------- */  
    /* 1.- Set execution times & state transition                  */
    /* ----------------------------------------------------------- */  
    
    job->em_state = GW_EM_STATE_INIT;
    
    switch (job->job_state)
    {
    	case GW_JOB_STATE_PRE_WRAPPER:
    		
    		/* --------------- Update pre-wrapper stats -------------------- */
    	
		    job->history->stats[PRE_WRAPPER_EXIT_TIME] = time(NULL);
		    
		    total      = gw_job_history_get_pre_wrapper_time(job->history);
		    active     = job->history->stats[ACTIVE_TIME];
    		suspension = job->history->stats[SUSPENSION_TIME];

		    gw_job_print(job,"DM",'I',"Pre-Wrapper DONE:\n");
		    gw_job_print(job,"DM",'I',"\tActive time     : %i\n", active);
		    gw_job_print(job,"DM",'I',"\tSuspension time : %i\n", suspension);
		    gw_job_print(job,"DM",'I',"\tTotal time      : %i\n", total);

    		/* -------------- Transition to Wrapper state ------------------ */
    				    
		    gw_am_trigger(&(gw_dm.am), "GW_DM_STATE_WRAPPER", _job_id);		    
    		break;
    		
    	case GW_JOB_STATE_WRAPPER:
    	
    		/* ----------------- Update wrapper stats ---------------------- */
    		
		    job->history->stats[WRAPPER_EXIT_TIME] = time(NULL);
		    
		    total      = gw_job_history_get_wrapper_time(job->history);
		    active     = job->history->stats[ACTIVE_TIME];
    		suspension = job->history->stats[SUSPENSION_TIME];

		    gw_job_print(job,"DM",'I',"Wrapper DONE:\n");
		    gw_job_print(job,"DM",'I',"\tActive time     : %i\n", active);
		    gw_job_print(job,"DM",'I',"\tSuspension time : %i\n", suspension);
		    gw_job_print(job,"DM",'I',"\tTotal time      : %i\n", total);

    		/* -------------- Free used slot from this host -------------- */

            gw_host_dec_uslots(job->history->host, job->template.np);
            
    		/* ---------- We do not need to re-schedule this job --------- */
    				    
			if ( job->reschedule == GW_TRUE )
			{
			    job->reschedule = GW_FALSE;
			    gw_dm_mad_job_del(&gw_dm.dm_mad[0],job->id);				
			}
			            		                
    	    /* -------------- Transition to Epilog state ------------------ */
            
            if ( gw_job_is_wrapper_based(job) )
                gw_am_trigger(&(gw_dm.am), "GW_DM_STATE_EPILOG_STD", _job_id);			
            else
                gw_am_trigger(&(gw_dm.am), "GW_DM_STATE_EPILOG", _job_id);
            break;
    	
    	case GW_JOB_STATE_STOP_CANCEL:
    	
    		/* ----------------- Update wrapper stats ---------------------- */    	
    		
            job->history->stats[WRAPPER_EXIT_TIME] = time(NULL);
		    
            total      = gw_job_history_get_wrapper_time(job->history);
            active     = job->history->stats[ACTIVE_TIME];
            suspension = job->history->stats[SUSPENSION_TIME];

            gw_job_print(job,"DM",'I',"Wrapper CANCELED:\n");
            gw_job_print(job,"DM",'I',"\tActive time     : %i\n", active);
            gw_job_print(job,"DM",'I',"\tSuspension time : %i\n", suspension);
            gw_job_print(job,"DM",'I',"\tTotal time      : %i\n", total);

            /* -------------- Free used slot from this host -------------- */
    		
            gw_host_dec_uslots(job->history->host, job->template.np);
			    		
            /* ------------ Transition to Stop Epilog state --------------- */
    		
            gw_am_trigger(&(gw_dm.am), "GW_DM_STATE_STOP_EPILOG", _job_id);		    
            break;
    	
    	case GW_JOB_STATE_KILL_CANCEL:

    		/* ----------------- Update wrapper stats ---------------------- */
    		    	    	    	
		    job->history->stats[WRAPPER_EXIT_TIME] = time(NULL);
		    
		    total      = gw_job_history_get_wrapper_time(job->history);
		    active     = job->history->stats[ACTIVE_TIME];
    		suspension = job->history->stats[SUSPENSION_TIME];

		    gw_job_print(job,"DM",'I',"Wrapper CANCELED:\n");
		    gw_job_print(job,"DM",'I',"\tActive time     : %i\n", active);
		    gw_job_print(job,"DM",'I',"\tSuspension time : %i\n", suspension);
		    gw_job_print(job,"DM",'I',"\tTotal time      : %i\n", total);    	

    		/* -------------- Free used slot from this host -------------- */

            gw_host_dec_uslots(job->history->host, job->template.np);
			            
    		/* ------------ Transition to Kill Epilog state ---------------- */
    		
			gw_am_trigger(&(gw_dm.am), "GW_DM_STATE_KILL_EPILOG", _job_id);		    
    		break;
    	
    	case GW_JOB_STATE_MIGR_CANCEL:

    		/* ----------- Update previous wrapper stats ------------------- */
    		    	
            job->history->next->stats[WRAPPER_EXIT_TIME] = time(NULL);
            
            active     = job->history->next->stats[ACTIVE_TIME];
            suspension = job->history->next->stats[SUSPENSION_TIME];

		    gw_job_print(job,"DM",'I',"Wrapper CANCELED:\n");
		    gw_job_print(job,"DM",'I',"\tActive time     : %i\n", active);
		    gw_job_print(job,"DM",'I',"\tSuspension time : %i\n", suspension);

    		/* -------------- Free used slot from previous host ------------ */
    		
            gw_host_dec_uslots(job->history->next->host, job->template.np);
	    			    
    		/* ---------- Transition to Migration Prolog state ------------ */
    		
			gw_am_trigger(&(gw_dm.am), "GW_DM_STATE_MIGR_PROLOG", _job_id);		    
    		break;

    	default:
			gw_log_print("DM",'E',"Wrapper done callback for job %i in wrong state.\n", job_id);
			
			free(_job_id);    	    	
    		break;    	
    }
    
    pthread_mutex_unlock(&(job->mutex));    
}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_dm_wrapper_failed_cb ( void *_job_id )
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
			gw_log_print("DM",'E',"Job %i does not exist (JOB_STATE_WRAPPER_FAILED_CB).\n",job_id);

			free(_job_id);
			return;
		}
	}
	else
		return;

    /* ----------------------------------------------------------- */  
    /* 1.- Set execution times & state transition                  */
    /* ----------------------------------------------------------- */  
   
   job->em_state = GW_EM_STATE_INIT;
    
    switch (job->job_state)
    {
    	case GW_JOB_STATE_PRE_WRAPPER:

    		/* --------------- Update pre-wrapper stats -------------------- */
    	
		    job->history->stats[PRE_WRAPPER_EXIT_TIME] = time(NULL);
		    total = gw_job_history_get_pre_wrapper_time(job->history);

		    gw_job_print(job,"DM",'E',"Pre-Wrapper failed:\n");
		    gw_job_print(job,"DM",'E',"\tTotal time      : %i\n", total);
    		break;
    		
    	case GW_JOB_STATE_WRAPPER:
    	
    		/* ----------------- Update wrapper stats ---------------------- */
    		
		    job->history->stats[WRAPPER_EXIT_TIME] = time(NULL);		    
		    total = gw_job_history_get_wrapper_time(job->history);

		    gw_job_print(job,"DM",'E',"Wrapper failed:\n");
		    gw_job_print(job,"DM",'E',"\tTotal time      : %i\n", total);		    
		    
    		/* ---------- We do not need to re-schedule this job --------- */
    				    
			if ( job->reschedule == GW_TRUE )
			{
			    job->reschedule = GW_FALSE;
			    gw_dm_mad_job_del(&gw_dm.dm_mad[0],job->id);				
			}
					    		   
    		break;
  	
    	default:
			gw_log_print("DM",'E',"Wrapper failed callback in wrong job (%i) state.\n", job_id);
    		break;    	
    }
    

    /* ----------------------------------------------------------- */  
    /* 1.- State transtition                                       */
    /* ----------------------------------------------------------- */  
    
	/* -------------- Free used slot from this host -------------- */
	
	if (job->history != NULL)
	{
		job->history->reason = GW_REASON_EXECUTION_ERROR;
        gw_host_dec_uslots(job->history->host, job->template.np);
	}		
   	
   	/* ----------------------------------------------------------- */

	gw_am_trigger(&(gw_dm.am), "GW_DM_STATE_EPILOG_FAIL", _job_id);

    pthread_mutex_unlock(&(job->mutex));
}
