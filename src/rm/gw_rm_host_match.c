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
#include "gw_job_pool.h"
#include "gw_array_pool.h"
      
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

void gw_rm_job_match(int client_socket, int job_id)
{
    int            i,j,rc;
    int            number_of_queues;
    gw_host_t *    host;
    gw_job_t *     job;
    gw_msg_match_t msg;
    gw_boolean_t   match;
    int            length;
    int            slots, allowed_slots;
    
    msg.msg_type = GW_MSG_JOB_MATCH;
    length       = sizeof(gw_msg_match_t);
    
    job = gw_job_pool_get(job_id, GW_TRUE);
      
    if ( job == NULL )
    {
        msg.msg_type = GW_MSG_END_JOB;
        msg.rc       = GW_RC_FAILED_BAD_JOB_ID;
        msg.job_id   = job_id;
        rc           = send(client_socket,(void *) &msg,length,0);
        return;
    }
    
    for (i=0;i<gw_conf.number_of_hosts;i++)
    {
        host = gw_host_pool_get_host (i, GW_TRUE);
        
        if ( host != NULL )
        {
            if (host->lrms_name == NULL)
            {
                pthread_mutex_unlock(&(host->mutex));    
                continue;
            }

			number_of_queues   = 0;
			msg.rc             = GW_RC_SUCCESS;
			msg.matched        = GW_FALSE;
			msg.host_id        = i;
			msg.job_id         = job_id;
			msg.fixed_priority = host->fixed_priority;
			msg.running_jobs   = host->running_jobs;
			
			gw_rm_copy_str_host(host->hostname, msg.hostname);
              
            for (j=0;j<GW_HOST_MAX_QUEUES;j++)
            {
                if (host->queue_name[j]!=NULL)
                {
                    gw_rm_copy_str_short(host->queue_name[j],
                        msg.queue_name[number_of_queues]);
                        
                    match = gw_host_check_reqs(host, j, job->template.requirements);
                    
                    if (match == GW_TRUE)
                    {
                        msg.matched = GW_TRUE;
                        msg.match[number_of_queues] = 1;
                        msg.rank[number_of_queues] = gw_host_compute_rank(host,j,job->template.rank);

                        slots = gw_conf.sch_conf.max_resource*gw_conf.scheduling_interval/60 + 1;

                        if (slots > host->queue_nodecount[j])
                            slots = host->queue_nodecount[j];

                        if (host->queue_maxjobsinqueue[j] > 0) {
                            allowed_slots = host->queue_maxjobsinqueue[j] - host->used_slots;
                            if (slots > allowed_slots)
		                slots = allowed_slots;
                        }

                        msg.slots[number_of_queues] = slots;
                    }
                    else
                    {
                        msg.match[number_of_queues] = 0;
                        msg.rank[number_of_queues] = 0;    
                        msg.slots[number_of_queues] = 0;                        
                    }
                    
                    number_of_queues++;
                }
            }
            
            msg.number_of_queues = number_of_queues;
            
            pthread_mutex_unlock(&(host->mutex));    
            
            if (msg.matched == GW_TRUE)
            {
            	rc = send(client_socket,(void *) &msg,length,0);
    
            	if ( rc == -1 )
                	gw_log_print("RM",'E',"Error sending message %s\n",strerror(errno));
            }
        }        
    }
    
    pthread_mutex_unlock(&(job->mutex));
    
    msg.msg_type = GW_MSG_END_JOB;
    msg.rc       = GW_RC_SUCCESS;
    
    rc = send(client_socket,(void *) &msg,length,0);
    
    if ( rc == -1 )
        gw_log_print("RM",'E',"Error sending message %s\n",strerror(errno));        
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

void gw_rm_array_match(int client_socket, int array_id)
{
    gw_array_t *   array;
    gw_msg_match_t msg;
    int            rc,i,length;

    msg.msg_type = GW_MSG_JOB_MATCH;
    length       = sizeof(gw_msg_match_t);
        
    array = gw_array_pool_get_array (array_id, GW_TRUE);
    
    if ( array ==  NULL )
    {
        msg.msg_type = GW_MSG_END_JOB;
        msg.rc       = GW_RC_FAILED_BAD_JOB_ID;
        msg.job_id   = array_id;
        rc           = send(client_socket,(void *) &msg,length,0);
        return;
    }
    
    /* Find a job from this array */
    
    for (i=0;i < array->number_of_tasks; i++)
	{
		if (array->job_ids[i] != -1)
		{
    	    gw_rm_job_match(client_socket, array->job_ids[i]);
		    break;	
		}
	}
	
	pthread_mutex_unlock(&(array->mutex));
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
