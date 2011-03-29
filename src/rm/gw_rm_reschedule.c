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
#include "gw_job_pool.h"

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

void gw_rm_reschedule_success(void *_job_id)
{
	int                    job_id;
    gw_msg_t               msg;
    int                    length;
    gw_connection_list_t * connection;
    int                    rc;
    
	job_id = *( (int *) _job_id );
	length = sizeof(gw_msg_t);
	free(_job_id);
	
	msg.msg_type = GW_MSG_RESCHEDULE;
	msg.job_id   = job_id;
	msg.rc       = GW_RC_SUCCESS;
	
	connection = gw_connection_list_get(&(gw_rm.connection_list),
										GW_MSG_RESCHEDULE,
										job_id);
	if ( connection == NULL )
		gw_log_print("RM",'W',"Connection for job %i has been closed (RESCHEDULE_SUCCESS).\n",
		             job_id);	
	else
		while ( connection != NULL )
		{			
			rc = send(connection->socket_fs,
			          (void *) &msg,
			          length,
			          0);
			          
			if ( rc == -1 )
				gw_log_print("RM",'E',"Error sending message %s\n",strerror(errno));
				
			free (connection);
			
			connection = gw_connection_list_get(&(gw_rm.connection_list),
						 				        GW_MSG_RESCHEDULE, job_id);
		}	
}

/* ------------------------------------------------------------------------- */

void gw_rm_reschedule_failed (void *_job_id)
{
	int                    job_id;
    gw_msg_t               msg;
    int                    length;
    gw_job_t *             job;
    gw_connection_list_t * connection;
    int                    rc;
    
	job_id = *( (int *) _job_id );
	length = sizeof(gw_msg_t);	
	free(_job_id);

	job = gw_job_pool_get(job_id, GW_FALSE);

	if (job == NULL)
		msg.rc   = GW_RC_FAILED_BAD_JOB_ID;
	else
		msg.rc   = GW_RC_FAILED_BAD_JOB_STATE;
		
	msg.msg_type = GW_MSG_RESCHEDULE;
	msg.job_id   = job_id;
	
	connection = gw_connection_list_get(&(gw_rm.connection_list),
										GW_MSG_RESCHEDULE,
										job_id);
	if ( connection == NULL )
		gw_log_print("RM",'W',"Connection for job %i has been closed (RESCHEDULE_FAILED).\n",
		             job_id);
	else
		while ( connection != NULL )
		{			
			rc = send(connection->socket_fs, 
			          (void *) &msg, 
			          length, 
			          0);
			          
			if ( rc == -1 )
				gw_log_print("RM",'E',"Error sending message %s\n",strerror(errno));
							
			free (connection);
			
			connection = gw_connection_list_get(&(gw_rm.connection_list),
						 				        GW_MSG_RESCHEDULE, job_id);
		}	
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
