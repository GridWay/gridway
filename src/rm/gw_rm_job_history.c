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
#include "gw_job_pool.h"

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

void gw_rm_history_to_msg (gw_history_t *history, gw_msg_history_t *msg)
{
	msg->msg_type = GW_MSG_JOB_HISTORY;
	msg->rc       = GW_RC_SUCCESS;

    msg->rank = history->rank;

    if (history->host != NULL)
    msg->host_id = history->host->host_id;
    
    msg->start_time = history->stats[START_TIME];
    msg->exit_time  = history->stats[EXIT_TIME];
    
    msg->prolog_stime = history->stats[PROLOG_START_TIME];
    msg->prolog_etime = history->stats[PROLOG_EXIT_TIME];

    msg->pre_wrapper_stime = history->stats[PRE_WRAPPER_START_TIME];
    msg->pre_wrapper_etime = history->stats[PRE_WRAPPER_EXIT_TIME];

    msg->wrapper_stime = history->stats[WRAPPER_START_TIME];
    msg->wrapper_etime = history->stats[WRAPPER_EXIT_TIME];

    msg->epilog_stime = history->stats[EPILOG_START_TIME];
    msg->epilog_etime = history->stats[EPILOG_EXIT_TIME];

    msg->migration_stime = history->stats[MIGRATION_START_TIME];
    msg->migration_etime = history->stats[MIGRATION_EXIT_TIME];;
 
 	if ( history->rdir != NULL )
 	{
	 	strncpy(msg->rdir, history->rdir, GW_MSG_STRING_SHORT);      
		if ( strlen(history->rdir) >= GW_MSG_STRING_SHORT )
			msg->rdir[GW_MSG_STRING_SHORT-1] = '\0';
 	}	
	else
		msg->rdir[0]='\0';

 	if ( history->em_rc != NULL )
 	{
	 	strncpy(msg->em_rc,history->em_rc,GW_MSG_STRING_SHORT);
		if ( strlen(history->em_rc) >= GW_MSG_STRING_SHORT )
			msg->em_rc[GW_MSG_STRING_SHORT-1] = '\0';
 	} 	
	else
		msg->em_rc[0]='\0';
		
 	if ( history->queue != NULL )
 	{
	   	strncpy(msg->queue,history->queue,GW_MSG_STRING_SHORT);
		if ( strlen(history->queue) >= GW_MSG_STRING_SHORT )
			msg->queue[GW_MSG_STRING_SHORT-1] = '\0';
 	} 	
	else
		msg->queue[0]='\0';
    	
    msg->tries  = history->tries;
    msg->reason = history->reason;
}


void gw_rm_job_history(int client_socket, int job_id)
{
    gw_job_t *       job;
    gw_msg_history_t msg;
    int              rc;
    int              length;
    gw_history_t *   tmp;
      
    job          = gw_job_pool_get(job_id, GW_TRUE);
	length       = sizeof(gw_msg_history_t);
	msg.msg_type = GW_MSG_JOB_HISTORY;
	  
	if ( job == NULL )
	{
		msg.rc = GW_RC_FAILED_BAD_JOB_ID;
		rc     = send(client_socket,(void *) &msg,length,0);
		return;
	}
	
	tmp = job->history;
	
	while (tmp != NULL )
	{
		gw_rm_history_to_msg (tmp, &msg);
		
		rc = send(client_socket,(void *) &msg,length,0);
		if ( rc == -1 )
			gw_log_print("RM",'E',"Error sending message %s\n",strerror(errno));
		
		tmp = tmp->next;
	}
			
	pthread_mutex_unlock(&(job->mutex));

    msg.msg_type = GW_MSG_END;
    msg.rc       = GW_RC_SUCCESS;
    
	rc = send(client_socket,(void *) &msg,length,0);
	if ( rc == -1 )
		gw_log_print("RM",'E',"Error sending message %s\n",strerror(errno));
}
             
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
