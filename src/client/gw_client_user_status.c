/* -------------------------------------------------------------------------- */
/* Copyright 2002-2010, GridWay Project Leads (GridWay.org)                   */
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
#include <string.h>
#include <unistd.h>

#include "gw_client.h"
#include "gw_rm_msg.h"
#include "gw_rm.h"

extern gw_client_t gw_client;

int  gw_client_connect();
void gw_client_disconnect(int socket);

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

gw_return_code_t gw_client_user_status(gw_msg_user_t **user_status, int *num_users)
{   
	int              length;
	int              rc;
	gw_msg_t         msg;
	gw_return_code_t gw_rc;
	int              fd;	
		
	if ( gw_client.initialize == GW_FALSE )
	{
		*num_users   = 0;
		*user_status = NULL;
		
		return GW_RC_FAILED_INIT;
	} 	
	
    /* ----------------------------------------------------------------- */
    /* 1.- Format msg     	      	      	      	      	      	     */
    /* ----------------------------------------------------------------- */

	msg.msg_type = GW_MSG_USERS;
	
	pthread_mutex_lock(&(gw_client.mutex));
	
	strncpy(msg.owner, gw_client.owner, GW_MSG_STRING_SHORT);	
	strncpy(msg.group, gw_client.group, GW_MSG_STRING_SHORT);
	strncpy(msg.proxy_path, gw_client.proxy_path, GW_MSG_STRING_SHORT);

	pthread_mutex_unlock(&(gw_client.mutex));
		
    length = sizeof(gw_msg_t);
    
    /* ----------------------------------------------------------------- */
    /* 2.- Send job status request    	      	      	      	      	 */
    /* ----------------------------------------------------------------- */

	fd = gw_client_connect();
	
	if (fd == -1)
	{
		return GW_RC_FAILED_CONNECTION;
	}
	
	rc = send(fd,(void *) &msg,length,0);

	if ( rc == -1 )
	{
		*num_users   = 0;
		*user_status = NULL;

		perror("send()");
		close(fd);
		
		return GW_RC_FAILED_CONNECTION;
	}
	else if ( rc != length )
	{
		*num_users   = 0;
		*user_status = NULL;
		
		fprintf(stderr,"Error sending message\n");
		close(fd);
		
		return GW_RC_FAILED_CONNECTION;
	}
   
    /* ----------------------------------------------------------------- */
    /* 3.- Receive response                                     	     */
    /* ----------------------------------------------------------------- */
    
    *num_users   = 0;
	length       = sizeof(gw_msg_user_t);
    *user_status = (gw_msg_user_t *) malloc(sizeof(gw_msg_user_t));
	
	rc = recv(fd,(void *) *user_status, length, MSG_WAITALL);
	
    if ( rc == -1) 
    {
    	free(*user_status);
    	
    	*num_users   = 0;
		*user_status = NULL;
    	
        perror("recv()");
        
        gw_client_disconnect(fd);
        
        return GW_RC_FAILED_CONNECTION;
    }
    else if ( rc != length)
	{
    	free(*user_status);
    			
		*num_users   = 0;
		*user_status = NULL;
		
		fprintf(stderr,"Error reading message\n");
		
        gw_client_disconnect(fd);
        		
		return GW_RC_FAILED_CONNECTION;
	}
	
    while((*user_status)[(*num_users)].msg_type != GW_MSG_END )
    {            
		*num_users   = *num_users + 1;
        *user_status = realloc((*user_status), ((*num_users)+1)*length);
                           
        if(*user_status==NULL)
        {
         	gw_client_disconnect(fd);
            return GW_RC_FAILED_NO_MEMORY;
        }

		rc = recv(fd,
		          (void *) &((*user_status)[(*num_users)]),
				  length, 
				  MSG_WAITALL);
	
	    if ( rc == -1) 
	    {
	        perror("recv()");
	        gw_client_disconnect(fd);
	        
	        return GW_RC_FAILED_CONNECTION;
	    }
	    else if ( rc != length)
		{
			fprintf(stderr,"Error reading message\n");
			gw_client_disconnect(fd);
			
			return GW_RC_FAILED_CONNECTION;
		}
    }    
    
    if ( *num_users  == 0 )
    {
       	gw_rc = (*user_status)[0].rc;
       	
      	free(*user_status);
       	*user_status = NULL;
    }
    else
	    gw_rc = (*user_status)[(*num_users)].rc;
	
	gw_client_disconnect(fd);
	    
    return gw_rc;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
