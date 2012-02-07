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
#include <sys/types.h> 
#include <sys/stat.h> 
#include <unistd.h> 
 
#include "gw_array_pool.h"
#include "gw_job_pool.h"
#include "gw_conf.h"
#include "gw_log.h"

/* -------------------------------------------------------------------------- */
static gw_array_pool_t gw_array_pool;
/* -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

gw_array_pool_t * gw_array_pool_init()
{
    int i;
    
    pthread_mutex_init(&(gw_array_pool.mutex),(pthread_mutexattr_t *)NULL); 
    
    pthread_mutex_lock(&(gw_array_pool.mutex));

    gw_array_pool.pool = (gw_array_t **) malloc( sizeof(gw_array_t *)
                                         * gw_conf.number_of_arrays);

    if (gw_array_pool.pool == NULL)
    {
        pthread_mutex_unlock(&(gw_array_pool.mutex));
        pthread_mutex_destroy(&(gw_array_pool.mutex));
        return NULL;
    }
        
    for ( i=0; i < gw_conf.number_of_arrays ; i++)
        gw_array_pool.pool[i] = NULL;

    gw_array_pool.number_of_arrays = 0;
    gw_array_pool.last_array_id    = -1;
    
    pthread_mutex_unlock(&(gw_array_pool.mutex));

    gw_log_print("DM",'I',"Array pool initialized.\n");

    return (&gw_array_pool);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_array_pool_finalize()
{
  int i;

  pthread_mutex_lock(&(gw_array_pool.mutex));

  for ( i=0; i < gw_conf.number_of_arrays ; i++)
      if ( gw_array_pool.pool[i] != NULL)
      {
      	  pthread_mutex_lock(&((gw_array_pool.pool[i])->mutex));
      	  
          gw_array_pool.number_of_arrays--;
          
		  gw_array_destroy (gw_array_pool.pool[i]);
		  
          free(gw_array_pool.pool[i]);
          
          gw_array_pool.pool[i] = NULL;
      }
      
  free(gw_array_pool.pool);

  pthread_mutex_unlock(&(gw_array_pool.mutex));

  pthread_mutex_destroy(&(gw_array_pool.mutex));
  
  gw_log_print("DM",'I',"Array pool destroyed.\n");

}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_array_pool_array_free (int array_id)
{
  gw_array_t *array;

    if ( (array_id >= 0) && (array_id < gw_conf.number_of_arrays) )
    {
  	    pthread_mutex_lock(&(gw_array_pool.mutex));
  	    
        array = gw_array_pool.pool[array_id];
        
        if ( array != NULL )
        {
        	pthread_mutex_lock(&(array->mutex));
        	
	        gw_array_pool.number_of_arrays--;
        	
			gw_array_pool.pool[array_id] = NULL;        	
			
            gw_array_destroy (array);
            
	        free(array);            
        }
                  
        pthread_mutex_unlock(&(gw_array_pool.mutex));
    }
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int gw_array_pool_array_allocate (const gw_msg_t * msg, 
                                  int              number_of_tasks, 
                                  int *            array_id)
{
    int jid, i, tid;
    int found, tries, rc;
    gw_job_t *job;
    gw_array_t *array;

    pthread_mutex_lock(&(gw_array_pool.mutex));
    
    /* ------- Check if there is enough space for the array -------- */
    
    if ( number_of_tasks + gw_job_pool_get_num_jobs() > 
                                                gw_conf.number_of_jobs )
    {
        pthread_mutex_unlock(&(gw_array_pool.mutex));    
        return -1;
    }
    
    *array_id = ( gw_array_pool.last_array_id + 1 )
                                           % gw_conf.number_of_arrays;
    found = 0;
    tries = 0;
    
    while(!found && (tries < gw_conf.number_of_arrays))
    {
      found = gw_array_pool.pool[*array_id] == NULL;
      if(!found)
      {
        tries++;
        *array_id = (*array_id+1) % gw_conf.number_of_arrays;
      }  
    } 
    
    if (!found)
    {
        pthread_mutex_unlock(&(gw_array_pool.mutex));    
        return -1;
    }
    
    gw_array_pool.last_array_id   = *array_id;
    
    gw_array_pool.pool[*array_id] = (gw_array_t *) malloc (sizeof (gw_array_t) );
    
    array = gw_array_pool.pool[*array_id];
    
    rc = gw_array_init (array, number_of_tasks, *array_id);
  
    if (rc != 0)
    {
        pthread_mutex_unlock(&(gw_array_pool.mutex));    
        return -1;
    }
  
  	pthread_mutex_lock(&(array->mutex));
  	  
    for (i = 0; i<number_of_tasks ; i++)
    {
        jid = gw_job_pool_allocate();
        tid = gw_array_add_task(array, jid);

        job = gw_job_pool_get(jid, GW_TRUE);
        
        job->task_id     = tid;
        job->array_id    = *array_id;
        job->total_tasks = number_of_tasks;
        
        pthread_mutex_unlock(&(job->mutex));        
    }
    
   	pthread_mutex_unlock(&(array->mutex));
    
    gw_array_pool.number_of_arrays++;

    pthread_mutex_unlock(&(gw_array_pool.mutex));    
    
    return 0;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

gw_array_t* gw_array_pool_get_array (int array_id, gw_boolean_t lock)
{
    gw_array_t * array;

    if ( (array_id >= 0) && (array_id < gw_conf.number_of_arrays) )
    {
  	    pthread_mutex_lock(&(gw_array_pool.mutex));
  	    
        array = gw_array_pool.pool[array_id];
        
        if ( (lock == GW_TRUE) && (array != NULL) )
            pthread_mutex_lock(&(array->mutex));
            
        pthread_mutex_unlock(&(gw_array_pool.mutex));
    }
    else
        array = NULL;

    return (array);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int gw_array_pool_get_number_tasks (int array_id)
{
    gw_array_t *array;
    int        num_tasks=1;
    
    if ( (array_id >= 0) && (array_id < gw_conf.number_of_arrays) )
    {
        array     = gw_array_pool.pool[array_id];
        if (array != NULL)
            num_tasks = array->number_of_tasks;
    }

    return (num_tasks);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int gw_array_pool_get_num_arrays()
{
	return gw_array_pool.number_of_arrays;
}
