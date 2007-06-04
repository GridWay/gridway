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

#include "gw_im.h"
#include "gw_host.h"
#include "gw_host_pool.h"
#include "gw_log.h"
#include "gw_conf.h"

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_im_discover(char *mad_name)
{
    gw_im_mad_t   *mad;

    if ( mad_name == NULL ) 
        return;

    mad = gw_im_get_mad_by_name(mad_name);

    if ( mad == NULL )
    {
        gw_log_print("IM",'E',"Unable to discover hosts with MAD %s, not registered.\n",
                     mad_name);
        return;
    }

    if ( mad->state != GW_IM_MAD_STATE_IDLE )
    {
#ifdef GWIMDEBUG         
        gw_log_print("IM",'D',"Not ready to discover hosts with MAD %s.\n", mad_name);
#endif        
        return;
    }

    mad->state = GW_IM_MAD_STATE_DISCOVERING;

    pthread_mutex_lock(&(gw_im.mutex));

    gw_im.active_queries++;

#ifdef GWIMDEBUG
    gw_log_print ("IM",'D',"Discovering hosts with MAD %s, %i active queries.\n",
            mad_name, gw_im.active_queries);
#endif

    pthread_mutex_unlock(&(gw_im.mutex));

    gw_im_mad_discover(mad);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_im_monitor(gw_host_t * host)
{
    gw_im_mad_t * mad;
    char *        mad_name;

    if ((host->state != GW_HOST_STATE_DISCOVERED) && 
        (host->state != GW_HOST_STATE_MONITORED ))
    {
#ifdef GWIMDEBUG         
        gw_log_print("IM",'D',"Not ready to monitor host %d (%s).\n",
                     host->host_id, host->hostname);
#endif        
        return;
    }       
    
    mad_name = host->im_mad;
    mad      = gw_im_get_mad_by_name (mad_name);

    if ( mad == NULL ) 
    {
        gw_log_print("IM",'E',"MAD (%s) not found.\n",mad_name);
        return;
    }

    host->state = GW_HOST_STATE_MONITORING;

    pthread_mutex_lock(&(gw_im.mutex));

    gw_im.active_queries++;

#ifdef GWIMDEBUG
    gw_log_print ("IM",'D',"Monitoring host %i (\"%s\"), %i active queries.\n",
            host->host_id, host->hostname, gw_im.active_queries);
#endif

    pthread_mutex_unlock(&(gw_im.mutex));
    
    gw_im_mad_monitor(mad, host->host_id, host->hostname);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_im_timer(void *_null)
{
    int i;
    
    time_t the_time;
    time_t discovery_interval;
    time_t monitoring_interval;
    static time_t last_discovery_time = 0;
    static int next_host_to_check = 0;
	static int mark = 0;
    gw_host_t *host;
    int hid;
	
	mark = mark + 5;
	if ( mark >= 300 )
	{
    	gw_log_print ("IM",'I',"-- MARK --\n");
    	mark = 0;
	}

    the_time = time(NULL);
    
    discovery_interval  = gw_conf.discovery_interval;
    monitoring_interval = gw_conf.monitoring_interval;
       
    if (the_time - last_discovery_time >= discovery_interval) 
    {
        gw_log_print ("IM",'I',"Discovering hosts.\n");

        last_discovery_time = the_time;
        
        for (i=0; (i<GW_MAX_MADS) && (gw_conf.im_mads[i][0] != NULL); i++) 
        {
			gw_im_discover (gw_conf.im_mads[i][GW_MAD_NAME_INDEX]);
        }

    }
    
    /* Go through the host poll checking the last monitoring time */
    hid = next_host_to_check;
    
#ifdef GWIMDEBUG                    
    gw_log_print ("IM",'D',"Checking hosts starting with %d...\n", hid);
#endif                    

    while (gw_im.active_queries < gw_conf.max_active_im_queries)
    {
#ifdef GWIMDEBUG                    
        gw_log_print ("IM",'D',"Checking host %d...\n", hid);
#endif

        host = gw_host_pool_get_host(hid, GW_TRUE);
        
        if (host == NULL)
        {
            if (hid > 0)
            {
                next_host_to_check = 0;
                break;
            }
            else
            {
#ifdef GWIMDEBUG
                gw_log_print ("IM",'D',"No hosts to monitor.\n");
#endif
                next_host_to_check = 0;
                break;
            }
        }

        if (host->last_monitoring_time == 0
                || the_time - host->last_monitoring_time >= monitoring_interval)
        {
            host->last_monitoring_time = the_time;

            gw_im_monitor(host);
        }

        pthread_mutex_unlock(&(host->mutex));

        hid++;
    }
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
