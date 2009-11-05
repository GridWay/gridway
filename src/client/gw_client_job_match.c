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

gw_return_code_t gw_client_match_job(int                 job_id,
                                     int                 array_id,
                                     gw_msg_match_t **   match_list, 
                                     int *               num_records)
{   
	int              length;
	int              rc;
	gw_msg_t         msg;
	gw_return_code_t gwrc;
	int              fd;
	
	if ( gw_client.initialize == GW_FALSE )
		return GW_RC_FAILED_INIT;
 		
    /* ----------------------------------------------------------------- */
    /* 1.- Format msg     	      	      	      	      	      	     */
    /* ----------------------------------------------------------------- */

	msg.msg_type = GW_MSG_JOB_MATCH;
	msg.job_id   = job_id;
	msg.array_id = array_id;

	pthread_mutex_lock(&(gw_client.mutex));
	
	strncpy(msg.owner, gw_client.owner, GW_MSG_STRING_SHORT);
	strncpy(msg.group, gw_client.group, GW_MSG_STRING_SHORT);
	strncpy(msg.proxy_path, gw_client.proxy_path, GW_MSG_STRING_SHORT);

	pthread_mutex_unlock(&(gw_client.mutex));
	
    length = sizeof(gw_msg_t);
    
    /* ----------------------------------------------------------------- */
    /* 2.- Send job match request    	      	      	      	      	 */
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
        *match_list  = NULL;
        *num_records = 0;
		
		close(fd);
		
		return GW_RC_FAILED_CONNECTION;
	}
	else if ( rc != length )
	{
		fprintf(stderr,"Error sending message\n");
        *match_list  = NULL;
        *num_records = 0;
		
		close(fd);
		
        return GW_RC_FAILED_CONNECTION;
	}
   
    /* ----------------------------------------------------------------- */
    /* 3.- Receive response                                     	     */
    /* ----------------------------------------------------------------- */
    
	length       = sizeof(gw_msg_match_t);
	*match_list  = (gw_msg_match_t *) malloc(length);
	*num_records = 0;
	
	rc = recv(fd,(void *) *match_list, length, MSG_WAITALL);
	
    if ( rc == -1) 
    {
        perror("recv()");
        
        free(*match_list);
        
        *match_list  = NULL;
        *num_records = 0;
		
		gw_client_disconnect(fd);
		
        return GW_RC_FAILED_CONNECTION;
    }
    else if ( rc != length)
	{
		fprintf(stderr,"Error reading message\n");
		
        free(*match_list);

        *match_list  = NULL;
        *num_records = 0;

		gw_client_disconnect(fd);
		
		return GW_RC_FAILED_CONNECTION;
	}
	else if ((*match_list)[0].rc != GW_RC_SUCCESS )
	{
		gwrc = (*match_list)[0].rc;
		
        free(*match_list);

        *match_list  = NULL;
        *num_records = 0;

		gw_client_disconnect(fd);
		
		return gwrc;
	}
				
    while( (*match_list)[*num_records].msg_type != GW_MSG_END_JOB )
    {            
       	*num_records = *num_records + 1;	
       	*match_list  = realloc((*match_list),length*(*num_records+1));
        	
       	if(*match_list==NULL)
       	{
       		gw_client_disconnect(fd);
       		
	        return GW_RC_FAILED_NO_MEMORY;
       	}
		
		rc = recv(fd,
		          (void *) &((*match_list)[(*num_records)]),
		          length, 
		          MSG_WAITALL);
	
	    if ( rc == -1) 
	    {
	        perror("recv()");
		
	        free(*match_list);

    	    *match_list  = NULL;
        	*num_records = 0;
        	
	        gw_client_disconnect(fd);
	        
	        return GW_RC_FAILED_CONNECTION;
	    }
	    else if ( rc != length)
		{
			fprintf(stderr,"Error reading message\n");

	        free(*match_list);

    	    *match_list  = NULL;
        	*num_records = 0;
        	
			gw_client_disconnect(fd);
			
			return GW_RC_FAILED_CONNECTION;
		}
    }    

    if ( *num_records  == 0 )
    {
       	gwrc = GW_RC_SUCCESS;
       	
      	free(*match_list);
      	
       	*match_list = NULL;
    }
    else
	    gwrc = (*match_list)[(*num_records)].rc;
	
	gw_client_disconnect(fd);
	
    return gwrc;	
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
