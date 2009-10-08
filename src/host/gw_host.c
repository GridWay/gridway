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
#include <stdio.h>
#include <time.h>
#include <pthread.h>

#include "gw_host.h"
#include "gw_common.h"
#include "gw_conf.h"
#include "gw_log.h"
#include "gw_host_pool.h"
#include "gw_dm.h"

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

void gw_host_init(gw_host_t *host, char *hostname, int host_id, int fixed_priority,
        char *em_mad, char *tm_mad, char *im_mad)
{
    int i;

    pthread_mutex_init(&(host->mutex),(pthread_mutexattr_t *) NULL);

    pthread_mutex_lock(&(host->mutex));

    host->hostname       = strdup(hostname);
    host->host_id        = host_id;
    host->fixed_priority = fixed_priority;

    host->arch       = NULL;
    host->os_name    = NULL;
    host->os_version = NULL;
    host->cpu_model  = NULL;

    host->em_mad = strdup(em_mad);
    host->tm_mad = strdup(tm_mad);
    host->im_mad = strdup(im_mad);

    host->cpu_mhz      = 0;
    host->cpu_free     = 0;
    host->cpu_smp      = 0;
    host->free_mem_mb  = 0;
    host->size_mem_mb  = 0;
    host->free_disk_mb = 0;
    host->size_disk_mb = 0;
    host->nodecount    = 0;
    host->used_slots   = 0;
    host->running_jobs = 0;

    host->fork_name = NULL;
    host->lrms_name = NULL;
    host->lrms_type = NULL;
    
    for (i= 0; i<GW_HOST_MAX_QUEUES; i++)
    {
        host->queue_name[i]           = NULL;
        host->queue_status[i]         = NULL;
        host->queue_dispatchtype[i]   = NULL;
        host->queue_priority[i]       = NULL;
                
        host->queue_nodecount[i]      = 0;
        host->queue_freenodecount[i]  = 0;
        host->queue_maxtime[i]        = 0;
        host->queue_maxcputime[i]     = 0;
        host->queue_maxcount[i]       = 0;
        host->queue_maxrunningjobs[i] = 0;
        host->queue_maxjobsinqueue[i] = 0;
    }

    for (i= 0; i<GW_HOST_MAX_GENVARS; i++)
    {
        host->genvar_int[i].name  = NULL;
        host->genvar_int[i].value = 0;
        
        host->genvar_str[i].name  = NULL;
        host->genvar_str[i].value = NULL;
    }

    host->state = GW_HOST_STATE_DISCOVERED;
    
    host->last_monitoring_time = 0;
    
    pthread_mutex_unlock(&(host->mutex));
}

/*----------------------------------------------------------------------------*/

void gw_host_destroy(gw_host_t *host)
{
    int i;
    
    if ( host->hostname != NULL )
    	free(host->hostname);

    if ( host->arch != NULL )    	
    	free(host->arch);
    	
    if ( host->os_name != NULL )    	
    	free(host->os_name);

    if ( host->os_version != NULL )    
    	free(host->os_version);

    if ( host->cpu_model != NULL )    
    	free(host->cpu_model);

    if ( host->em_mad != NULL )
	    free(host->em_mad);

    if ( host->tm_mad != NULL )    
    	free(host->tm_mad);    

    if ( host->im_mad != NULL )        
	    free(host->im_mad);    

    if ( host->fork_name != NULL )        
	    free(host->fork_name);

    if ( host->lrms_name != NULL )        
	    free(host->lrms_name);

    if ( host->lrms_type != NULL )	    
	    free(host->lrms_type);
    
    for (i= 0; i<GW_HOST_MAX_QUEUES; i++)
    {
   	    if ( host->queue_name[i] != NULL )
	        free(host->queue_name[i]);

   	    if ( host->queue_dispatchtype[i] != NULL )    
	        free(host->queue_dispatchtype[i]);
	        
   	    if ( host->queue_priority[i] != NULL )    
        	free(host->queue_priority[i]);
        
   	    if ( host->queue_status[i] != NULL )
	        free(host->queue_status[i]);
    }
    
    for (i= 0; i<GW_HOST_MAX_GENVARS; i++)
    {
        if ( host->genvar_int[i].name != NULL )
            free(host->genvar_int[i].name);
            
        if ( host->genvar_str[i].name  != NULL )
            free(host->genvar_str[i].name);
        
        if ( host->genvar_str[i].value != NULL )
            free(host->genvar_str[i].value);
    }    
 
    pthread_mutex_unlock(&(host->mutex));

    pthread_mutex_destroy(&(host->mutex));
}

