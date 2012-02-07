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

void gw_rm_submit(void *_msg)
{
    gw_msg_t *             msg;
    int                    length;
    int                    rc;
    
	msg    = (gw_msg_t *) _msg;
	length = sizeof(gw_msg_t);

	rc = send(msg->client_socket,
	          (void *) msg, 
	          length, 
	          0);

	if ( rc == -1 )
		gw_log_print("RM",'E',"Error sending message %s\n",strerror(errno));
					
	free (_msg);
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
