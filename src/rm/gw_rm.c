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
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <fcntl.h>

#include "gw_rm.h"
#include "gw_rm_msg.h"
#include "gw_log.h"
#include "gw_job_pool.h"
#include "gw_user_pool.h"

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- */
gw_rm_t gw_rm;
/* ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

gw_rm_t* gw_rm_init()
{    
	int    yes = 1;
	int    rc;
	FILE * fd;
    char   str_buffer[2048];
    char   hostname[256];
    
    /* ----------------------------------------------------------------- */
    /* 1.- Socket Initialization                                         */
    /* ----------------------------------------------------------------- */
    
    
    rc = gethostname(hostname, 255);
    if (rc == -1)
    {
		perror("[RM]: gethostname()");
		return NULL;
	}
		   
    gw_rm.socket = socket(AF_INET, SOCK_STREAM, 0);
    if ( gw_rm.socket == -1) 
    {
		perror("[RM]: socket()");
		return NULL;
	}
	
	fcntl(gw_rm.socket,F_SETFD,FD_CLOEXEC); /* Close socket in MADs */
	
	rc = setsockopt(gw_rm.socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));	
	if ( rc == -1) 
	{
		perror("[RM]: setsockopt()");
		return NULL;
	}

	gw_rm.rm_addr.sin_family      = AF_INET;
	gw_rm.rm_addr.sin_port        = htons(gw_conf.gwd_port);
	gw_rm.rm_addr.sin_addr.s_addr = INADDR_ANY;

	/* ----- Loop until we find a free port ----- */
	do 
	{
		rc = bind(gw_rm.socket,(struct sockaddr *) &(gw_rm.rm_addr),sizeof(struct sockaddr));
		if ( rc == -1) 
		{
			gw_conf.gwd_port = gw_conf.gwd_port + 1;
			gw_rm.rm_addr.sin_port = htons(gw_conf.gwd_port);
		}
	} while ( rc == -1 );

	/* ----- Write the port we found in $GW_LOCATION/var/gw.port to let clients know ----- */	
    rc = snprintf(str_buffer,sizeof(char)*2048,"%s/var/gwd.port",gw_conf.gw_location);    
    if ( rc >= (sizeof(char) * 2048) )
    {
    	fprintf(stderr,"[RM]: Error creating port filename (%s)\n",str_buffer);
        return NULL;    	
    }
    
	fd = fopen(str_buffer,"w");
	if ( fd == NULL )
	{
		perror("[RM]: fopen()");
		return NULL;
	}
	
	fprintf(fd,"%s %i\n",hostname ,gw_conf.gwd_port);
	fclose(fd);



	/* ----- Listen on that port ---- */
	rc = listen(gw_rm.socket, 10);	
	if ( rc == -1) {
		perror("[RM]: listen()");
		exit(1);
	}
	    
    /* ----------------------------------------------------------------- */
    /* 2.- Register Request Manager Events                               */   
    /* ----------------------------------------------------------------- */
    
    gw_am_init(&(gw_rm.am));
    
    gw_am_register(GW_ACTION_FINALIZE, 
                   GW_ACTION_SEQUENTIAL, 
                   gw_rm_finalize,
                   &(gw_rm.am));
                           
    gw_am_register("GW_RM_CONNECTION", 
                   GW_ACTION_THREADED, 
                   gw_rm_connection, 
                   &(gw_rm.am));

    gw_am_register("GW_RM_SUBMIT", 
                   GW_ACTION_THREADED, 
                   gw_rm_submit, 
                   &(gw_rm.am));
    
    gw_am_register("GW_RM_HOLD_SUCCESS", 
                   GW_ACTION_SEQUENTIAL, 
                   gw_rm_hold_success, 
                   &(gw_rm.am));
    
    gw_am_register("GW_RM_HOLD_FAILED", 
                   GW_ACTION_SEQUENTIAL, 
                   gw_rm_hold_failed, 
                   &(gw_rm.am));
    
    gw_am_register("GW_RM_RELEASE_SUCCESS", 
                   GW_ACTION_SEQUENTIAL, 
                   gw_rm_release_success, 
                   &(gw_rm.am));
    
    gw_am_register("GW_RM_RELEASE_FAILED", 
                   GW_ACTION_SEQUENTIAL, 
                   gw_rm_release_failed, 
                   &(gw_rm.am));
    
    gw_am_register("GW_RM_KILL_SUCCESS", 
                   GW_ACTION_SEQUENTIAL, 
                   gw_rm_kill_success, 
                   &(gw_rm.am));
    
    gw_am_register("GW_RM_KILL_FAILED", 
                   GW_ACTION_SEQUENTIAL, 
                   gw_rm_kill_failed, 
                   &(gw_rm.am));

    gw_am_register("GW_RM_RESCHEDULE_SUCCESS", 
                   GW_ACTION_SEQUENTIAL, 
                   gw_rm_reschedule_success, 
                   &(gw_rm.am));
    
    gw_am_register("GW_RM_RESCHEDULE_FAILED", 
                   GW_ACTION_SEQUENTIAL, 
                   gw_rm_reschedule_failed, 
                   &(gw_rm.am));
    
    gw_am_register("GW_RM_STOP_SUCCESS", 
                   GW_ACTION_SEQUENTIAL, 
                   gw_rm_stop_success, 
                   &(gw_rm.am));
    
    gw_am_register("GW_RM_STOP_FAILED", 
                   GW_ACTION_SEQUENTIAL, 
                   gw_rm_stop_failed, 
                   &(gw_rm.am));
    
    gw_am_register("GW_RM_RESUME_SUCCESS", 
                   GW_ACTION_SEQUENTIAL, 
                   gw_rm_resume_success, 
                   &(gw_rm.am));

    gw_am_register("GW_RM_RESUME_FAILED", 
                   GW_ACTION_SEQUENTIAL, 
                   gw_rm_resume_failed, 
                   &(gw_rm.am));
    
    gw_am_register("GW_RM_WAIT_SUCCESS", 
                   GW_ACTION_SEQUENTIAL, 
                   gw_rm_wait_success, 
                   &(gw_rm.am));
    
    gw_am_register("GW_RM_WAIT_FAILED", 
                   GW_ACTION_SEQUENTIAL, 
                   gw_rm_wait_failed, 
                   &(gw_rm.am));
        
    return &(gw_rm);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void gw_rm_finalize(void *_null)
{
    /* ----------------------------------------------------------------- */
    /* 1.- gw_mt_free msg Queues Initialization                          */
    /* ----------------------------------------------------------------- */
   
	close(gw_rm.socket);
    
    gw_connection_list_destroy(&(gw_rm.connection_list));
    
    /* ----------------------------------------------------------------- */
    /* 2.- gw_mt_free the Event Handler                                  */
    /* ----------------------------------------------------------------- */
    
    gw_am_destroy(&(gw_rm.am));

    /* ----------------------------------------------------------------- */
    /* 3.- Terminate the Listening Thread                                */
    /* ----------------------------------------------------------------- */

    pthread_cancel(gw_rm.listener_thread);
    
    gw_log_print("RM",'I',"Request Manager finalized.\n");  
    
    pthread_exit(0);  
      
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
 
void gw_rm_start (void *_null)
{ 
    pthread_attr_t attr;
    sigset_t       sig_group;
            
    sigfillset(&sig_group);
         
    pthread_sigmask(SIG_BLOCK,&sig_group,NULL);

    pthread_attr_init (&attr);
    
    pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);
        
    pthread_create(&(gw_rm.listener_thread), &attr, (void *)gw_rm_listener,
                NULL);
    
    gw_log_print("RM",'I',"Request Manager started.\n");

    gw_am_loop(&(gw_rm.am),0,NULL);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

static void gw_rm_set_fd_set(fd_set * clients, int *active_connections)
{
	int i;
	
	FD_ZERO(clients);
    	
   	for (i=0;i<gw_conf.max_number_of_clients;i++)
		if ( active_connections[i] != -1)
			FD_SET (active_connections[i], clients);	
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

static int gw_rm_add_fd_set(int *active_connections, int socket, int *max_fd)
{
	int j;
	int close_connection;
	
    close_connection = 1;
    				
	for (j=1;j<gw_conf.max_number_of_clients;j++)
		if (active_connections[j] == -1)
		{
			active_connections[j] = socket;

			if (socket > *max_fd)
				*max_fd = socket;
		    					
			close_connection = 0;
			break;
		}
	
	return close_connection;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

static void gw_rm_del_fd_set(int *active_connections, int socket, int *max_fd)
{
	int j;
							
	for (j=1;j<gw_conf.max_number_of_clients;j++)
		if (active_connections[j] == socket)
		{
				active_connections[j] = -1;
				close(socket);
				break;
		}
						
	*max_fd = 0;

	for (j=0;j<gw_conf.max_number_of_clients;j++)
		if (active_connections[j] > *max_fd)
			*max_fd = active_connections[j];
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void gw_rm_listener(void *_null)
{
    int                    client_socket;    
	struct sockaddr_in     client_addr;
	socklen_t              client_size;
	gw_msg_t               * msg;
	fd_set                 clients;
	int                    *active_connections;
	int                    i, rc;
	int                    max_fd;
    ssize_t 			   bytes;
    size_t				   length;
    int                    close_connection;
	
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL); 
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
  
    /* ------------------ msg Loop ------------------------*/
    /* 1.- Wait until a new msg arrives                    */
    /* 2.- Forward the msg to the msg handler              */
    /* ----------------------------------------------------*/

	active_connections    = (int *) malloc (sizeof(int) * gw_conf.max_number_of_clients);
	active_connections[0] = gw_rm.socket;
	
	for (i=1;i<gw_conf.max_number_of_clients;i++)
		active_connections[i] = -1;
		
	max_fd = active_connections[0];
	
    while(1)
    {
    	gw_rm_set_fd_set(&clients, active_connections);
    	
    	rc = select(max_fd + 1, &clients, NULL, NULL, NULL);

    	if (rc == -1)
    	{
    		gw_log_print("RM",'E',"Error in select(): %s",strerror(errno));
			continue;
    	}
    	
    	for ( i = 0; i < max_fd + 1 ; i++)
    	{
    		if (FD_ISSET(i, &clients))
    		{
    			if ( i == active_connections[0]) /* New connection, update FD_SET */
    			{
			    	client_size   = sizeof(struct sockaddr_in);
			    	client_socket = accept(i, 
    	    		            	       (struct sockaddr *) &client_addr,
                    			           & client_size);
                               
			        if (client_socket == -1)
			        {
			            gw_log_print("RM",'E',"Error accepting client connection %s\n",strerror(errno));
			            continue;
			        }
    				
					close_connection = gw_rm_add_fd_set(active_connections, client_socket, &max_fd);
					
					if (close_connection)
					{
						gw_log_print("RM",'W',"Maximum number of clients reached, try later.\n");
						close(client_socket);
					}				
    			}
    			else /* Active connection, read and forward event */
    			{
					msg    = (gw_msg_t *) malloc(sizeof(gw_msg_t));    
				    length = sizeof(gw_msg_t);
    
				    bytes  = recv(i, (void *) msg,  length, MSG_WAITALL);
				    
				    if ( bytes == length )
				    	close_connection = msg->msg_type == GW_MSG_DISENGAGE;
                
				    if ( (bytes == -1) || (bytes != length) || close_connection)  /* close connection*/
				    {
						gw_rm_del_fd_set(active_connections, i, &max_fd);
				        free(msg);
				        
				        gw_connection_list_delete(&(gw_rm.connection_list),i);
				    }
    				else
    				{
    					msg->client_socket = i;
    				    gw_am_trigger(&(gw_rm.am),"GW_RM_CONNECTION", (void *) msg);
    				}
    			}
    		}
    	}
    }
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

static gw_boolean_t gw_rm_auth_user(int job_id, const char * owner)
{
	gw_job_t *   job;
	int          user_id;
	gw_boolean_t user_exists;
	
	if (strncmp(owner, gw_conf.gwadmin,GW_MSG_STRING_SHORT) == 0)
		return GW_TRUE;
	
	user_exists	= gw_user_pool_exists (owner, &user_id);
	
	if ( user_exists)
	{
		job =  gw_job_pool_get(job_id, GW_FALSE);
		
		if ( job == NULL )
			return GW_TRUE;
			
		return (job->user_id == user_id);
	}
	else
	{
		return GW_FALSE;	
	}			
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

static void gw_rm_permission_denied(gw_msg_t *msg)
{
    int      length;
    int      rc;
    
	length  = sizeof(gw_msg_t);
	msg->rc = GW_RC_FAILED_PERM;
	
	rc = send(msg->client_socket, 
	          (void *) msg, 
	          length, 
	          0);

	if ( rc == -1 )
		gw_log_print("RM",'E',"Error sending message %s\n",strerror(errno));
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void gw_rm_connection(void *_msg)
{
    int *        job_id;
	gw_msg_t *   msg;
	gw_boolean_t in_list;
	gw_boolean_t auth;	
	    
    msg = (gw_msg_t *) _msg;

    switch(msg->msg_type)
    {
		case GW_MSG_SUBMIT:
		
			gw_am_trigger(gw_rm.dm_am, "GW_DM_ALLOCATE_JOB", (void *) msg);
	  		break;
	  		
	  	case GW_MSG_SUBMIT_ARRAY:
	  
			gw_am_trigger(gw_rm.dm_am, "GW_DM_ALLOCATE_ARRAY", (void *) msg);	  
		  	break;		  	
		  	
	  	case GW_MSG_WAIT:

		  	auth = gw_rm_auth_user(msg->job_id, msg->owner);
		  	
		  	if (!auth)
		  	{
		  		gw_rm_permission_denied(msg);
		  		gw_log_print("RM",'W',"Permission denied, user %s can not wait job %i\n",msg->owner,msg->job_id);
	  			free (msg);
	  			break;
		  	}
	  		
	  		if ( msg->wait_type == GW_MSG_WAIT_ANY )
	  		{ /* 
	  		   * Not the first in set, return if we already 
	  		   * notified the client, i.e. no WAITs_ANY in list
	  		   */
	  			in_list = gw_connection_list_wait_in_list(gw_rm.connection_list,					
				   				                          GW_MSG_WAIT_ANY,
							                              msg->client_socket);

	  			if (in_list == GW_FALSE)
	  			{
	  				free(msg);
	  				break;
	  			}
	  		} 
	  		else if ( msg->wait_type == GW_MSG_WAIT_ANY_FIRST)
	  			msg->wait_type = GW_MSG_WAIT_ANY;

	  		gw_connection_list_add(&(gw_rm.connection_list),
	  						   msg->client_socket,
	  						   msg->msg_type,
	  						   msg->wait_type,
	  						   msg->job_id);
	  		
	  		job_id  = (int *) malloc(sizeof(int));
	  		*job_id = msg->job_id;
	  		
			gw_am_trigger(gw_rm.dm_am, "GW_DM_WAIT", (void *) job_id);
			
			free(msg);
		  	break;
		  	
	  	case GW_MSG_KILL:
	  	case GW_MSG_KILL_ASYNC:
	  	case GW_MSG_KILL_HARD:
	  		  	
	  		auth = gw_rm_auth_user(msg->job_id, msg->owner);
		  	
		  	if (!auth)
		  	{
		  		gw_rm_permission_denied(msg);
		  		gw_log_print("RM",'W',"Permission denied, user %s can not kill job %i\n",msg->owner,msg->job_id);		  		
	  			free (msg);
	  			break;
		  	}	  	
			
			if ( msg->msg_type == GW_MSG_KILL )
		  		gw_connection_list_add(&(gw_rm.connection_list),
	  						   msg->client_socket,
	  						   msg->msg_type,
	  						   msg->wait_type,	  						   
	  						   msg->job_id);
	  	
	  		job_id  = (int *) malloc(sizeof(int));
	  		*job_id = msg->job_id;

			if ( msg->msg_type == GW_MSG_KILL_HARD )
				gw_am_trigger(gw_rm.dm_am, "GW_DM_KILL_HARD", (void *) job_id);			
			else	  		
				gw_am_trigger(gw_rm.dm_am, "GW_DM_KILL", (void *) job_id);
			
			free(msg);
		  	break;

	  	case GW_MSG_STOP:
	  	case GW_MSG_STOP_ASYNC:
	  		  	
	  		auth = gw_rm_auth_user(msg->job_id, msg->owner);
		  	
		  	if (!auth)
		  	{
		  		gw_rm_permission_denied(msg);
		  		gw_log_print("RM",'W',"Permission denied, user %s can not stop job %i\n",msg->owner,msg->job_id);		  				  		
	  			free (msg);
	  			break;
		  	}	  	
		  	
			if ( msg->msg_type == GW_MSG_STOP )	  
		  		gw_connection_list_add(&(gw_rm.connection_list),
	  						   msg->client_socket,
	  						   msg->msg_type,
	  						   msg->wait_type,	  						   
	  						   msg->job_id);
	  	
	  		job_id  = (int *) malloc(sizeof(int));
	  		*job_id = msg->job_id;
	  		
			gw_am_trigger(gw_rm.dm_am, "GW_DM_STOP", (void *) job_id);
			
			free(msg);
		  	break;
		  		  
	  	case GW_MSG_RESUME:
	  		auth = gw_rm_auth_user(msg->job_id, msg->owner);
		  	
		  	if (!auth)
		  	{
		  		gw_rm_permission_denied(msg);
		  		gw_log_print("RM",'W',"Permission denied, user %s can not resume job %i\n",msg->owner,msg->job_id);
	  			free (msg);
	  			break;
		  	}
	  
	  		gw_connection_list_add(&(gw_rm.connection_list),
	  						   msg->client_socket,
	  						   msg->msg_type,
	  						   msg->wait_type,	  						   
	  						   msg->job_id);

	  		job_id  = (int *) malloc(sizeof(int));
	  		*job_id = msg->job_id;
	  		
			gw_am_trigger(gw_rm.dm_am, "GW_DM_RESUME", (void *) job_id);
			
			free(msg);
		  	break;

	  	case GW_MSG_HOLD:
	  		auth = gw_rm_auth_user(msg->job_id, msg->owner);
		  	
		  	if (!auth)
		  	{
		  		gw_rm_permission_denied(msg);
		  		gw_log_print("RM",'W',"Permission denied, user %s can not hold job %i\n",msg->owner,msg->job_id);
	  			free (msg);
	  			break;
		  	}
	  
	  		gw_connection_list_add(&(gw_rm.connection_list),
	  						   msg->client_socket,
	  						   msg->msg_type,
	  						   msg->wait_type,	  						   
	  						   msg->job_id);

	  		job_id  = (int *) malloc(sizeof(int));
	  		*job_id = msg->job_id;
	  		
			gw_am_trigger(gw_rm.dm_am, "GW_DM_HOLD", (void *) job_id);
			
			free(msg);
		  	break;

	  	case GW_MSG_RELEASE:
	  		auth = gw_rm_auth_user(msg->job_id, msg->owner);
		  	
		  	if (!auth)
		  	{
		  		gw_rm_permission_denied(msg);
		  		gw_log_print("RM",'W',"Permission denied, user %s can not release job %i\n",msg->owner,msg->job_id);
	  			free (msg);
	  			break;
		  	}

	  		gw_connection_list_add(&(gw_rm.connection_list),
	  						   msg->client_socket,
	  						   msg->msg_type,
	  						   msg->wait_type,	  						   
	  						   msg->job_id);

	  		job_id  = (int *) malloc(sizeof(int));
	  		*job_id = msg->job_id;

			gw_am_trigger(gw_rm.dm_am, "GW_DM_RELEASE", (void *) job_id);
			
			free(msg);
		  	break;

	  	case GW_MSG_RESCHEDULE:
	  		auth = gw_rm_auth_user(msg->job_id, msg->owner);
		  	
		  	if (!auth)
		  	{
		  		gw_rm_permission_denied(msg);
		  		gw_log_print("RM",'W',"Permission denied, user %s can not reschedule job %i\n",msg->owner,msg->job_id);		  		
	  			free (msg);
	  			break;
		  	}
	  
	  		gw_connection_list_add(&(gw_rm.connection_list),
	  						   msg->client_socket,
	  						   msg->msg_type,
	  						   msg->wait_type,	  						   
	  						   msg->job_id);

	  		job_id  = (int *) malloc(sizeof(int));
	  		*job_id = msg->job_id;
	  		
			gw_am_trigger(gw_rm.dm_am, "GW_DM_RESCHEDULE", (void *) job_id);
			
			free(msg);
		  	break;
		  	
	  	case GW_MSG_JOB_STATUS:
	  	
			gw_rm_job_status(msg->client_socket, msg->job_id);
			free(msg);
		  	break;
		  	
	  	case GW_MSG_JOB_POOL_STATUS:
	  	
			gw_rm_job_pool_status(msg->client_socket);
			free(msg);
		  	break;
		  	
	  	case GW_MSG_HOST_STATUS:
	  	
			gw_rm_host_status(msg->client_socket, msg->job_id);
			free(msg);
		  	break;
		  	
	  	case GW_MSG_HOST_POOL_STATUS:
	  	
			gw_rm_host_pool_status(msg->client_socket);
			free(msg);
		  	break;				  	

	  	case GW_MSG_JOB_HISTORY:
	  	
			gw_rm_job_history(msg->client_socket, msg->job_id);
			free(msg);
		  	break;		  	

	  	case GW_MSG_JOB_MATCH:
	  	
	  	    if (msg->array_id == -1)
			    gw_rm_job_match(msg->client_socket, msg->job_id);
			else
			    gw_rm_array_match(msg->client_socket, msg->array_id);
			    
			free(msg);
		  	break;

	  	case GW_MSG_USERS:
	  	
			gw_rm_user_pool(msg->client_socket);
			free(msg);
		  	break;
		  	
		default:
			free(msg);
			break;
    }
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
                            
void gw_rm_set_dm_am (gw_am_t *dm_am)
{
    gw_rm.dm_am = dm_am;
}
