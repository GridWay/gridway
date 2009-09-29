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

gw_return_code_t gw_client_job_status(int job_id, gw_msg_job_t *job_status)
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

	msg.msg_type   = GW_MSG_JOB_STATUS;
	msg.job_id     = job_id;
	
	pthread_mutex_lock(&(gw_client.mutex));
	
	strncpy(msg.owner,gw_client.owner,GW_MSG_STRING_SHORT);	
	strncpy(msg.group,gw_client.group,GW_MSG_STRING_SHORT);
		
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
    
	length  = sizeof(gw_msg_job_t);
	
	rc = recv(fd,(void *) job_status, length, MSG_WAITALL);
	
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
	
    return job_status->rc;  
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

gw_return_code_t gw_client_job_status_all( )
{
    gw_msg_t       msg;
	gw_msg_job_t * job_msg;
	int            length;
	int            i;
	int            rc;
	int            fd;

	if ( gw_client.initialize == GW_FALSE )
		return GW_RC_FAILED_INIT;
	   
    /* ----------------------------------------------------------------- */
    /* 1.- Format message     	      	      	      	      	      	 */
    /* ----------------------------------------------------------------- */

    msg.msg_type = GW_MSG_JOB_POOL_STATUS;
    
    pthread_mutex_lock(&(gw_client.mutex));
    
	strncpy(msg.owner,gw_client.owner,GW_MSG_STRING_SHORT);
	strncpy(msg.group,gw_client.group,GW_MSG_STRING_SHORT);
	    
    pthread_mutex_unlock(&(gw_client.mutex));
	
	length       = sizeof(gw_msg_t);
	
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

	job_msg = (gw_msg_job_t *) malloc (sizeof(gw_msg_job_t));
	length  = sizeof(gw_msg_job_t);
	
	rc = recv(fd,(void *) job_msg, length, MSG_WAITALL);
	
    if ( rc == -1) 
    {
        perror("recv()");
        free(job_msg);
        
        gw_client_disconnect(fd);
        
        return GW_RC_FAILED_CONNECTION;
    }
    else if ( rc != length)
	{
		fprintf(stderr,"Error reading message\n");
        free(job_msg);
        
        gw_client_disconnect(fd);
        
		return GW_RC_FAILED_CONNECTION;
	}
	
	pthread_mutex_lock(&(gw_client.mutex));
	
    for (i = 0 ; i < gw_client.number_of_jobs; i++)
    	if ( gw_client.job_pool[i] != NULL )
    	{
       		free(gw_client.job_pool[i]);
       		gw_client.job_pool[i] = NULL;
    	}	

	pthread_mutex_unlock(&(gw_client.mutex));
	    	
	while ( job_msg->msg_type != GW_MSG_END )
	{
		if (job_msg->rc == GW_RC_SUCCESS) 
		{	
			pthread_mutex_lock(&(gw_client.mutex));
			
			if ( job_msg->id < gw_client.number_of_jobs)
			{
				gw_client.job_pool[job_msg->id] = job_msg;
				job_msg = (gw_msg_job_t *) malloc (sizeof(gw_msg_job_t));
			}
			
			pthread_mutex_unlock(&(gw_client.mutex));
		}
		
		rc  = recv(fd,(void *) job_msg, length, MSG_WAITALL);
    	
	    if ( rc == -1) 
    	{
        	perror("recv()");
        	free(job_msg);
        	
        	gw_client_disconnect(fd);
        	
        	return GW_RC_FAILED_CONNECTION;
    	}
    	else if ( rc != length)
		{
			fprintf(stderr,"Error reading message\n");
	        free(job_msg);
	        
	        gw_client_disconnect(fd);
	        	
			return GW_RC_FAILED_CONNECTION;
		}
	}

	free(job_msg);
	
	gw_client_disconnect(fd);
    
    return GW_RC_SUCCESS;  
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

gw_return_code_t gw_client_job_history(int                 job_id, 
                                       gw_msg_history_t ** history_list, 
                                       int *               num_records)
{
    gw_msg_t          msg;
	gw_return_code_t  gw_rc;	
	int               length;
	int               rc;
	int               fd;

	if ( gw_client.initialize == GW_FALSE )
	{
		*num_records  = 0;
		*history_list = NULL;
		
		return GW_RC_FAILED_INIT;
	}
	
    /* ----------------------------------------------------------------- */
    /* 1.- Format message                                                */
    /* ----------------------------------------------------------------- */

    msg.msg_type = GW_MSG_JOB_HISTORY;   
    msg.job_id   = job_id;
    
    pthread_mutex_lock(&(gw_client.mutex));
    
	strncpy(msg.owner,gw_client.owner,GW_MSG_STRING_SHORT);
	strncpy(msg.group,gw_client.group,GW_MSG_STRING_SHORT);
		
    pthread_mutex_unlock(&(gw_client.mutex));
   	
   	length       = sizeof(gw_msg_t);

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
    /* 3.- Receive response & update job history                	     */
    /* ----------------------------------------------------------------- */
    
    *num_records  = 0;
    *history_list = NULL;
   	length        = sizeof(gw_msg_history_t);
    *history_list = (gw_msg_history_t *) malloc(sizeof(gw_msg_history_t));
    
	rc = recv(fd,(void *) *history_list, length, MSG_WAITALL);
	
    if ( rc == -1) 
    {
        perror("recv()");
    	    
        free(*history_list);
        
       	*num_records  = 0;
       	*history_list = NULL;
        
        gw_client_disconnect(fd);
        
        return GW_RC_FAILED_CONNECTION;
    }
    else if ( rc != length)
	{
		fprintf(stderr,"Error reading message\n");
        
        free(*history_list);
        		
      	*num_records  = 0;
       	*history_list = NULL;
        
        gw_client_disconnect(fd);
        
		return GW_RC_FAILED_CONNECTION;
	} 
	else if ( (*history_list)[0].rc != GW_RC_SUCCESS )
	{
		gw_rc = (*history_list)[0].rc;
		
       	free(*history_list);
        	
       	*num_records  = 0;
       	*history_list = NULL;

        gw_client_disconnect(fd);
                	
        return gw_rc;			
	}
	
    while((*history_list)[(*num_records)].msg_type != GW_MSG_END )
    {            
    	*num_records  = *num_records +1;
        *history_list = realloc((*history_list),((*num_records)+1)*length);
                           
        if(*history_list==NULL)
        {
        	gw_client_disconnect(fd);
        	
        	return GW_RC_FAILED_NO_MEMORY;
        }

		rc = recv(fd,
		          (void *) &((*history_list)[(*num_records)]),
				  length, 
				  MSG_WAITALL);
	
	    if ( rc == -1) 
	    {
	        perror("recv()");

        	free(*history_list);
        	
        	*num_records  = 0;
        	*history_list = NULL;

	        gw_client_disconnect(fd);
	        
	        return GW_RC_FAILED_CONNECTION;
	    }
	    else if ( rc != length)
		{
			fprintf(stderr,"Error reading message\n");

        	free(*history_list);
        	
        	*num_records  = 0;
        	*history_list = NULL;
        	
        	gw_client_disconnect(fd);
        	
			return GW_RC_FAILED_CONNECTION;
		}
    }    
    
    if ( *num_records == 0 )
    {
    	gw_rc = GW_RC_SUCCESS;
    	
       	free(*history_list);

       	*history_list = NULL;    	
    }
    else
    	gw_rc = (*history_list)[(*num_records)].rc;
    
    gw_client_disconnect(fd);
    
    return gw_rc;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
