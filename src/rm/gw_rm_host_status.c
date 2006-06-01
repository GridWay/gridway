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
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>

#include "gw_rm.h"
#include "gw_rm_msg.h"
#include "gw_log.h"
#include "gw_host_pool.h"

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

void gw_rm_host_to_msg (gw_host_t *host, gw_msg_host_t *msg)
{
	int i;
	int number_of_queues;
	int number_of_int_vars;
	int number_of_str_vars;
	
	msg->msg_type = GW_MSG_HOST_STATUS;
	msg->rc       = GW_RC_SUCCESS;

	gw_rm_copy_str_short(host->em_mad,msg->em_mad);
	gw_rm_copy_str_short(host->tm_mad,msg->tm_mad);
	gw_rm_copy_str_short(host->im_mad,msg->im_mad);

	msg->used_slots   = host->used_slots;
	msg->running_jobs = host->running_jobs;
	
	msg->host_id = host->host_id;
	msg->nice    = host->nice;
	
	gw_rm_copy_str_short(host->hostname,msg->hostname);
	gw_rm_copy_str_short(host->arch,msg->arch);
	gw_rm_copy_str_short(host->os_name,msg->os_name);
	gw_rm_copy_str_short(host->os_version,msg->os_version);	
	gw_rm_copy_str_short(host->cpu_model,msg->cpu_model);
	
	msg->cpu_mhz  = host->cpu_mhz;
	msg->cpu_free = host->cpu_free;
	msg->cpu_smp  = host->cpu_smp;
	msg->nodecount= host->nodecount;

    msg->size_mem_mb = host->size_mem_mb;
    msg->free_mem_mb = host->free_mem_mb;
    msg->size_disk_mb= host->size_disk_mb;
    msg->free_disk_mb= host->free_disk_mb;

	gw_rm_copy_str_short(host->fork_name,msg->fork_name);
	gw_rm_copy_str_short(host->lrms_name,msg->lrms_name);
	gw_rm_copy_str_short(host->lrms_type,msg->lrms_type);
	
	number_of_queues   = 0;
	number_of_int_vars = 0;
	number_of_str_vars = 0;
	
	for (i=0;i<GW_HOST_MAX_QUEUES;i++)
		if (host->queue_name[i]!=NULL)
		{
			msg->queue_nodecount[number_of_queues]      = 
				host->queue_nodecount[i];
			msg->queue_freenodecount[number_of_queues]  = 
				host->queue_freenodecount[i];
			msg->queue_maxtime[number_of_queues]        = 
				host->queue_maxtime[i];
			msg->queue_maxcputime[number_of_queues]     = 
				host->queue_maxcputime[i];
			msg->queue_maxcount[number_of_queues]       = 
				host->queue_maxcount[i];
			msg->queue_maxrunningjobs[number_of_queues] = 
				host->queue_maxrunningjobs[i];
			msg->queue_maxjobsinqueue[number_of_queues] = 
				host->queue_maxjobsinqueue[i];

			gw_rm_copy_str_short(host->queue_name[i],
				msg->queue_name[number_of_queues]);
			gw_rm_copy_str_short(host->queue_status[i],
				msg->queue_status[number_of_queues]);
			gw_rm_copy_str_short(host->queue_dispatchtype[i],
				msg->queue_dispatchtype[number_of_queues]);
			gw_rm_copy_str_short(host->queue_priority[i],
				msg->queue_priority[number_of_queues]);
			
			number_of_queues++;
		}
	msg->number_of_queues = number_of_queues;
	
	for (i=0;i<GW_HOST_MAX_GENVARS;i++)
		if (host->genvar_int[i].name != NULL )
		{
			gw_rm_copy_str_short(host->genvar_int[i].name
				,msg->gen_var_int_name[number_of_int_vars]);
			msg->gen_var_int_value[number_of_int_vars] = 
				host->genvar_int[i].value;
				
			number_of_int_vars++;
		}
	msg->number_of_int_vars = number_of_int_vars;	

	for (i=0;i<GW_HOST_MAX_GENVARS;i++)
		if (host->genvar_str[i].name != NULL )
		{
			gw_rm_copy_str_short(host->genvar_str[i].name,
				msg->gen_var_str_name[number_of_str_vars]);
			gw_rm_copy_str_short(host->genvar_str[i].value,
				msg->gen_var_str_value[number_of_str_vars]);
			number_of_str_vars++;
		}
	msg->number_of_str_vars = number_of_str_vars;
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

void gw_rm_host_status(int client_socket, int host_id)
{
    gw_host_t *    host;
    gw_msg_host_t  msg;
    int            rc;
    int            length;
    
    host         = gw_host_pool_get_host (host_id, GW_TRUE);
	length       = sizeof(gw_msg_host_t);
	msg.msg_type = GW_MSG_HOST_STATUS;
	  
	if ( host == NULL )
	{
		msg.rc = GW_RC_FAILED_BAD_HOST_ID;
		rc     = send(client_socket,(void *) &msg,length,0);
		return;
	}

	msg.rc = GW_RC_SUCCESS;
	
	gw_rm_host_to_msg (host, &msg);
			
	pthread_mutex_unlock(&(host->mutex));
	
	rc = send(client_socket,(void *) &msg,length,0);
	
	if ( rc == -1 )
		gw_log_print("RM",'E',"Error sending message %s\n",strerror(errno));
}
         
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

void gw_rm_host_pool_status(int client_socket)
{
    int           host_id;
    gw_host_t *   host;
    gw_msg_host_t msg;
    int           length;
	int           rc;
	      
	length = sizeof(gw_msg_host_t);
    
    for ( host_id=0; host_id < gw_conf.number_of_hosts ; host_id++)
    {
        host = gw_host_pool_get_host (host_id, GW_FALSE);
        
        if (host != NULL)
            gw_rm_host_status (client_socket, host_id);            
    }
    
    msg.msg_type = GW_MSG_END;
    msg.rc       = GW_RC_SUCCESS;
    
	rc = send(client_socket,(void *) &msg,length,0);
	
	if ( rc == -1 )
		gw_log_print("RM",'E',"Error sending message %s\n",strerror(errno));
}
