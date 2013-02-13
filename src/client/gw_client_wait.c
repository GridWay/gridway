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
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <time.h>

#include "gw_client.h"
#include "gw_rm_msg.h"
#include "gw_rm.h"
#include "gw_file_parser.h"

extern gw_client_t gw_client;

int  gw_client_connect();
void gw_client_disconnect(int socket);

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

gw_return_code_t gw_client_wait(int job_id, int *exit_code, signed long timeout)
{
    gw_msg_t       msg;
 	int            rc;
 	int            length;
 	struct timeval tout;
 	fd_set         socket;
	gw_msg_job_t   job_status;
	int            fd;
	 	
 	if ( gw_client.initialize == GW_FALSE )
		return GW_RC_FAILED_INIT;

	if ( timeout == 0 )
	{
		rc = gw_client_job_status(job_id, &job_status);
		
		if ( rc != GW_RC_SUCCESS )
			return rc;
			
		if ((job_status.job_state == GW_JOB_STATE_ZOMBIE) ||
		    (job_status.job_state == GW_JOB_STATE_FAILED))
		{
			*exit_code = job_status.exit_code;
			
			return GW_RC_SUCCESS;
		}
		else
			return GW_RC_FAILED_TIMEOUT;	
	}
	
    /* ----------------------------------------------------------------- */
    /* 1.- Format msg     	      	      	      	      	      	     */
    /* ----------------------------------------------------------------- */
    
  	msg.msg_type  = GW_MSG_WAIT;
  	msg.wait_type = GW_MSG_WAIT_JOB;
  	msg.job_id    = job_id;
	length        = sizeof(gw_msg_t);
	  	
  	pthread_mutex_lock(&(gw_client.mutex));
  	
	strncpy(msg.owner, gw_client.owner, GW_MSG_STRING_SHORT);  	
	strncpy(msg.group, gw_client.group, GW_MSG_STRING_SHORT);
	strncpy(msg.proxy_path, gw_client.proxy_path, GW_MSG_STRING_LONG);
	
  	pthread_mutex_unlock(&(gw_client.mutex));	
  	
    /* ----------------------------------------------------------------- */
    /* 2.- Send submit request    	      	      	      	      	     */
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
    /* 3.- Receive response     	      	      	      	      	     */
    /* ----------------------------------------------------------------- */
	
	if ( timeout > -1 )
	{
		tout.tv_sec = timeout;
		tout.tv_usec= 0;		
		
		FD_ZERO(&socket);
		FD_SET(fd,&socket);
		
		rc = select(fd+1,&socket,0,0,&tout);
		
		if ( rc == 0 )
		{
			gw_client_disconnect(fd);
			return GW_RC_FAILED_TIMEOUT;	
		}
		
	}
	
	rc = recv(fd, (void *) &msg,length,MSG_WAITALL);
	
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

	if (msg.rc == GW_RC_SUCCESS)
		*exit_code = msg.exit_code;
	
	gw_client_disconnect(fd);
	
	return msg.rc;
}

/* ---------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------- */

static gw_return_code_t gw_client_test_job(int job_id, int *exit_code, 
                                           gw_boolean_t *finished)
{
	gw_return_code_t rc;
	gw_msg_job_t     job_status;
	
	rc = gw_client_job_status(job_id, &job_status);
		
	if ( rc != GW_RC_SUCCESS )
		return rc;
			
	if ((job_status.job_state == GW_JOB_STATE_ZOMBIE) ||
	    (job_status.job_state == GW_JOB_STATE_FAILED))
	{
		*exit_code = job_status.exit_code;
		*finished  = GW_TRUE;
	}
	else
		*finished  = GW_FALSE;
	
	return GW_RC_SUCCESS;
}

/* ---------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------- */

