/* -------------------------------------------------------------------------- */
/* Copyright 2002-2011, GridWay Project Leads (GridWay.org)                   */
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

    static time_t last_discovery_time = 0;
	static int mark = 0;
	
	mark = mark + 5;
	if ( mark >= 300 )
	{
    	gw_log_print ("IM",'I',"-- MARK --\n");
    	mark = 0;
	}

    the_time = time(NULL);
    
    discovery_interval  = gw_conf.discovery_interval;

    if (the_time - last_discovery_time >= discovery_interval) 
    {
        gw_log_print ("IM",'I',"Discovering hosts.\n");

        last_discovery_time = the_time;
        
        for (i=0; (i<GW_MAX_MADS) && (gw_conf.im_mads[i][0] != NULL); i++) 
        {
			gw_im_discover (gw_conf.im_mads[i][GW_MAD_IM_NAME_INDEX]);
        }

    }
    
    gw_host_pool_monitor_hosts(&gw_im.active_queries);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
