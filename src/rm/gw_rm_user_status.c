/* -------------------------------------------------------------------------- */
/* Copyright 2002-2013, GridWay Project Leads (GridWay.org)                   */
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
#include "gw_user_pool.h"

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

void gw_rm_user_pool(int client_socket)
{
    gw_msg_user_t msg;
    int           length;
    int           rc;
    int           i;
	      
    length = sizeof(gw_msg_user_t);
	
    for (i=0; i<gw_conf.number_of_users; i++)
    {
    	if ( gw_user_pool_get_info(i, &msg) == GW_TRUE )
    	{
       	    msg.msg_type = GW_MSG_USERS;
            msg.rc       = GW_RC_SUCCESS;
    		          		      
            rc = send(client_socket,(void *) &msg,length,0);
	
            if ( rc == -1 )
                gw_log_print("RM",'E',"Error sending message %s\n",strerror(errno));		    		
        }
    }
	
    msg.msg_type = GW_MSG_END;
    msg.rc       = GW_RC_SUCCESS;
    
    rc = send(client_socket,(void *) &msg,length,0);
	
    if ( rc == -1 )
        gw_log_print("RM",'E',"Error sending message %s\n",strerror(errno));
}
