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
#include <signal.h>

#include "gw_rm_connection_list.h"
#include "gw_rm.h"

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

void gw_connection_list_init(gw_connection_list_t **list)
{
	pthread_mutex_init(&(gw_rm.connection_list_mutex),(pthread_mutexattr_t *) NULL);
    *list = NULL;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

void gw_connection_list_destroy(gw_connection_list_t **list)
{
	gw_connection_list_t *	tmp_list;
	
	pthread_mutex_lock(&(gw_rm.connection_list_mutex));
		
    while (*list != NULL)
    {
    	tmp_list = *list;
    	*list    = (*list)->next;
    	
    	free(tmp_list);
    }
    
	pthread_mutex_unlock(&(gw_rm.connection_list_mutex));    
    
    pthread_mutex_destroy(&(gw_rm.connection_list_mutex));
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

void gw_connection_list_add(gw_connection_list_t ** list,
                            int                     socket_fs,
                            gw_msg_type_t           msg_id,
                            gw_msg_wait_type_t      wait_type,
							int                     job_id)
{
	gw_connection_list_t * new_connection;
	
	pthread_mutex_lock(&(gw_rm.connection_list_mutex));
	
	new_connection = (gw_connection_list_t *) malloc (sizeof(gw_connection_list_t));
	
	new_connection->job_id    = job_id;
	new_connection->msg_id    = msg_id;
	new_connection->wait_type = wait_type;
	new_connection->socket_fs = socket_fs;
	
	
	if ( *list != NULL )
		new_connection->next = *list;
	else
		new_connection->next = NULL;
				
	*list = new_connection;
	
	pthread_mutex_unlock(&(gw_rm.connection_list_mutex)); 	
}							

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

gw_connection_list_t * gw_connection_list_get(gw_connection_list_t ** list,							
				   							  gw_msg_type_t           msg_id,
							                  int                     job_id)
{
	gw_connection_list_t *	tmp_list;
	gw_connection_list_t *	connection;

	pthread_mutex_lock(&(gw_rm.connection_list_mutex)); 	
	
	if ((*list) == NULL)
	{
		pthread_mutex_unlock(&(gw_rm.connection_list_mutex)); 
		return NULL;
	}
		
	if (((*list)->msg_id == msg_id ) && ( (*list)->job_id == job_id ))
	{
		connection = *list;
		*list      = (*list)->next;
		
		pthread_mutex_unlock(&(gw_rm.connection_list_mutex)); 
		
		return connection;
	}
	
	connection = (*list)->next;
	tmp_list   =  *list;
	
	while (connection != NULL)
	{
		if (( connection->msg_id == msg_id ) && ( connection->job_id == job_id ))
		{			
			tmp_list->next = connection->next;
			break;
		}
		else
			tmp_list   = connection;
			connection = tmp_list->next;
	}

	pthread_mutex_unlock(&(gw_rm.connection_list_mutex)); 	
	return connection;
}							                  

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

gw_connection_list_t * gw_connection_list_get_by_client(gw_connection_list_t ** list,							
				   							  			gw_msg_type_t           msg_id,
							                  			int                     client_socket)
{
	gw_connection_list_t *	tmp_list;
	gw_connection_list_t *	connection;

	pthread_mutex_lock(&(gw_rm.connection_list_mutex));
		
	if ((*list) == NULL)
	{
		pthread_mutex_unlock(&(gw_rm.connection_list_mutex));
		return NULL;
	}
		
	if (((*list)->msg_id == msg_id ) && ( (*list)->socket_fs == client_socket ))
	{
		connection = *list;
		*list      = (*list)->next;
		
		pthread_mutex_unlock(&(gw_rm.connection_list_mutex));		
		
		return connection;
	}
	
	connection = (*list)->next;
	tmp_list   =  *list;
	
	while (connection != NULL)
	{
		if (( connection->msg_id == msg_id ) && ( connection->socket_fs == client_socket ))
		{			
			tmp_list->next = connection->next;
			break;
		}
		else
			tmp_list   = connection;
			connection = tmp_list->next;
	}

	pthread_mutex_unlock(&(gw_rm.connection_list_mutex));
		
	return connection;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

gw_boolean_t gw_connection_list_wait_in_list(gw_connection_list_t * list,							
				   				             gw_msg_wait_type_t     wait_type,
							                 int                    client_socket)
{
	gw_boolean_t in_list=GW_FALSE;

	pthread_mutex_lock(&(gw_rm.connection_list_mutex));

	while (list != NULL)
	{
		if ((list->wait_type == wait_type) && (list->socket_fs == client_socket))
		{			
			in_list = GW_TRUE;
			break;
		}
		else
			list = list->next;
	}

	pthread_mutex_unlock(&(gw_rm.connection_list_mutex));
		
	return in_list;	
}							            
			                  
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void gw_connection_list_delete(gw_connection_list_t ** list, int client_socket)
{
	gw_connection_list_t *	tmp_list;
	gw_connection_list_t *	tmp_next;

	pthread_mutex_lock(&(gw_rm.connection_list_mutex));
		
	if ((*list) == NULL)
	{
		pthread_mutex_unlock(&(gw_rm.connection_list_mutex));
		return;
	}

	while ( (*list)->socket_fs == client_socket)
	{
		tmp_list = (*list)->next;
		free(*list);
		*list = tmp_list;
		
		if ((*list) == NULL)
		{
			pthread_mutex_unlock(&(gw_rm.connection_list_mutex));			
			return;
		}
	}
	
	tmp_list =  *list;
	tmp_next = (*list)->next;
	
	while (tmp_next != NULL)
	{
		if (tmp_next->socket_fs == client_socket)
		{
			tmp_list->next = tmp_next->next;
			free(tmp_next);
			tmp_next = tmp_list->next;	
		}
		else
		{
			tmp_list = tmp_next;
			tmp_next = tmp_list->next;
		}
	}
	
	pthread_mutex_unlock(&(gw_rm.connection_list_mutex));
}

