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

#include <stdlib.h>
#include "gw_job_pool.h"
#include "gw_log.h"
#include "gw_conf.h"
#include "gw_user_pool.h"
#include "gw_dm.h"

#ifdef HAVE_GLOBUS_USAGE
#include "globus_usage.h"
#endif

#ifdef HAVE_LIBDB
#include "gw_acct.h"
#endif

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

static gw_job_pool_t       gw_job_pool;

static gw_job_dep_matrix_t gw_job_deps;

#ifdef HAVE_GLOBUS_USAGE
static globus_usage_stats_handle_t usage_handle = NULL;
#endif

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

static int gw_job_dep_init()
{
  int i;
  
  pthread_mutex_init(&(gw_job_deps.mutex),(pthread_mutexattr_t *) NULL);

  pthread_mutex_lock(&(gw_job_deps.mutex));
  
  gw_job_deps.deps = (int **) malloc( sizeof(int *) * gw_conf.number_of_jobs);
  
  if ( gw_job_deps.deps == NULL )
  	return -1;
  	
  for ( i=0; i< gw_conf.number_of_jobs; i++ )
  	gw_job_deps.deps[i] = NULL;
  
  pthread_mutex_unlock(&(gw_job_deps.mutex));
  
  return 0;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

static void gw_job_dep_destroy()
{
  int i;
  
  pthread_mutex_lock(&(gw_job_deps.mutex));
  
  if ( gw_job_deps.deps != NULL )
  {
  	
	  for ( i=0; i< gw_conf.number_of_jobs; i++ )
	  	if (gw_job_deps.deps[i] != NULL)
	  	{
	  		free(gw_job_deps.deps[i]);
	  		gw_job_deps.deps[i] = NULL;
	  	}
  }
  
  pthread_mutex_unlock(&(gw_job_deps.mutex));
  
  pthread_mutex_destroy(&(gw_job_pool.mutex));
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_job_pool_dep_cp (const int * src, int **dst)
{
	int i = 0;
	
	if (src == NULL)
	{
		*dst = NULL;
		return;
	}
	
	while (src[i]!=-1)
    	i++;

	if ( i == 0 )
		*dst = NULL;
	else
	{
		*dst = (int *) malloc (sizeof(int)*(i+1));
		
		i = 0;		
		while (src[i] != -1)
		{
			(*dst)[i] = src[i];
			i++;
		}
		
		(*dst)[i] = -1;
	}	
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
 
void gw_job_pool_dep_set(int job_id, int *deps)
{
	pthread_mutex_lock(&(gw_job_deps.mutex));

    if ( ( job_id >= 0 ) && ( job_id < gw_conf.number_of_jobs ) )
    {
    	if ( gw_job_deps.deps[job_id] != NULL )
    		free(gw_job_deps.deps[job_id]);
    
	    gw_job_pool_dep_cp (deps, &(gw_job_deps.deps[job_id]));
    }
	
	pthread_mutex_unlock(&(gw_job_deps.mutex));
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_job_pool_dep_check(int job_id)
{
	int i=0;
	int j=0;
	gw_boolean_t all_done;
	gw_job_t *   job;
	
	pthread_mutex_lock(&(gw_job_deps.mutex));

	for ( i=0; i<gw_conf.number_of_jobs; i++)
	{
    	if ( gw_job_deps.deps[i] != NULL )
    	{
    		all_done = GW_TRUE;
    		j = 0;
    		
	    	while ( gw_job_deps.deps[i][j] != -1 )
	    	{
	    		if ( gw_job_deps.deps[i][j] == job_id )
	    		{
	    			gw_job_deps.deps[i][j] = -2;
	    		}
	    		else if ( gw_job_deps.deps[i][j] != -2 )
	    			all_done = GW_FALSE;
	    			
	    		j++;
	    	}
    		
    		if ( all_done == GW_TRUE ) /* release the job */
    		{
				job = gw_job_pool_get(i, GW_TRUE);

				if ( job != NULL )
				{
					if ( job->job_state == GW_JOB_STATE_HOLD )
					{
						gw_log_print("DM",'I',"Dependencies of job %i satisfied, releasing job.\n",i);
						
						gw_job_set_state(job, GW_JOB_STATE_PENDING, GW_FALSE);
						
                        gw_dm_mad_job_schedule(&gw_dm.dm_mad[0],
                                               job->id,
                                               job->array_id,
                                               job->user_id,
                                               GW_REASON_NONE);
					}
					
					pthread_mutex_unlock(&(job->mutex));
				}
    		}
    	}
	}
	    	    		
	pthread_mutex_unlock(&(gw_job_deps.mutex));
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_job_pool_dep_consistency()
{
	int          i,j;
	gw_boolean_t all_done;	
	gw_job_t *   job;
	int          jid;
	
	pthread_mutex_lock(&(gw_job_deps.mutex));
	pthread_mutex_lock(&(gw_job_pool.mutex));	
	
	/* Remove the following jobs from the dependencies matrix:
	 *     1.- "ZOMBIE"
	 *     2.- "KILLED" (the user may killed this job before gwd crash)
	 */
	
	for ( i=0; i < gw_conf.number_of_jobs; i++)
	{
    	if ( gw_job_deps.deps[i] != NULL )
    	{
    		all_done = GW_TRUE;
    		j = 0;
    		
	    	while ( gw_job_deps.deps[i][j] != -1 )
	    	{
	    		jid = gw_job_deps.deps[i][j];
	    		
	    		if ((jid >= 0) && (jid < gw_conf.number_of_jobs))
	    		{
					if (gw_job_pool.pool[jid] != NULL)
      				{
						pthread_mutex_lock(&((gw_job_pool.pool[jid])->mutex));
						
						if ( (gw_job_pool.pool[jid])->job_state 
								== GW_JOB_STATE_ZOMBIE )
							gw_job_deps.deps[i][j] = -2;
						else 
							all_done = GW_FALSE;
							
						pthread_mutex_unlock(&((gw_job_pool.pool[jid])->mutex));
						
      				} else if (gw_job_pool.pool[jid] == NULL)
      					gw_job_deps.deps[i][j] = -2;
	    		}
	    		else
	    			gw_job_deps.deps[i][j] = -2;

	    		j++;
	    	}
    		
    		if ( all_done == GW_TRUE ) /* release the job */
    		{
				job = gw_job_pool.pool[i];
				
				if (job != NULL)
				{
					pthread_mutex_lock(&(job->mutex));
				
					if ( job->job_state == GW_JOB_STATE_HOLD )
					{
						gw_log_print("DM",'I',"Dependencies of job %i satisfied, releasing job.\n",i);
						
						gw_job_set_state(job, GW_JOB_STATE_PENDING, GW_FALSE);
						
                        gw_dm_mad_job_schedule(&gw_dm.dm_mad[0],
                                               job->id,
                                               job->array_id,
                                               job->user_id,
                                               GW_REASON_NONE);
					}

					pthread_mutex_unlock(&(job->mutex));					
				}
			}
    	}
	}

	pthread_mutex_unlock(&(gw_job_pool.mutex));		
	pthread_mutex_unlock(&(gw_job_deps.mutex));	
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

gw_job_pool_t * gw_job_pool_init()
{
  int i;

#ifdef HAVE_GLOBUS_USAGE
  globus_result_t rc;
  globus_object_t *err = GLOBUS_NULL;
  char *errstr=GLOBUS_NULL;
#endif

  i = gw_job_dep_init();
  
  if ( i != 0 )
  	return NULL;
  	
  pthread_mutex_init(&(gw_job_pool.mutex),(pthread_mutexattr_t *) NULL);

  pthread_mutex_lock(&(gw_job_pool.mutex));

  gw_job_pool.pool = (gw_job_t**) malloc(gw_conf.number_of_jobs
          * sizeof(gw_job_t*));
  gw_job_pool.number_of_jobs = 0;
  gw_job_pool.last_job_id    = -1;

  if (gw_job_pool.pool == NULL)
  {
      pthread_mutex_unlock(&(gw_job_pool.mutex));
      pthread_mutex_destroy(&(gw_job_pool.mutex));
      return NULL;
  }

  for ( i=0; i < gw_conf.number_of_jobs; i++)
      gw_job_pool.pool[i] = NULL;

  pthread_mutex_unlock(&(gw_job_pool.mutex));
  
  gw_log_print("DM",'I',"Job pool initialized.\n");

#ifdef HAVE_GLOBUS_USAGE
  rc = globus_module_activate(GLOBUS_USAGE_MODULE);

  if (rc != GLOBUS_SUCCESS) {
    err = globus_error_get(rc);
    errstr = globus_object_printable_to_string(err);
    gw_log_print("DM",'E',"USAGE: Error activating module: %s.\n", errstr);
  } else {
    gw_log_print("DM",'I',"USAGE: Module activated.\n");
  }

  rc = globus_usage_stats_handle_init(&usage_handle, 25,
      0, NULL); //statistics.ige-project.eu:4810

  if (rc != GLOBUS_SUCCESS) {
    err = globus_error_get(rc);
    errstr = globus_object_printable_to_string(err);
    gw_log_print("DM",'E',"USAGE: Error initializing handle: %s.\n", errstr);
  } else {
    gw_log_print("DM",'I',"USAGE: Handle initialized.\n");
  }
#endif
  
  return (&gw_job_pool);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_job_pool_finalize()
{
  int i;

  pthread_mutex_lock(&(gw_job_pool.mutex));

  for ( i=0; i < gw_conf.number_of_jobs; i++)
      if (gw_job_pool.pool[i] != NULL)
      {
      	   	pthread_mutex_lock(&((gw_job_pool.pool[i])->mutex));            
  
	        gw_job_pool.number_of_jobs--;
        
	        gw_job_destroy (gw_job_pool.pool[i]);
	        
	        free(gw_job_pool.pool[i]);
	        
	        gw_job_pool.pool[i] = NULL;
      }
      
  free(gw_job_pool.pool);
  
  pthread_mutex_unlock(&(gw_job_pool.mutex));

  pthread_mutex_destroy(&(gw_job_pool.mutex));

  gw_job_dep_destroy();
  
  gw_log_print("DM",'I',"Job pool destroyed.\n");
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int gw_job_pool_allocate ()
{
    int found,tries;
    int job_id;
    
    pthread_mutex_lock(&(gw_job_pool.mutex));

    job_id = (gw_job_pool.last_job_id+1)% gw_conf.number_of_jobs;

    found = 0;
    tries = 0;

    while(!found && (tries < gw_conf.number_of_jobs))
    {
        found = gw_job_pool.pool[job_id] == NULL;

        if(!found)
        {
            tries++;
            job_id = (job_id + 1) % gw_conf.number_of_jobs;
        }
    }

    if (!found)
    {
        pthread_mutex_unlock(&(gw_job_pool.mutex));
        return -1;
    }

    gw_job_pool.pool[job_id] = (gw_job_t *) malloc (sizeof(gw_job_t));

    gw_job_pool.last_job_id = job_id;
    
    gw_job_pool.number_of_jobs++;
    
    gw_job_init (gw_job_pool.pool[job_id], job_id);

    pthread_mutex_unlock(&(gw_job_pool.mutex));
    
    return job_id;
}
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int gw_job_pool_allocate_by_id (int job_id)
{
	if (( job_id < 0 ) || ( job_id >= gw_conf.number_of_jobs ))
	{
		gw_log_print("DM",'E',"job id %i is out of range, modify gwd.conf\n",job_id);		
		return -1;
	}
		
    pthread_mutex_lock(&(gw_job_pool.mutex));
		
	if (gw_job_pool.pool[job_id] != NULL)
	{
		gw_log_print("DM",'E',"Could not allocate job %i, already exists\n",job_id);
		
        pthread_mutex_unlock(&(gw_job_pool.mutex));
        return -1;			
	}
	
    gw_job_pool.pool[job_id] = (gw_job_t *) malloc (sizeof(gw_job_t));
    
    gw_job_pool.number_of_jobs++;
    
    gw_job_init (gw_job_pool.pool[job_id], job_id);

    pthread_mutex_unlock(&(gw_job_pool.mutex));
    
    return job_id;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


void gw_job_pool_free (int job_id)
{
    gw_job_t *            job;
    gw_migration_reason_t reason;
    
    if ( ( job_id >= 0 ) && ( job_id < gw_conf.number_of_jobs ) )
    {
    	pthread_mutex_lock(&(gw_job_pool.mutex));
    	
        job = gw_job_pool.pool[job_id];
        
        if ( job != NULL )
        {
            pthread_mutex_lock(&(job->mutex));
        	
            if ( job->history == NULL )
                reason = GW_REASON_NONE;
            else
                reason = job->history->reason;
           
            if (job->job_state == GW_JOB_STATE_PENDING
                    || (job->job_state  == GW_JOB_STATE_WRAPPER
                        && job->reschedule == GW_TRUE))
                gw_dm_mad_job_del(&gw_dm.dm_mad[0],job->id);

            if ( job->exit_time == 0 )
            {
                job->exit_time = time(NULL);
				
                if ( job->history != NULL 
                        && job->history->stats[EXIT_TIME] == 0 )
                    job->history->stats[EXIT_TIME] = time(NULL);
            }
#ifdef HAVE_LIBDB			
            gw_acct_write_job(job);
#endif

            gw_user_pool_dec_jobs(job->user_id);        	         
            gw_job_pool.number_of_jobs--;
            gw_job_pool.pool[job_id] = NULL;
            gw_job_destroy (job);
            free(job);
        }
        
        pthread_mutex_unlock(&(gw_job_pool.mutex));
	    
    	pthread_mutex_lock(&(gw_job_deps.mutex));
    	
        if ( gw_job_deps.deps[job_id] != NULL )
        {
            free(gw_job_deps.deps[job_id]);
            gw_job_deps.deps[job_id] = NULL;
        }

    	pthread_mutex_unlock(&(gw_job_deps.mutex));
    }
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

gw_job_t* gw_job_pool_get (int job_id, int lock)
{
    gw_job_t *job;

    if ( ( job_id >= 0 ) && ( job_id < gw_conf.number_of_jobs ) )
    {
   	    pthread_mutex_lock(&(gw_job_pool.mutex));
   	    
        job = gw_job_pool.pool[job_id];
        
        if ( (lock == GW_TRUE) && (job != NULL) )
           pthread_mutex_lock(&(job->mutex));            
        
        pthread_mutex_unlock(&(gw_job_pool.mutex));
    }
    else
        job = NULL;

    return (job);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int gw_job_pool_em_recover (gw_em_mad_t * em_mad, gw_am_t *em_am)
{
    gw_job_t * job;
    int        i;
    char *     job_contact;
    int *      _job_id; 
 
    pthread_mutex_lock(&(gw_job_pool.mutex));
    
    for (i=0; i < gw_conf.number_of_jobs; i++)
    {
        job = gw_job_pool.pool[i];
        
        if (job != NULL)
        {
            pthread_mutex_lock(&(job->mutex));            

            if ((job->history != NULL)
                    && (job->history->em_mad == em_mad)
                    && (job->job_state == GW_JOB_STATE_WRAPPER))
            {
                job_contact = gw_job_recover_get_contact(job);
                
                if ( job_contact != NULL )
                {
#ifdef GWJOBDEBUG
                    gw_log_print("DM",'D',"Recovering job %i, contact is %s.\n", 
                            job->id, job_contact);
#endif
                    gw_em_mad_recover(em_mad, job->id, job_contact);
                    
                    free(job_contact);      
                }
                else
                {
#ifdef GWJOBDEBUG
                    gw_log_print("DM",'D',"Could not recover job %i, no contact.\n", 
                            job->id);
#endif

                    _job_id    = (int *) malloc (sizeof(int));
                    *(_job_id) = job->id;

                    gw_am_trigger(em_am, "GW_EM_STATE_FAILED",
                            (void *) _job_id);
                }
            }

            pthread_mutex_unlock(&(job->mutex));
        }
    }

    pthread_mutex_unlock(&(gw_job_pool.mutex));
    
    return 0;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int gw_job_pool_dm_recover (gw_dm_mad_t * dm_mad)
{
    gw_job_t * job;
    int        i;
        
    pthread_mutex_lock(&(gw_job_pool.mutex));
    
    for ( i=0; i < gw_conf.number_of_jobs; i++)
    {
        job = gw_job_pool.pool[i];
        
        if (job != NULL)
        {
            pthread_mutex_lock(&(job->mutex));            

            if (job->job_state == GW_JOB_STATE_PENDING)
            {
#ifdef GWDMDEBUG
                gw_log_print("DM",'D',"Recovering (sched) job %i.\n",job->id);
#endif                
                gw_dm_mad_job_schedule(dm_mad,
                                   job->id,
                                   job->array_id,
                                   job->user_id,
                                   GW_REASON_NONE);                
            }

            pthread_mutex_unlock(&(job->mutex));
        }
    }
        
    pthread_mutex_unlock(&(gw_job_pool.mutex));
    
    return 0;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_job_pool_tm_recover (gw_am_t *dm_am)
{
    gw_job_t * job;
    int        i;
    int *      _job_id; 
        
    pthread_mutex_lock(&(gw_job_pool.mutex));
    
    for ( i=0; i < gw_conf.number_of_jobs; i++)
    {
        job = gw_job_pool.pool[i];
        
        if (job != NULL)
        {
            pthread_mutex_lock(&(job->mutex));            

            if (job->job_state == GW_JOB_STATE_PROLOG)
            {
#ifdef GWTMDEBUG
                gw_log_print("TM",'D',"MAD reloaded, recovering job %i from prolog.\n",job->id);
		        gw_job_print(job,"TM",'D',"MAD reloaded, restarting Prolog\n");
#endif                
		        _job_id    = (int *) malloc (sizeof(int));
                *(_job_id) = job->id;
			
                /* Reset TM state */ 				
                job->tm_state = GW_TM_STATE_INIT;
		
                /* Trigger the DM Prolog state */
                gw_am_trigger(dm_am, "GW_DM_STATE_PROLOG", _job_id);
            }
      	    else
            {
#ifdef GWTMDEBUG
	        gw_log_print("TM",'D',"MAD reloaded, recovering job %i from epilog.\n",job->id);
  	    	gw_job_print(job,"TM",'D',"MAD reloaded, restarting Epilog\n");
#endif					
                _job_id    = (int *) malloc (sizeof(int));
                *(_job_id) = job->id;

                /* Reset TM state */ 
                job->tm_state = GW_TM_STATE_INIT;

                switch(job->job_state)
                {
                    case GW_JOB_STATE_EPILOG:
                        gw_am_trigger(dm_am, "GW_DM_STATE_EPILOG", _job_id);					
                        break;
                        
                    case GW_JOB_STATE_EPILOG_STD:
                        gw_am_trigger(dm_am, "GW_DM_STATE_EPILOG_STD", _job_id);				
                        break;
                        
                    case GW_JOB_STATE_MIGR_EPILOG:
                        gw_am_trigger(dm_am, "GW_DM_STATE_MIGR_EPILOG", _job_id);				
                        break;
                        
                    case GW_JOB_STATE_EPILOG_RESTART:
                        gw_am_trigger(dm_am, "GW_DM_STATE_EPILOG_RESTART", _job_id);				
                        break;
                        						
                    case GW_JOB_STATE_EPILOG_FAIL:
                        gw_am_trigger(dm_am, "GW_DM_STATE_EPILOG_FAIL", _job_id);			
                        break;
                        
                    case GW_JOB_STATE_KILL_EPILOG:
                        gw_am_trigger(dm_am, "GW_DM_STATE_KILL_EPILOG", _job_id);			
                        break;
                        	
                    case GW_JOB_STATE_STOP_EPILOG:
                        gw_am_trigger(dm_am, "GW_DM_STATE_STOP_EPILOG", _job_id);			
                        break;
                        
                    default:
                        break;											
                }						
            }

            pthread_mutex_unlock(&(job->mutex));
        }
    }
        
    pthread_mutex_unlock(&(gw_job_pool.mutex));
    
    return;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int gw_job_pool_get_num_jobs()
{
	return gw_job_pool.number_of_jobs;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int gw_job_send_usage(gw_job_t *job)
{
#ifdef HAVE_GLOBUS_USAGE
    globus_result_t rc;
    globus_object_t *err = GLOBUS_NULL;
    char *errstr=GLOBUS_NULL;

    /*gw_history_t * p;*/

    char start_time[256];
    char exit_time[256];

    /*job->total_tasks;
    job->template.type;
    job->template.np;
    job->exit_code;
    job->restarted;*/

    /* Calculate stats from all hosts in history */
    /*cpu_time = 0;
    xfr_time = 0;

    p = job->history;
    while (p != NULL )
    {
        cpu_time += gw_job_history_get_wrapper_time(p);
        xfr_time += gw_job_history_get_prolog_time(p)
                + gw_job_history_get_epilog_time(p)
                + gw_job_history_get_migration_time(p);
        p = p->next;
    }*/

    sprintf(start_time, "%ld", job->start_time);
    sprintf(exit_time, "%ld", job->exit_time);

    rc = globus_usage_stats_send(usage_handle, 2,
            "START_TIME", start_time,
            "EXIT_TIME", exit_time);

    if (rc != GLOBUS_SUCCESS) {
        err = globus_error_get(rc);
        errstr = globus_object_printable_to_string(err);
        gw_log_print("DM",'E',"USAGE: Error sending stats: %s.\n", errstr);
    } else {
        gw_log_print("DM",'I',"USAGE: Stats sent.\n");
    }
#endif

    return 0;
}
