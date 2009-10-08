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
#include "gw_template.h"
#include "gw_host.h"

extern gw_client_t gw_client;

static void gw_client_dep_cp (const int * src, int dst[]);

int  gw_client_connect();
void gw_client_disconnect(int socket);

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

gw_return_code_t gw_client_job_submit(char *         template,
                                      gw_job_state_t init_state,
                                      int *          job_id,
                                      int *          deps,
                                      int            fixed_priority)
{
    gw_msg_t  msg;
 	int       rc;
 	int       length;
	int       fd;
	gw_host_t host = {PTHREAD_MUTEX_INITIALIZER,NULL,NULL,NULL,0,0,0,0,"str",
	"str","str","str","str",1,1,1,1,1,1,1,1,"str","str","str",{"str"},{1},{1},
	{0},{0},{1},{1},{1},{"str"},{"str"},{"str"}};
	
	*job_id = -1;
	 	
 	if ( gw_client.initialize == GW_FALSE )
		return GW_RC_FAILED_INIT;
		
		
    if ((fixed_priority != GW_JOB_DEFAULT_PRIORITY) && (
        (fixed_priority < GW_JOB_MIN_PRIORITY) ||
        (fixed_priority > GW_JOB_MAX_PRIORITY)))
		return GW_RC_FAILED_PERM;
		
    /* ----------------------------------------------------------------- */
    /* 1.- Format msg     	      	      	      	      	      	     */
    /* ----------------------------------------------------------------- */
    
    if ( ( init_state != GW_JOB_STATE_PENDING ) && 
         ( init_state != GW_JOB_STATE_HOLD    ) )
    	init_state = GW_JOB_STATE_PENDING;
    
    msg.init_state = init_state;	     
  	msg.msg_type   = GW_MSG_SUBMIT;  	
  	msg.pinc       = 0;
  	msg.pstart     = 0;
  	
  	msg.fixed_priority = fixed_priority;
  	
	rc = gw_template_init(&(msg.jt), template);
	if ( rc != 0 )
		return GW_RC_FAILED_JT;
	
	rc =  gw_host_client_check_syntax(&host, msg.jt.requirements, msg.jt.rank);
	
	if ( rc == -2 )
		return GW_RC_FAILED;
	else if ( rc == -1 )
		return GW_RC_FAILED_JT;
			
	if (deps != NULL)
	{
		if ( deps[0] != -1 )
		{
			msg.init_state = GW_JOB_STATE_HOLD;
			gw_client_dep_cp (deps, msg.jt.job_deps);
		}
	}

	pthread_mutex_lock(&(gw_client.mutex));
	
	strncpy(msg.owner,gw_client.owner,GW_MSG_STRING_SHORT);
	strncpy(msg.group,gw_client.group,GW_MSG_STRING_SHORT);
	
	pthread_mutex_unlock(&(gw_client.mutex));	
	
	length = sizeof(gw_msg_t);

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
		*job_id = msg.job_id;
	
	gw_client_disconnect(fd);
	
	return msg.rc;
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

gw_return_code_t gw_client_array_submit(char *         template,
                                        int            tasks, 
                                        gw_job_state_t init_state,                                        
                                        int *          array_id, 
                                        int **         job_ids,
                                        int *          deps,
                                        int            pstart,
                                        int            pinc,
                                        int            fixed_priority)
{
    gw_msg_t  msg;          
    int       i;
    int       length;
	int       rc;
	int       fd;
	gw_host_t host = {PTHREAD_MUTEX_INITIALIZER,NULL,NULL,NULL,0,0,0,0,"str",
	"str","str","str","str",1,1,1,1,1,1,1,1,"str","str","str",{"str"},{1},{1},
	{0},{0},{1},{1},{1},{"str"},{"str"},{"str"}};

 	if ( gw_client.initialize == GW_FALSE )
 	{
 		(*job_ids)  = NULL;
 		(*array_id) = -1;
 		
		return GW_RC_FAILED_INIT;
 	}

    if ((fixed_priority != GW_JOB_DEFAULT_PRIORITY) && (
        (fixed_priority < GW_JOB_MIN_PRIORITY) ||
        (fixed_priority > GW_JOB_MAX_PRIORITY)))
	{
 		(*job_ids)  = NULL;
 		(*array_id) = -1;
		
		return GW_RC_FAILED_PERM;
	}
	
    (*job_ids) = (int *) malloc (sizeof(int) * tasks);
    
    if((*job_ids) == NULL)
    {
      *array_id = -1;
      return GW_RC_FAILED_NO_MEMORY;
    }
    
	if ( tasks <= 0 )
		return GW_RC_FAILED;
		
    /* ----------------------------------------------------------------- */
    /* 1.- Format msg     	      	      	      	      	      	     */
    /* ----------------------------------------------------------------- */
    
    if ( ( init_state != GW_JOB_STATE_PENDING ) && 
         ( init_state != GW_JOB_STATE_HOLD    ) )
    	init_state = GW_JOB_STATE_PENDING;
    	 
  	msg.msg_type        = GW_MSG_SUBMIT_ARRAY;
  	msg.init_state      = init_state;
  	msg.number_of_tasks = tasks;
  	msg.pinc            = pinc;
  	msg.pstart          = pstart;
  	msg.fixed_priority  = fixed_priority;
  	
	rc = gw_template_init(&(msg.jt), template);
	if ( rc != 0 )
		return GW_RC_FAILED_JT;

	rc =  gw_host_client_check_syntax(&host, msg.jt.requirements, msg.jt.rank);
	
	if ( rc == -2 )
		return GW_RC_FAILED;
	else if ( rc == -1 )
		return GW_RC_FAILED_JT;
		
	if (deps != NULL)
	{
		if ( deps[0] != -1 )
		{
			msg.init_state = GW_JOB_STATE_HOLD;
			gw_client_dep_cp (deps, msg.jt.job_deps);
		}
	}

	pthread_mutex_lock(&(gw_client.mutex));
	
	strncpy(msg.owner,gw_client.owner,GW_MSG_STRING_SHORT);
	strncpy(msg.group,gw_client.group,GW_MSG_STRING_SHORT);
		
	pthread_mutex_unlock(&(gw_client.mutex));
	
	length = sizeof(gw_msg_t);
      
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
		fprintf(stderr,"Error reading message\n");
		close(fd);
		
		return GW_RC_FAILED_CONNECTION;
	}

    /* ----------------------------------------------------------------- */
    /* 3.- Receive response     	      	      	      	      	     */
    /* ----------------------------------------------------------------- */

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
	{
		*array_id = msg.array_id;
		
        gw_client_job_status_all( );

		pthread_mutex_lock(&(gw_client.mutex));
        
        for(i=0; i< gw_client.number_of_jobs; i++)
        	if ((gw_client.job_pool[i] != NULL) && 
        	    (gw_client.job_pool[i]->array_id == *array_id ))
            	(*job_ids)[gw_client.job_pool[i]->task_id] = gw_client.job_pool[i]->id;
            	
		pthread_mutex_unlock(&(gw_client.mutex));
	}
	else
		free(*job_ids);
		
    
    gw_client_disconnect(fd);
    
    return msg.rc;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

static void gw_client_dep_cp (const int * src, int dst[])
{
	int i = 0;
	
	if (src == NULL)
	{
		dst[0] = -1;
		return;
	}
	
	while (src[i]!=-1)
    	i++;

	if ( i == 0 )
		dst[0] = -1;
	else
	{		
		i = 0;		
		while ((src[i] != -1) && (i<(GW_JT_DEPS -1)))
		{
			dst[i] = src[i];
			i++;
		}
		
		dst[i] = -1;
	}	
}

/* ------------------------------------------------------------------------- */