/*----------------------------------------------------------------------------*/

void gw_host_update(int host_id, char *attrs)
{
    int rc;
    gw_host_t *host;
    
    host = gw_host_pool_get_host(host_id,GW_TRUE);
    
    if ( host == NULL )
    {
        gw_log_print("IM",'E',"Error updating host %i, not found.",
                     host_id);
    	return;
    }
    	
    /* Parse attrs and update host */
    
    rc = gw_host_update_attr(host, attrs);
 	
    if ( rc != 0 )
    {
        gw_log_print("IM",'E',"Error updating host %i attributes, parse error.\n",
                     host_id);
                     
        host->state = GW_HOST_STATE_UNKNOWN;                     
                     
        pthread_mutex_unlock(&(host->mutex));
        return;
    }

    host->state = GW_HOST_STATE_MONITORED;
    
    /* Notify the scheduler */
    
    gw_dm_mad_host_monitor(&gw_dm.dm_mad[0],
                           host->host_id,
                           host->used_slots,
                           host->running_jobs,
                           host->hostname);

    pthread_mutex_unlock(&(host->mutex));
}

/*----------------------------------------------------------------------------*/

void gw_host_clear_dynamic_info(int host_id)
{
    int i;
    gw_host_t *host;
    
    host = gw_host_pool_get_host(host_id,GW_TRUE);
    
    if ( host == NULL )
    {
        gw_log_print("IM",'E',"Error clearing host %i information, not found.",
                     host_id);
    	return;
    }

    host->cpu_free     = 0;
    host->nodecount    = 0;
        
    host->free_mem_mb  = 0;
    host->size_mem_mb  = 0;
    
    host->free_disk_mb = 0;
    host->size_disk_mb = 0;

    for (i= 0; i<GW_HOST_MAX_QUEUES; i++)
    {
        if ( host->queue_status[i] != NULL )
            free(host->queue_status[i]);
            
        host->queue_status[i]        = NULL;
        host->queue_freenodecount[i] = 0;
    }

    host->state = GW_HOST_STATE_UNKNOWN;

    host->last_monitoring_time = 0;
    
    pthread_mutex_unlock(&(host->mutex));
}

/*----------------------------------------------------------------------------*/

void gw_host_dec_rjobs(gw_host_t *host)
{
    pthread_mutex_lock(&(host->mutex));			

	host->running_jobs--;
			
    gw_dm_mad_host_monitor(&gw_dm.dm_mad[0],
                           host->host_id,
                           host->used_slots,
                           host->running_jobs,
                           host->hostname);
                                   
	pthread_mutex_unlock(&(host->mutex));
}

/*----------------------------------------------------------------------------*/

void gw_host_dec_uslots(gw_host_t *host, int slots)
{
    pthread_mutex_lock(&(host->mutex));			

	host->used_slots-= slots;
			
    gw_dm_mad_host_monitor(&gw_dm.dm_mad[0],
                           host->host_id,
                           host->used_slots,
                           host->running_jobs,
                           host->hostname);
                                   
	pthread_mutex_unlock(&(host->mutex));
}

/*----------------------------------------------------------------------------*/

void gw_host_dec_slots(gw_host_t *host, int slots)
{
    pthread_mutex_lock(&(host->mutex));			

	host->used_slots-= slots;
	
	host->running_jobs--;
				
    gw_dm_mad_host_monitor(&gw_dm.dm_mad[0],
                           host->host_id,
                           host->used_slots,
                           host->running_jobs,
                           host->hostname);
                                   
	pthread_mutex_unlock(&(host->mutex));
}

/*----------------------------------------------------------------------------*/

void gw_host_inc_rjobs(gw_host_t *host)
{
    pthread_mutex_lock(&(host->mutex));			

	host->running_jobs++;			
			
    gw_dm_mad_host_monitor(&gw_dm.dm_mad[0],
                           host->host_id,
                           host->used_slots,
                           host->running_jobs,
                           host->hostname);
                                   
	pthread_mutex_unlock(&(host->mutex));
}

/*----------------------------------------------------------------------------*/


