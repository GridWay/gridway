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

#include <stdlib.h>
#include <string.h>

#include "gw_host_pool.h"
#include "gw_log.h"
#include "gw_conf.h"
#include "gw_sch_conf.h"
#include "gw_im.h"

/* -------------------------------------------------------------------------- */

static gw_host_pool_t gw_host_pool;

/* -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

gw_host_pool_t * gw_host_pool_init()
{
  int i;

  pthread_mutex_init(&(gw_host_pool.mutex),(pthread_mutexattr_t *) NULL);

  pthread_mutex_lock(&(gw_host_pool.mutex));

  gw_host_pool.pool = (gw_host_t**) malloc(gw_conf.number_of_hosts
                                                           * sizeof(gw_host_t*));
  gw_host_pool.number_of_hosts = 0;
  gw_host_pool.last_host_id    = -1;

  if (gw_host_pool.pool == NULL)
  {
      pthread_mutex_unlock(&(gw_host_pool.mutex));
      pthread_mutex_destroy(&(gw_host_pool.mutex));
      return NULL;
  }

  for ( i=0; i < gw_conf.number_of_hosts; i++)
      gw_host_pool.pool[i] = NULL;

  pthread_mutex_unlock(&(gw_host_pool.mutex));

  gw_log_print("IM",'I',"Host pool initialized.\n");
  
  return (&gw_host_pool);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_host_pool_finalize()
{
  int i;

  pthread_mutex_lock(&(gw_host_pool.mutex));

  for ( i=0; i < gw_conf.number_of_hosts; i++)
      if (gw_host_pool.pool[i] != NULL)
      {
      	  	pthread_mutex_lock(&((gw_host_pool.pool[i])->mutex));
	    	
	        gw_host_pool.number_of_hosts--;

	        gw_host_destroy (gw_host_pool.pool[i]);

	        free(gw_host_pool.pool[i]);
	                
	        gw_host_pool.pool[i] = NULL;
      }

  free(gw_host_pool.pool);

  pthread_mutex_unlock(&(gw_host_pool.mutex));

  pthread_mutex_destroy(&(gw_host_pool.mutex));
  
  gw_log_print("IM",'I',"Host pool destroyed.\n");
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int gw_host_pool_host_allocate (char* hostname, int fixed_priority,
        char *em_mad, char *tm_mad, char *im_mad)
{
    int found,tries;
    int host_id;
    
    pthread_mutex_lock(&(gw_host_pool.mutex));

    host_id = (gw_host_pool.last_host_id+1)% gw_conf.number_of_hosts;

    found = 0;
    tries = 0;

    while(!found && (tries < gw_conf.number_of_hosts))
    {
        found = gw_host_pool.pool[host_id] == NULL;

        if(!found)
        {
            tries++;
            host_id = (host_id + 1) % gw_conf.number_of_hosts;
        }
    }

    if (!found)
    {
        pthread_mutex_unlock(&(gw_host_pool.mutex));
        return -1;
    }

    gw_host_pool.pool[host_id] = (gw_host_t *) malloc (sizeof(gw_host_t));

    gw_host_pool.last_host_id = host_id;
    
    pthread_mutex_unlock(&(gw_host_pool.mutex));

    gw_host_init(gw_host_pool.pool[host_id], 
                 hostname, 
                 host_id, 
                 fixed_priority,
                 em_mad, 
                 tm_mad, 
                 im_mad);

    gw_host_pool.number_of_hosts++;

    return host_id;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


void gw_host_pool_host_free (int host_id)
{
    if ( ( host_id >= 0 ) && ( host_id < gw_conf.number_of_hosts ) )
    {
		pthread_mutex_lock(&(gw_host_pool.mutex));    	
        
	    if ( gw_host_pool.pool[host_id] != NULL )
	    {
	    	pthread_mutex_lock(&((gw_host_pool.pool[host_id])->mutex));
	    	
	        gw_host_pool.number_of_hosts--;

	        gw_host_destroy (gw_host_pool.pool[host_id]);

	        free(gw_host_pool.pool[host_id]);
	                
	        gw_host_pool.pool[host_id] = NULL;
		}
		
		pthread_mutex_unlock(&(gw_host_pool.mutex));        
    }
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

gw_host_t* gw_host_pool_get_host (int host_id, gw_boolean_t lock)
{
    gw_host_t *host;

    pthread_mutex_lock(&(gw_host_pool.mutex));

    if ( ( host_id >= 0 ) && ( host_id < gw_conf.number_of_hosts ) )
    {
        host = gw_host_pool.pool[host_id];
        
        if ( (lock == GW_TRUE) && (host != NULL) )
            pthread_mutex_lock(&(host->mutex));            
    }
    else
        host = NULL;

    pthread_mutex_unlock(&(gw_host_pool.mutex));

    return (host);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

gw_host_t* gw_host_pool_search (char *hostname, gw_boolean_t lock)
{
    gw_host_t *host;
    int i;

    pthread_mutex_lock(&(gw_host_pool.mutex));

    i = 0;
    host = NULL;
    while (i < gw_conf.number_of_hosts && host == NULL) 
    {
        if (gw_host_pool.pool[i] != NULL
                && strcmp(gw_host_pool.pool[i]->hostname, hostname) == 0)
        {
            host = gw_host_pool.pool[i];

            if (lock == GW_TRUE)
                pthread_mutex_lock(&(host->mutex));
        }
        i++;
    }

    pthread_mutex_unlock(&(gw_host_pool.mutex));

    return (host);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_host_pool_update (char *hostnames, char *em_mad, char *tm_mad, char *im_mad)
{
    char      *hostname;
    char      *lasts;
    gw_host_t *host;
    int       host_id;
    int       priority;

    hostname = strtok_r(hostnames, " ", &lasts);

    while (hostname != NULL)
    {
        host = gw_host_pool_search(hostname, GW_TRUE);

        if (host == NULL) 
        {
        	priority = gw_sch_get_host_priority(&(gw_conf.sch_conf), 
        	                                    hostname,
                                                im_mad);
                                                
            host_id = gw_host_pool_host_allocate(hostname, 
                                                 priority, 
                                                 em_mad,
                                                 tm_mad, 
                                                 im_mad);
                                                 
            host    = gw_host_pool_get_host (host_id, GW_TRUE);
            
            if ( host != NULL )
            {
                host->last_monitoring_time = 0;
                
                pthread_mutex_unlock(&(host->mutex));
            }
        }
        else
        {
            if (host->state == GW_HOST_STATE_UNKNOWN)
                host->state = GW_HOST_STATE_DISCOVERED;

            pthread_mutex_unlock(&(host->mutex));
        }

        hostname = strtok_r(NULL, " ", &lasts);
    }
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_host_pool_monitor_hosts( int *active_queries)
{
    static int last_checked = 0;
    
    int i=0;
    int hid;
    int num_hosts;
    
    time_t monitoring_interval;
    time_t the_time;
        
    gw_host_t * host;
    
    pthread_mutex_lock(&(gw_host_pool.mutex));
    
    num_hosts = gw_host_pool.number_of_hosts;
    
    if ( num_hosts == 0 )
    {
        pthread_mutex_unlock(&(gw_host_pool.mutex));
        return;
    }
    
    monitoring_interval = gw_conf.monitoring_interval;    
    the_time            = time(NULL);
    
    hid = (last_checked + 1) % num_hosts;    

#ifdef GWIMDEBUG                    
    gw_log_print ("IM",'D',"Checking hosts starting with %d...\n", hid);
#endif   
            
    while ((i<num_hosts) && (*active_queries < gw_conf.max_active_im_queries))
    {
        host = gw_host_pool.pool[hid];
        
        if ( host != NULL )
        {
            if (host->last_monitoring_time == 0
                || the_time - host->last_monitoring_time >= monitoring_interval)
            {
#ifdef GWIMDEBUG
                gw_log_print ("IM",'D',"\tMonitoring host %d.\n", hid);
#endif
                last_checked = hid;
                host->last_monitoring_time = the_time;

                gw_im_monitor(host);
            }
        }
        else
            gw_log_print("IM",'E',"Host %d no longer exists.\n", hid);
        
        hid = ( hid + 1 ) % num_hosts;
        i++;
    }

    pthread_mutex_unlock(&(gw_host_pool.mutex));       
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_host_pool_print (FILE *fd)
{
    int i;

    pthread_mutex_lock(&(gw_host_pool.mutex));

    for (i = 0; i<gw_conf.number_of_hosts; i++)
        if (gw_host_pool.pool[i] != NULL)
            gw_host_print(fd, gw_host_pool.pool[i]);

    pthread_mutex_unlock(&(gw_host_pool.mutex));
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
