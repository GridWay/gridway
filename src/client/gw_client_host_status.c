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
#include <string.h>
#include <unistd.h>

#include "gw_client.h"
#include "gw_rm_msg.h"
#include "gw_rm.h"
#include "gw_file_parser.h"

extern gw_client_t gw_client;

int  gw_client_connect();
void gw_client_disconnect(int socket);

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

gw_return_code_t gw_client_host_status(int host_id, gw_msg_host_t *host_status)
{   
	int      length;
	int      rc;
	gw_msg_t msg;
	int      fd;
		
	if ( gw_client.initialize == GW_FALSE )
		return GW_RC_FAILED_INIT;
 	
    /* ----------------------------------------------------------------- */
    /* 1.- Format msg     	      	      	      	      	      	     */
    /* ----------------------------------------------------------------- */

	msg.msg_type   = GW_MSG_HOST_STATUS;
	msg.job_id     = host_id;
	
	pthread_mutex_lock(&(gw_client.mutex));
	
	strncpy(msg.owner,gw_client.owner,GW_MSG_STRING_LONG);	
	strncpy(msg.group,gw_client.group,GW_MSG_STRING_SHORT);
	
	pthread_mutex_unlock(&(gw_client.mutex));
		
    length = sizeof(gw_msg_t);
    
    /* ----------------------------------------------------------------- */
    /* 2.- Send host status request    	      	      	      	      	 */
    /* ----------------------------------------------------------------- */
    
	fd = gw_client_connect();
	
	if (fd == -1)
	{
		return GW_RC_FAILED_CONNECTION;
	}

	rc = send(fd,(void *) &msg,length,0);

	if ( rc == -1 )
	{
		perror("send()");
		close(fd);
		
		return GW_RC_FAILED_CONNECTION;
	}
	else if ( rc != length )
	{
		fprintf(stderr,"Error sending message\n");
		close(fd);
		
		return GW_RC_FAILED_CONNECTION;
	}
   
    /* ----------------------------------------------------------------- */
    /* 3.- Receive response                                     	     */
    /* ----------------------------------------------------------------- */
    
	length  = sizeof(gw_msg_host_t);
	
	rc = recv(fd,(void *) host_status, length, MSG_WAITALL);
	
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
	
	gw_client_disconnect(fd);
	
    return host_status->rc;  
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

gw_return_code_t gw_client_host_status_all( )
{
    gw_msg_t        msg;
	gw_msg_host_t * host_msg;
	int             length;
	int             i;
	int             rc;
	int             fd;
	
	if ( gw_client.initialize == GW_FALSE )
		return GW_RC_FAILED_INIT;
	   
    /* ----------------------------------------------------------------- */
    /* 1.- Format message     	      	      	      	      	      	 */
    /* ----------------------------------------------------------------- */

    msg.msg_type = GW_MSG_HOST_POOL_STATUS;
	length       = sizeof(gw_msg_t);
	
	pthread_mutex_lock(&(gw_client.mutex));
	
	strncpy(msg.owner,gw_client.owner,GW_MSG_STRING_LONG);	
	strncpy(msg.group,gw_client.group,GW_MSG_STRING_SHORT);
		
	pthread_mutex_unlock(&(gw_client.mutex));
	
    /* ----------------------------------------------------------------- */
    /* 2.- Send pool status request    	      	      	      	      	 */
    /* ----------------------------------------------------------------- */

	fd = gw_client_connect();
	
	if (fd == -1)
	{
		return GW_RC_FAILED_CONNECTION;
	}

	rc = send(fd,(void *) &msg,length,0);

	if ( rc == -1 )
	{
		perror("send()");
		close(fd);
		
		return GW_RC_FAILED_CONNECTION;
	}
	else if ( rc != length )
	{
		fprintf(stderr,"Error sending message\n");
		close(fd);
		
		return GW_RC_FAILED_CONNECTION;
	}

    /* ----------------------------------------------------------------- */
    /* 3.- Receive response & update client pool array           	     */
    /* ----------------------------------------------------------------- */

	host_msg = (gw_msg_host_t *) malloc (sizeof(gw_msg_host_t));
	length   = sizeof(gw_msg_host_t);
	
	rc = recv(fd,(void *) host_msg, length, MSG_WAITALL);
	
    if ( rc == -1) 
    {
        perror("recv()");
        free(host_msg);
        
        gw_client_disconnect(fd);
        
        return GW_RC_FAILED_CONNECTION;
    }
    else if ( rc != length)
	{
		fprintf(stderr,"Error reading message\n");
        free(host_msg);		
        
        gw_client_disconnect(fd);        
		
		return GW_RC_FAILED_CONNECTION;
	}
	
	pthread_mutex_lock(&(gw_client.mutex));
	
    for (i = 0 ; i < gw_client.number_of_hosts; i++)
    	if ( gw_client.host_pool[i] != NULL )
    	{
       		free(gw_client.host_pool[i]);
       		gw_client.host_pool[i] = NULL;
    	}	

	pthread_mutex_unlock(&(gw_client.mutex));
	    	
	while ( host_msg->msg_type != GW_MSG_END )
	{
		if (host_msg->rc == GW_RC_SUCCESS )
		{	
			pthread_mutex_lock(&(gw_client.mutex));
			
			if (host_msg->host_id < gw_client.number_of_hosts)
			{
				gw_client.host_pool[host_msg->host_id] = host_msg;
				host_msg = (gw_msg_host_t *) malloc (sizeof(gw_msg_host_t));
			}
			
			pthread_mutex_unlock(&(gw_client.mutex));
		}
		
		rc  = recv(fd,(void *) host_msg, length, MSG_WAITALL);
    	
	    if ( rc == -1) 
    	{
        	perror("recv()");
        	free(host_msg);
        	
        	gw_client_disconnect(fd);
        	
        	return GW_RC_FAILED_CONNECTION;
    	}
    	else if ( rc != length)
		{
			fprintf(stderr,"Error reading message\n");
	        free(host_msg);
	        
        	gw_client_disconnect(fd);	        	
			
			return GW_RC_FAILED_CONNECTION;
		}
	}

	free(host_msg);
	
   	gw_client_disconnect(fd);
    
    return GW_RC_SUCCESS;  
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