void gw_host_inc_uslots(gw_host_t *host, int slots)
{
    pthread_mutex_lock(&(host->mutex));			

	host->used_slots+= slots;
			
    gw_dm_mad_host_monitor(&gw_dm.dm_mad[0],
                           host->host_id,
                           host->used_slots,
                           host->running_jobs,
                           host->hostname);
                                   
	pthread_mutex_unlock(&(host->mutex));
}

/*----------------------------------------------------------------------------*/

void gw_host_inc_slots(gw_host_t *host, int slots)
{
    pthread_mutex_lock(&(host->mutex));			

	host->used_slots+= slots;
	
	host->running_jobs++;
				
    gw_dm_mad_host_monitor(&gw_dm.dm_mad[0],
                           host->host_id,
                           host->used_slots,
                           host->running_jobs,
                           host->hostname);
                                   
	pthread_mutex_unlock(&(host->mutex));
}

/*----------------------------------------------------------------------------*/

void gw_host_inc_slots_nb(gw_host_t *host, int slots)
{
	host->used_slots+= slots;
	
	host->running_jobs++;
				
    gw_dm_mad_host_monitor(&gw_dm.dm_mad[0],
                           host->host_id,
                           host->used_slots,
                           host->running_jobs,
                           host->hostname);
}

/*----------------------------------------------------------------------------*/

void gw_host_inc_rjobs_nb(gw_host_t *host)
{
	host->running_jobs++;
				
    gw_dm_mad_host_monitor(&gw_dm.dm_mad[0],
                           host->host_id,
                           host->used_slots,
                           host->running_jobs,
                           host->hostname);
}

/*----------------------------------------------------------------------------*/

void gw_host_print(FILE *fd, gw_host_t *host)
{
    int i;
    char str[500];
    char str2[500];
    
    if (host != NULL)
    {
        pthread_mutex_lock(&(host->mutex));

        gw_print (fd,"IM",'I',"\t-------------- Host info.  --------------\n");
        gw_print (fd,"IM",'I',"\tName = %s\n", GWNSTR(host->hostname));
        gw_print (fd,"IM",'I',"\tOS   = %s %s\n", GWNSTR(host->os_name), GWNSTR(host->os_version));
        gw_print (fd,"IM",'I',"\tCPU  = %s (%s) at %i MHz\n",
                GWNSTR(host->cpu_model), GWNSTR(host->arch), host->cpu_mhz);
        gw_print (fd,"IM",'I',"\tMem  = %i of %i MB\n",
                host->free_mem_mb, host->size_mem_mb);
        gw_print (fd,"IM",'I',"\tDisk = %i of %i MB\n",
                host->free_disk_mb, host->size_disk_mb);
        gw_print (fd,"IM",'I',"\tLRMS = %s (%s) with %i nodes\n",
                GWNSTR(host->lrms_type), GWNSTR(host->lrms_name), host->nodecount);
        if (host->queue_name[0] != NULL)
            gw_print (fd,"IM",'I',"\t                  NC FNC    MT   MCT  MC MRJ MJQ\n");

        for (i = 0; i<GW_HOST_MAX_QUEUES; i++)
        {
            if (host->queue_name[i] != NULL)
            {
                sprintf (str,"QUEUE= %-8s (%3i %3i %5i %5i %3i %3i %3i)",
                        GWNSTR(host->queue_name[i]), host->queue_nodecount[i],
                        host->queue_freenodecount[i], host->queue_maxtime[i],
                        host->queue_maxcputime[i], host->queue_maxcount[i],
                        host->queue_maxrunningjobs[i],
                        host->queue_maxjobsinqueue[i]);
                if (host->queue_status[i] != NULL)
                {
                    sprintf (str2, ", %s status", 
                                GWNSTR(host->queue_status[i]));
                    strncat (str, str2, sizeof(str));
                }
                if (host->queue_dispatchtype[i] != NULL)
                {
                    sprintf (str2, ", %s type",
                                GWNSTR(host->queue_dispatchtype[i]));
                    strncat (str, str2, sizeof(str));
                }
                if (host->queue_priority[i] != NULL)
                {
                    sprintf (str2, ", %s priority",
                                GWNSTR(host->queue_priority[i]));
                    strncat (str, str2, sizeof(str));
                }
                gw_print (fd, "IM",'I',"\t%s\n", str);
            }
        }
        gw_print (fd,"IM",'I',"\t-----------------------------------------\n");

        pthread_mutex_unlock(&(host->mutex));
    }
}


