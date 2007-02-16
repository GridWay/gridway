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
#include "gw_job.h"

extern gw_client_t gw_client;

int  gw_client_connect();
void gw_client_disconnect(int socket);

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

gw_return_code_t gw_client_job_signal (int                job_id, 
                                       gw_client_signal_t signal, 
                                       gw_boolean_t       blocking)
{
    gw_msg_t msg;
 	int      rc;
 	int      length;
	int      async = 0;
	int      fd;
			 	
 	if ( gw_client.initialize == GW_FALSE )
		return GW_RC_FAILED_INIT;

    /* ----------------------------------------------------------------- */
    /* 1.- Format msg     	      	      	      	      	      	     */
    /* ----------------------------------------------------------------- */
    
    msg.job_id = job_id;
   	length     = sizeof(gw_msg_t);
   	
   	pthread_mutex_lock(&(gw_client.mutex));
   	
	strncpy(msg.owner,gw_client.owner,GW_MSG_STRING_SHORT);   	
	strncpy(msg.group,gw_client.group,GW_MSG_STRING_SHORT);
		
	pthread_mutex_unlock(&(gw_client.mutex));
	
    switch(signal)
    {
		case GW_CLIENT_SIGNAL_KILL:
			if (blocking == GW_TRUE)
				msg.msg_type = GW_MSG_KILL;
			else
			{
				msg.msg_type = GW_MSG_KILL_ASYNC;
				async        = 1;
			}
			break;
			
		case GW_CLIENT_SIGNAL_KILL_HARD:
			msg.msg_type = GW_MSG_KILL_HARD;		
			async = 1;
			break;
			
		case GW_CLIENT_SIGNAL_STOP:
			if (blocking == GW_TRUE)
				msg.msg_type = GW_MSG_STOP;
			else
			{
				msg.msg_type = GW_MSG_STOP_ASYNC;
				async        = 1;
			}
			break;
		case GW_CLIENT_SIGNAL_RESUME: msg.msg_type = GW_MSG_RESUME;
			break;		 
		case GW_CLIENT_SIGNAL_HOLD: msg.msg_type = GW_MSG_HOLD;
			break;		
		case GW_CLIENT_SIGNAL_RELEASE: msg.msg_type = GW_MSG_RELEASE;
			break;		
		case GW_CLIENT_SIGNAL_RESCHEDULE: msg.msg_type = GW_MSG_RESCHEDULE;
			break;
		default: 
			return GW_RC_FAILED;
    }
    
    /* ----------------------------------------------------------------- */
    /* 2.- Send signal to job    	      	      	      	      	     */
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

	if (async)
	{
		gw_client_disconnect(fd);
		
		return GW_RC_SUCCESS;
	}
		
    /* ----------------------------------------------------------------- */
    /* 3.- Receive response     	      	      	      	      	     */
    /* ----------------------------------------------------------------- */
	
	rc = recv(fd, (void *) &msg, length, MSG_WAITALL);
	
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

	return msg.rc;
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

gw_return_code_t gw_client_array_signal (int                array_id, 
                                         gw_client_signal_t signal, 
                                         gw_boolean_t       blocking)
{
	int i;
	int array_exists = 0;
	gw_return_code_t rc;
	gw_return_code_t frc;
	
	frc = GW_RC_SUCCESS;
	rc  = gw_client_job_status_all( );

   	if (rc == GW_RC_SUCCESS)
    {
    	pthread_mutex_lock(&(gw_client.mutex));
    	
    	/* First make a FAST kill */
		for (i=0;i<gw_client.number_of_jobs;i++)
			if (gw_client.job_pool[i] != NULL )
				if (gw_client.job_pool[i]->array_id  ==  array_id)
				{
					switch(gw_client.job_pool[i]->job_state)
					{
                        case GW_JOB_STATE_INIT:						
                        case GW_JOB_STATE_PENDING:
                        case GW_JOB_STATE_HOLD:
                        case GW_JOB_STATE_STOPPED:
                        case GW_JOB_STATE_ZOMBIE:
                        case GW_JOB_STATE_FAILED:
						
           					pthread_mutex_unlock(&(gw_client.mutex));
					
				         	rc = gw_client_job_signal (i, signal, blocking);
					
					        pthread_mutex_lock(&(gw_client.mutex));
					
         					if ( rc != GW_RC_SUCCESS)
		                        frc = GW_RC_FAILED;
		                
		                    array_exists = 1;
		                                       
                            break;
                            
                         default:
                            break;
     				  }
				}
				
    	/* kill the rest of the array */
		for (i=0;i<gw_client.number_of_jobs;i++)
			if (gw_client.job_pool[i] != NULL )
				if (gw_client.job_pool[i]->array_id == array_id)
				{
					switch(gw_client.job_pool[i]->job_state)
					{
                        case GW_JOB_STATE_PROLOG:
                        case GW_JOB_STATE_PRE_WRAPPER:
                        case GW_JOB_STATE_WRAPPER:
                        case GW_JOB_STATE_EPILOG:
                        case GW_JOB_STATE_EPILOG_STD:
                        case GW_JOB_STATE_EPILOG_RESTART:
                        case GW_JOB_STATE_EPILOG_FAIL:
                        case GW_JOB_STATE_STOP_CANCEL:
                        case GW_JOB_STATE_STOP_EPILOG:
                        case GW_JOB_STATE_KILL_CANCEL:
                        case GW_JOB_STATE_KILL_EPILOG:
                        case GW_JOB_STATE_MIGR_CANCEL:
                        case GW_JOB_STATE_MIGR_PROLOG:
                        case GW_JOB_STATE_MIGR_EPILOG:
						
           					pthread_mutex_unlock(&(gw_client.mutex));
					
				         	rc = gw_client_job_signal (i, signal, blocking);
					
					        pthread_mutex_lock(&(gw_client.mutex));
					
         					if ( rc != GW_RC_SUCCESS)
		                        frc = GW_RC_FAILED;
		                
		                    array_exists = 1;		                                                
                            
                            break;
                            
                         default:
                            break;
     				  }
				}
				
        pthread_mutex_unlock(&(gw_client.mutex));				
    }  
	
	if (array_exists == 0)
		frc = GW_RC_FAILED_BAD_ARRAY_ID;
		
	return frc;
}                                         

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