gw_return_code_t gw_client_wait_set(int *        job_ids, 
                                    int **       exit_codes, 
                                    gw_boolean_t any,
                                    signed long  timeout)
{
    gw_msg_t msg;
 	int      rc;
 	int      length;
 	int      i,j,k;
 	int      fd;
 	
 	gw_return_code_t gwrc;
 	struct timeval   tout;
 	fd_set           socket;
	time_t           the_time, the_time2;
	signed long      tv_waited;
	gw_boolean_t     finished;
		 	
 	if ( gw_client.initialize == GW_FALSE )
		return GW_RC_FAILED_INIT;

	if ( timeout == 0 )
	{
		i = 0;
			
		if ( any == GW_TRUE )
		{
			*exit_codes = (int *) malloc(sizeof(int));
			
			while ( job_ids[i] != -1 )
			{
				
				gwrc = gw_client_test_job(job_ids[i], &((*exit_codes)[0]), &finished);
				
				if ( gwrc != GW_RC_SUCCESS )
				{
					return gwrc;
				}
				else if (finished == GW_TRUE)
				{
					job_ids[0]   = job_ids[i];
					return GW_RC_SUCCESS;
				}

				i++;
			}
			
			return GW_RC_FAILED_TIMEOUT;			
		}
		else
		{
			while ( job_ids[i] != -1 )
				i++;
				
			*exit_codes = (int *) malloc(sizeof(int)*i);
			
			for ( j=0;j<i;j++)
			{				
				gwrc = gw_client_test_job(job_ids[j],&((*exit_codes)[j]),&finished);
				
				if ( gwrc != GW_RC_SUCCESS )
				{
					return gwrc;
				}
				else if (finished == GW_FALSE)
				{
					return GW_RC_FAILED_TIMEOUT;
				} 				
			}
			
			return GW_RC_SUCCESS;								
		}
		
	}
	
    /* ----------------------------------------------------------------- */
    /* 1.- Format msg     	      	      	      	      	      	     */
    /* ----------------------------------------------------------------- */
    
  	msg.msg_type  = GW_MSG_WAIT;  	
	length = sizeof(gw_msg_t);
	
	pthread_mutex_lock(&(gw_client.mutex));
	
	strncpy(msg.owner, gw_client.owner, GW_MSG_STRING_SHORT);	
	strncpy(msg.group, gw_client.group, GW_MSG_STRING_SHORT);
        strncpy(msg.proxy_path, gw_client.proxy_path, GW_MSG_STRING_LONG);

	pthread_mutex_unlock(&(gw_client.mutex));
	
    /* ----------------------------------------------------------------- */
    /* 2.- Send submit request    	      	      	      	      	     */
    /* ----------------------------------------------------------------- */

	fd = gw_client_connect();
	
	if (fd == -1)
	{
		return GW_RC_FAILED_CONNECTION;
	}

	i = 0;
	while ( job_ids[i] != -1 )
	{
		msg.job_id = job_ids[i];
		
		if (any == GW_TRUE)
		{
			if (i == 0)
				msg.wait_type = GW_MSG_WAIT_ANY_FIRST;
			else
				msg.wait_type = GW_MSG_WAIT_ANY;
		}
		else
			msg.wait_type = GW_MSG_WAIT_JOB;		

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
		
		i++;	
	}
	
	if ( i == 0 )
	{	
		gw_client_disconnect(fd);
		return GW_RC_SUCCESS;
	}
		
	if (any == GW_TRUE)
	{
		*exit_codes = (int *) malloc(sizeof(int));
				
		if ( timeout > -1 )
		{
			tout.tv_sec = timeout;
			tout.tv_usec=0;
					
			FD_ZERO(&socket);
			FD_SET(fd,&socket);
		
			rc = select(fd+1,&socket,0,0,&tout);
		
			if ( rc == 0 )
			{
				gw_client_disconnect(fd);
				return GW_RC_FAILED_TIMEOUT;	
			}		
		}
		
		rc = recv(fd,(void *) &msg,length,MSG_WAITALL);
	
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

		if (msg.rc == GW_RC_SUCCESS)
		{
			**exit_codes = msg.exit_code;
			job_ids[0]   = msg.job_id;
		}
		
		gw_client_disconnect(fd);
		return msg.rc;
	}
	
	*exit_codes = (int *) malloc(sizeof(int)*i);
	gwrc        = GW_RC_SUCCESS;
	
	tv_waited = 0;
	
	for (j=0;j<i;j++)
	{
		if ( timeout > -1 )
		{
			tout.tv_sec = timeout - tv_waited;
			tout.tv_usec=0;
			
			FD_ZERO(&socket);
			FD_SET(fd,&socket);
			
			time(&the_time);
			
			rc = select(fd+1,&socket,0,0,&tout);
			
			time(&the_time2);
			
			tv_waited = tv_waited + (the_time2 - the_time);
			
			if ( rc == 0 )
			{
				gw_client_disconnect(fd);
				
				return GW_RC_FAILED_TIMEOUT;	
			}
		
		}
		
		rc = recv(fd,(void *) &msg,length,MSG_WAITALL);

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

		if (msg.rc == GW_RC_SUCCESS)
		{
			for (k=0;k<i;k++)
				if ( job_ids[k] == msg.job_id )
				{
					(*exit_codes)[k] = msg.exit_code;
					break;
				}
		}
		else
			gwrc = msg.rc;
	}
	
	gw_client_disconnect(fd);
	
	return gwrc;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
