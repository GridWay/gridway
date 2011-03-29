/* -------------------------------------------------------------------------- */
/* Copyright 2002-2011, GridWay Project Leads (GridWay.org)                   */
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

#include "gw_tm_ftp_transfer.h"
#include "gw_common.h"

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

extern gw_tm_ftp_transfer_t **	gw_tm_ftp_xfr_pool;
extern int	GW_TM_FTP_XFR_POOL_MAX;

int  gw_tm_ftp_mad_init (int ids)
{
	static int initialized = 0;
	int rc;
	
	if (initialized == 1 )
		return 0;
			    
   	rc = globus_module_activate(GLOBUS_FTP_CLIENT_MODULE);
    if ( rc != GLOBUS_SUCCESS )
        return 1;

   	rc = globus_module_activate(GLOBUS_GASS_COPY_MODULE);
    if ( rc != GLOBUS_SUCCESS )
        return 1;
 
	gw_tm_ftp_init_xfr_pool(ids);
	
	initialized = 1;
    
    return 0;
}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int gw_tm_ftp_transfer_init (gw_tm_ftp_transfer_t *xfr, int jid)
{
    globus_result_t  grc;
	
    grc = globus_ftp_client_handleattr_init (&(xfr->attr));
    if ( grc != GLOBUS_SUCCESS )
		return -1;
		
	grc = globus_ftp_client_handleattr_set_cache_all (&(xfr->attr), GLOBUS_TRUE);
    if ( grc != GLOBUS_SUCCESS )
		return -1;
	
    grc = globus_ftp_client_handle_init (&(xfr->handle),&(xfr->attr));
    if ( grc != GLOBUS_SUCCESS )
		return -1;

    grc = globus_ftp_client_operationattr_init (&(xfr->op_attr));
    if ( grc != GLOBUS_SUCCESS )
		return -1;

    grc = globus_gass_copy_attr_init (&(xfr->gass_attr));
    if ( grc != GLOBUS_SUCCESS )
		return -1;

    grc = globus_gass_copy_attr_init (&(xfr->src_gass_attr));
    if ( grc != GLOBUS_SUCCESS )
		return -1;

    grc = globus_ftp_client_operationattr_init (&(xfr->src_op_attr));
    if ( grc != GLOBUS_SUCCESS )
		return -1;

    grc = globus_gass_copy_attr_set_ftp (&(xfr->gass_attr), &(xfr->op_attr));
    if ( grc != GLOBUS_SUCCESS )
		return -1;
    
    grc = globus_gass_copy_attr_set_ftp (&(xfr->src_gass_attr), &(xfr->src_op_attr));
    if ( grc != GLOBUS_SUCCESS )
		return -1;
	
	grc = globus_gass_copy_handleattr_init(&(xfr->gass_handel_attr));
    if ( grc != GLOBUS_SUCCESS )
		return -1;
	
	grc = globus_gass_copy_handle_init  (&(xfr->gass_handle), &(xfr->gass_handel_attr));
    if ( grc != GLOBUS_SUCCESS )
		return -1;

    grc = globus_gass_copy_set_no_third_party_transfers (&(xfr->gass_handle), GLOBUS_FALSE);
    if ( grc != GLOBUS_SUCCESS )
		return -1;
        
    xfr->jid           = jid;
    xfr->handle_in_use = GW_FALSE;
    xfr->base_dir      = NULL;
    xfr->list_buffer   = NULL;    
    xfr->buffer_length = 0;
    xfr->read_buffer   = (globus_byte_t *) malloc( GW_TM_FTP_BUFFER_LENGTH * 
    												sizeof(globus_byte_t));
    xfr->current_xfr.dst_url   = NULL;
    xfr->current_xfr.src_url   = NULL;
    xfr->current_xfr.cp_xfr_id = 0;
    
	gw_tm_ftp_stack_init(&(xfr->file_stack));
	
	gw_tm_ftp_queue_init(&(xfr->url_queue));	
	
	return 0;
}

void gw_tm_ftp_transfer_flush (gw_tm_ftp_transfer_t *xfr)
{    
	if ( xfr->list_buffer != NULL )
		free(xfr->list_buffer);
		
	xfr->list_buffer = NULL;
    xfr->buffer_length = 0;
}

void gw_tm_ftp_transfer_cancel_cb(	void *                      user_arg,
                                   	globus_gass_copy_handle_t * handle,
                                   	globus_object_t *           err)
{
	int xfr_id;
	gw_tm_ftp_transfer_t * xfr;	
	xfr_id = *( (int *) user_arg );
	
	if ( ( xfr_id < GW_TM_FTP_XFR_POOL_MAX ) && (xfr_id >= 0 ) )
	    if ( gw_tm_ftp_xfr_pool[xfr_id] != NULL )
	    	xfr = gw_tm_ftp_xfr_pool[xfr_id];
	    else
	    {
	    	free(user_arg);
	    	return;
	    }
	else
	{
    	free(user_arg);
		return;	
	}
	
   	free(user_arg);
	        
	globus_ftp_client_handle_destroy (&(xfr->handle));
    globus_ftp_client_handleattr_destroy (&(xfr->attr));
	
	globus_ftp_client_operationattr_destroy (&(xfr->op_attr));
	globus_ftp_client_operationattr_destroy (&(xfr->src_op_attr));
	
	globus_gass_copy_handle_destroy (&(xfr->gass_handle));
	globus_gass_copy_handleattr_destroy(&(xfr->gass_handel_attr));
	
	/* THERE IS NO FUNCTION TO DESTROY 	globus_gass_copy_attr_t */
	/*
	globus_gass_copy_attr_destroy(&(xfr->gass_attr));
	globus_gass_copy_attr_destroy(&(xfr->src_gass_attr));
	*/

	if ( xfr->current_xfr.src_url != NULL )
		free(xfr->current_xfr.src_url);	
		
	if ( xfr->current_xfr.dst_url != NULL )
		free(xfr->current_xfr.dst_url);
	
	if ( xfr->list_buffer != NULL )
		free(xfr->list_buffer);
		
	if ( xfr->read_buffer != NULL )
		free(xfr->read_buffer);	
		
	if ( xfr->base_dir != NULL )
		free(xfr->base_dir);

    gw_tm_ftp_stack_destroy(&(xfr->file_stack));
	gw_tm_ftp_queue_destroy(&(xfr->url_queue));	
	
	printf("END %i - SUCCESS -\n",xfr_id);
	
   	free(gw_tm_ftp_xfr_pool[xfr_id]);
   	
    gw_tm_ftp_xfr_pool[xfr_id]  = NULL;
	
}

void gw_tm_ftp_transfer_destroy (gw_tm_ftp_transfer_t *xfr)
{    
	int *xfr_id;
	globus_result_t rc;
	
	xfr_id  = ( int *) malloc (sizeof(int));
	*xfr_id = xfr->jid;
	
	globus_ftp_client_abort (&(xfr->handle));
	
	rc = globus_gass_copy_cancel( &(xfr->gass_handle), gw_tm_ftp_transfer_cancel_cb,
        (void *)  xfr_id);
    
    if ( rc != GLOBUS_SUCCESS ) /* No transfer in progress */
		gw_tm_ftp_transfer_cancel_cb((void *)  xfr_id, NULL, NULL);
	    
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int gw_tm_ftp_transfer_expand_url ( int                                   xfr_id,
									const char *                          url_dir,
									globus_ftp_client_complete_callback_t done_cb)
{
	globus_result_t        grc;
	int 			       rc;
	int				   length;
	gw_tm_ftp_transfer_t * xfr;
	int *                  _xfr_id;
	
	if ( ( xfr_id < GW_TM_FTP_XFR_POOL_MAX ) && (xfr_id >= 0 ) )
	    if ( gw_tm_ftp_xfr_pool[xfr_id] != NULL )
	    	xfr = gw_tm_ftp_xfr_pool[xfr_id];
	    else
	    	return 1;
	else
		return 1;	
		
	length = strlen(url_dir);

	if ( xfr->base_dir != NULL )
		free(xfr->base_dir);
	
	if ( url_dir[length-1] != '/' )
	{
		xfr->base_dir = (char *) malloc ( (length + 2)	* sizeof(char) );
		snprintf(xfr->base_dir,(length + 2)*sizeof(char), "%s/",url_dir);
	}
	else
		xfr->base_dir = strdup(url_dir);
		
	xfr->base_dir_length = strlen(xfr->base_dir);
	
	rc = gw_tm_ftp_stack_push(&(xfr->file_stack), xfr->base_dir, GW_TM_FTP_DIR, GW_TRUE);
		
	if ( rc != 0 )
		return 1;
	
	/* MACHINE LIST SHOULD BE CHECK IF FEATURE IS SUPPORTED */
	/* WE WILL USE LIST COMMAND...
	grc = globus_ftp_client_machine_list ( &(xfr->handle), xfr->base_dir, 
				&(xfr->op_attr), done_cb, (void *) xfr);
	*/
	
	_xfr_id    = ( int *) malloc (sizeof(int));
	*(_xfr_id) = xfr->jid;
	
	grc = globus_ftp_client_verbose_list( &(xfr->handle), xfr->base_dir,
				&(xfr->op_attr), done_cb, (void *) _xfr_id);
							
	if ( grc != GLOBUS_SUCCESS)
	{
		free(_xfr_id);
		return 1;
	}

	_xfr_id    = ( int *) malloc (sizeof(int));
	*(_xfr_id) = xfr->jid;
		
	grc = globus_ftp_client_register_read (&(xfr->handle), (xfr->read_buffer),
			GW_TM_FTP_BUFFER_LENGTH, gw_tm_ftp_list_read_callback, (void *) _xfr_id);
			
    if ( grc != GLOBUS_SUCCESS )
    {
    	free(_xfr_id); /*Leaving one int (_xfr_id) behind!!!!*/
        return 1;
    }
    
	return 0;
}									



/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_tm_ftp_list_read_callback(void *                             user_arg,
					    		  globus_ftp_client_handle_t *       handle,
								  globus_object_t *                  err,
								  globus_byte_t *                    buffer,
								  globus_size_t                      length,
								  globus_off_t                       offset,
								  globus_bool_t                      eof)
{
	
	gw_tm_ftp_transfer_t * xfr;	
    globus_result_t    grc;
    char *             temp_p = NULL;
	char *             start_ptr;
	char *             end_ptr;
	char               mode[16];
	char               links[16];
	char               owner[16];
	char               group[16];
	char               size[16];
	char               month[5];
	char               day[5];
	char               file[256];
	char               alt_file[256];	
	char *             filename;
	char *             url;
    int		       i,len, rc;
    int                xfr_id;
        		    
    if(err)  /* ERROR DIR DOES NOT EXISTS : ERROR HANDLING NOT HERE */
		return;

	xfr_id = *( (int *) user_arg );
	
	if ( ( xfr_id < GW_TM_FTP_XFR_POOL_MAX ) && (xfr_id >= 0 ) )
	    if ( gw_tm_ftp_xfr_pool[xfr_id] != NULL )
	    	xfr = gw_tm_ftp_xfr_pool[xfr_id];
	    else
	    {
	    	free(user_arg);
	    	return;
	    }
	else
	{
    	free(user_arg);
		return;	
	}
	   	
    if( xfr->list_buffer == NULL && eof && offset == 0)
    {
        xfr->list_buffer   = (char *) malloc( sizeof(char) *length); 
        xfr->buffer_length = length;
        
		memcpy(xfr->list_buffer,buffer,length);
    }
    else
    {
        if( (length + offset) > xfr->buffer_length )
        {
            temp_p             = (char *) realloc(xfr->list_buffer, length + offset);    
            xfr->list_buffer   = temp_p;
            xfr->buffer_length = length + offset;
        }       

        memcpy(xfr->list_buffer + offset,buffer,length);
    }
    
    if(!eof)
    {
		grc = globus_ftp_client_register_read (&(xfr->handle), buffer,
			GW_TM_FTP_BUFFER_LENGTH, gw_tm_ftp_list_read_callback, user_arg);
			
        if( grc != GLOBUS_SUCCESS ) /* ERROR & CANCEL? */
        {
		  printf("RMDIR %d - FAILURE globus_ftp_client_register_read command\n", xfr->jid);
		  free(user_arg);
          return;
        } 
    }
    else
    {
    	i         = 0;
    	start_ptr = xfr->list_buffer;
    	end_ptr   = xfr->list_buffer;
    	
    	while (i < xfr->buffer_length)
    	{
    		while ((*end_ptr != '\n') && (*end_ptr != '\r'))
    		{
    			end_ptr++;
    			i++;
    		}
    		
    		*end_ptr = '\0';
    		
    		rc = sscanf(start_ptr,"%s %s %s %s %s %s %s %s %s",
    		                 mode,
    		                 links,
    		                 owner,
    		                 group,
    		                 size,
    		                 month,
    		                 day,
    		                 file,
    		                 alt_file);
    		if ( rc == 8 )
    			filename = file;
    		else if (rc == 9)
    			filename = alt_file;
			else    		  			
  			{
  		    	start_ptr = end_ptr+1;
   				end_ptr   = start_ptr;
	     		i         = i+1;
   				continue;    				
   			}
  			
   			if ( (strcmp(filename,".") !=0) && 
				 (strcmp(filename,"..")!=0) )	
			{
    			len = xfr->base_dir_length + strlen(filename) + 1;
					
				if (mode[0] == '-')
			    {					
					url = (char *) malloc (sizeof(char)* len);
					
					snprintf(url, sizeof(char)* len,"%s%s",
						                xfr->base_dir, 
						                filename);
						                
					gw_tm_ftp_stack_push(&(xfr->file_stack), 
					                     url, 
						   			     GW_TM_FTP_FILE, 
						   			     GW_FALSE);
     			    free(url);						   			     
				}
				else if (mode[0] == 'd')
				{		
				 	url = (char *) malloc (sizeof(char)* (len+1));
					snprintf(url, sizeof(char)* (len + 1),"%s%s/",
					                    xfr->base_dir, 
					                    filename);
					                    
					gw_tm_ftp_stack_push(&(xfr->file_stack), 
					                     url, 
						   			     GW_TM_FTP_DIR, 
						   			     GW_FALSE);
     			    free(url);
				}
			}						   		

	    	start_ptr = end_ptr+1;
   			end_ptr   = start_ptr;
	   		i         = i+1;		
    	}
    	
    	free(xfr->list_buffer);
    	
    	xfr->list_buffer   = NULL;
    	xfr->buffer_length = 0;
    	
   	   	free(user_arg);
    }

    return;
}								  

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int gw_tm_ftp_transfer_rmdir ( int xfr_id,  const char * url_dir)					     
{
	int rc;

	rc = gw_tm_ftp_transfer_expand_url (xfr_id, url_dir, gw_tm_ftp_transfer_rmdir_cb);	
	
	return rc;
}							   

/* -------------------------------------------------------------------------- */
							   
void gw_tm_ftp_transfer_rmdir_cb(
        void *                         user_arg,
        globus_ftp_client_handle_t *   handle,
        globus_object_t *              err)
{
	gw_tm_ftp_transfer_t *  xfr;
	gw_tm_ftp_stack_t *     file;
    globus_result_t         grc;
    int                     xfr_id;
    
	xfr_id = *( (int *) user_arg );
	
	if ( ( xfr_id < GW_TM_FTP_XFR_POOL_MAX ) && (xfr_id >= 0 ) )
	    if ( gw_tm_ftp_xfr_pool[xfr_id] != NULL )
	    	xfr = gw_tm_ftp_xfr_pool[xfr_id];
	    else
	    {
	    	/*printf("RMDIR %d - FAILURE transfer does not exisit.\n", xfr_id);*/
	    	
	    	free(user_arg);
	    	return;
	    }
	else
	{
	    /*printf("RMDIR %d - FAILURE invalid transfer id in callback.\n", xfr_id);*/
	    			
    	free(user_arg);
		return;	
	}
    
    if(err != GLOBUS_SUCCESS) /* ERROR RMDIR OR EXPAND */
    {
		printf("RMDIR %d - FAILURE ftp_client_delete/rmdir command.\n",xfr->jid);
		
		free(user_arg);
		return;
    }

	file = gw_tm_ftp_stack_pop(&(xfr->file_stack));
	
	if( file != NULL )
	{
		if ( (file->type == GW_TM_FTP_DIR) && (file->expanded == GW_FALSE) )
		{
			
			gw_tm_ftp_transfer_flush (xfr);
			gw_tm_ftp_transfer_expand_url (xfr_id, file->file_name, 
					gw_tm_ftp_transfer_rmdir_cb);
					
			free(file->file_name);
			free(file);
			free(user_arg);
			return;						
		}
		else
		{
			if ( file->type == GW_TM_FTP_FILE )
				grc = globus_ftp_client_delete (&(xfr->handle),
        										file->file_name,
        								    	&(xfr->op_attr),
									        	gw_tm_ftp_transfer_rmdir_cb,
									        	user_arg);
			else
				grc = globus_ftp_client_rmdir ( &(xfr->handle),
												file->file_name,
												&(xfr->op_attr),
									        	gw_tm_ftp_transfer_rmdir_cb,
									        	user_arg);
									        	
			if ( grc != GLOBUS_SUCCESS )
			{
				printf("RMDIR %d - FAILURE ftp_client_delete/rmdir command\n", xfr->jid);
				free(user_arg);
			}					
				      
			free(file->file_name);
			free(file);
			return;
		}
	}
	
	printf("RMDIR %d - SUCCESS -\n", xfr->jid);	
	free(user_arg);
}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_tm_ftp_transfer_exists_dir_cb(void *                         user_arg,
							          globus_ftp_client_handle_t *   handle,
                                      globus_object_t *              err)
{
	gw_tm_ftp_transfer_t *  xfr;
	int                     xfr_id;
	
	xfr_id = *( (int *) user_arg );
	
	if ( ( xfr_id < GW_TM_FTP_XFR_POOL_MAX ) && (xfr_id >= 0 ) )
	    if ( gw_tm_ftp_xfr_pool[xfr_id] != NULL )
	    	xfr = gw_tm_ftp_xfr_pool[xfr_id];
	    else
	    {
	    	/*printf("EXISTS %d - FAILURE transfer does not exisit.\n", xfr_id);*/
	    	
	    	free(user_arg);
	    	return;
	    }
	else
	{
    	/*printf("EXISTS %d - FAILURE invalid transfer id in callback.\n", xfr_id);*/		
    	
    	free(user_arg);
		return;	
	}
    
    if(err != GLOBUS_SUCCESS) /* FILE DOES NOT EXISTS, OR GSS ERROR*/
    {    		
    	printf("EXISTS %d - FAILURE -\n", xfr->jid);
    }
    else
    {
		printf("EXISTS %d - SUCCESS -\n", xfr->jid);
    }
    
    free(user_arg);
}

/* ---------------------------------------------------------------------------- */

int gw_tm_ftp_transfer_exists_dir (int                              xfr_id,
						   		   const char *                     url_dir)
{
    globus_result_t        grc;
   	int				   length;
	gw_tm_ftp_transfer_t * xfr;
	int *                  _xfr_id;
	
	if ( ( xfr_id < GW_TM_FTP_XFR_POOL_MAX ) && (xfr_id >= 0 ) )
	    if ( gw_tm_ftp_xfr_pool[xfr_id] != NULL )
	    	xfr = gw_tm_ftp_xfr_pool[xfr_id];
	    else
	    	return 1;
	else
		return 1;	   	
	
	if ( xfr->base_dir != NULL )
		free(xfr->base_dir);

	length = strlen(url_dir);
			
	if ( url_dir[length-1] != '/' )
	{
		xfr->base_dir = (char *) malloc ( (length + 2)	* sizeof(char) );
		snprintf(xfr->base_dir,(length + 2)*sizeof(char), "%s/",url_dir);
	}
	else
		xfr->base_dir = strdup(url_dir);
		
	xfr->base_dir_length = strlen(xfr->base_dir);

	_xfr_id    = ( int *) malloc (sizeof(int));
	*(_xfr_id) = xfr->jid;
		
	grc = globus_ftp_client_exists ( &(xfr->handle), 
							         xfr->base_dir, 
							         &(xfr->op_attr), 
							         gw_tm_ftp_transfer_exists_dir_cb, 
							         (void *) _xfr_id);
	if (grc != GLOBUS_SUCCESS)
	{
		free(_xfr_id);
		return 1;
	}
	else
		return 0;
	
}							   
							   


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int gw_tm_ftp_transfer_mkdir(int                                xfr_id,
						   	 const char *                       url_dir)
{
    globus_result_t        grc;
   	int				   length;
	gw_tm_ftp_transfer_t * xfr;
	int *                  _xfr_id;

	if ( ( xfr_id < GW_TM_FTP_XFR_POOL_MAX ) && (xfr_id >= 0 ) )
	    if ( gw_tm_ftp_xfr_pool[xfr_id] != NULL )
	    	xfr = gw_tm_ftp_xfr_pool[xfr_id];
	    else
	    	return 1;
	else
		return 1;	   
			
	if ( xfr->base_dir != NULL )
		free(xfr->base_dir);
	
	length = strlen(url_dir);

	if ( url_dir[length-1] != '/' )
	{
		xfr->base_dir = (char *) malloc ( (length + 2)	* sizeof(char) );
		snprintf(xfr->base_dir,(length + 2)*sizeof(char), "%s/",url_dir);
	}
	else
		xfr->base_dir = strdup(url_dir);
		
	xfr->base_dir_length = strlen(xfr->base_dir);

	_xfr_id    = ( int *) malloc (sizeof(int));
	*(_xfr_id) = xfr->jid;
		
	grc = globus_ftp_client_mkdir ( &(xfr->handle), 
					         		xfr->base_dir, 
							        &(xfr->op_attr), 
									gw_tm_ftp_transfer_mkdir_cb,
			         				(void *) _xfr_id);
				
	if (grc != GLOBUS_SUCCESS)
	{
		free(_xfr_id);
		return 1;
	}
	else
		return 0;
	
}							   
							   
void gw_tm_ftp_transfer_mkdir_cb(void *                         user_arg,
							     globus_ftp_client_handle_t *   handle,
                                 globus_object_t *              err)
{
	gw_tm_ftp_transfer_t *  xfr;
	int                     xfr_id;
	
	xfr_id = *( (int *) user_arg );
	
	if ( ( xfr_id < GW_TM_FTP_XFR_POOL_MAX ) && (xfr_id >= 0 ) )
	    if ( gw_tm_ftp_xfr_pool[xfr_id] != NULL )
	    	xfr = gw_tm_ftp_xfr_pool[xfr_id];
	    else
	    {
	    	/*printf("MKDIR %d - FAILURE transfer does not exisit.\n", xfr_id);*/
	    	
	    	free(user_arg);
	    	return;
	    }
	else
	{
    	/*printf("MKDIR %d - FAILURE invalid transfer id in callback.\n", xfr_id);*/		
    	
    	free(user_arg);
		return;	
	}
    
    if(err != GLOBUS_SUCCESS) 
    {    		
    	printf("MKDIR %d - FAILURE %s\n", xfr->jid, xfr->base_dir);
    }
    else
    {
		printf("MKDIR %d - SUCCESS %s\n", xfr->jid,xfr->base_dir);
    }
    
    free(user_arg);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int gw_tm_ftp_transfer_url_to_url ( int                             xfr_id,
                                    int                             cp_xfr_id,
						            char *                          src_url,
						            char *                          dst_url,
						            char                            modex)
{
    globus_result_t         grc;
    int *                   _xfr_id;
    gw_tm_ftp_transfer_t *  xfr;
    
	if ( ( xfr_id < GW_TM_FTP_XFR_POOL_MAX ) && (xfr_id >= 0 ) )
	    if ( gw_tm_ftp_xfr_pool[xfr_id] != NULL )
	    	xfr = gw_tm_ftp_xfr_pool[xfr_id];
	    else
	    	return 1;
	else
		return 1;
		    
    
	if ( xfr->handle_in_use == GW_FALSE ) /* No transfers in progress, start this one */
	{
		xfr->handle_in_use = GW_TRUE;

		_xfr_id  = ( int *) malloc (sizeof(int));		
		*_xfr_id = xfr->jid;    
		
		if (xfr->current_xfr.src_url != NULL )
			free(xfr->current_xfr.src_url);
			
		xfr->current_xfr.src_url = strdup(src_url);
		
		if (xfr->current_xfr.dst_url != NULL )
			free(xfr->current_xfr.dst_url);
			
		xfr->current_xfr.dst_url = strdup(dst_url);
		
		xfr->current_xfr.cp_xfr_id = cp_xfr_id;
		
		if ( modex == 'X' )
			xfr->current_xfr.cp_type   = GW_TM_FTP_MODEX;
		else
			xfr->current_xfr.cp_type   = GW_TM_FTP_NONE;		
		
		grc = globus_gass_copy_register_url_to_url (
        	&(xfr->gass_handle), 
        	xfr->current_xfr.src_url, 
        	&(xfr->src_gass_attr), 
        	xfr->current_xfr.dst_url, 
        	&(xfr->gass_attr),
			gw_tm_ftp_transfer_url_to_url_cb, 
            (void *) _xfr_id );
            
		if (grc != GLOBUS_SUCCESS)
		{
			xfr->handle_in_use = GW_FALSE;
			free(_xfr_id);

			return 1;
		}
		else
			return 0;				
	}
	else /* Copying a file, queue this request */
	{
		if ( modex == 'X' )
			gw_tm_ftp_queue_put (&(xfr->url_queue), 
								 src_url,
								 dst_url, 
								 cp_xfr_id,
								 GW_TM_FTP_MODEX);
		else
			gw_tm_ftp_queue_put (&(xfr->url_queue), 
								 src_url,
								 dst_url, 
								 cp_xfr_id,
								 GW_TM_FTP_NONE);
		return 0;
	}
}							   

void gw_tm_ftp_transfer_url_to_url_cb(	void *                      user_arg,
                                    	globus_gass_copy_handle_t * handle,
                                    	globus_object_t *           err)
{
	gw_tm_ftp_transfer_t *  xfr;
	gw_tm_ftp_queue_t *     pending;
    globus_result_t  		grc;	
	int 					xfr_id;
	int *                   _xfr_id;
	int                     end;
	
	if (user_arg == NULL)
		return;
		
	xfr_id = *( (int *) user_arg );
	
	if ( ( xfr_id < GW_TM_FTP_XFR_POOL_MAX ) && (xfr_id >= 0 ) )
	    if ( gw_tm_ftp_xfr_pool[xfr_id] != NULL )
	    	xfr = gw_tm_ftp_xfr_pool[xfr_id];
	    else	    
	    {
	    	/*printf("CP %d - FAILURE transfer does not exisit.\n", xfr_id);*/
	    	
	    	free(user_arg);
	    	return;
	    }
	else
	{
    	/*printf("CP %d - FAILURE invalid transfer id in callback.\n", xfr_id);*/
    	
    	free(user_arg);
		return;	
	}	

   	free(user_arg);
   	
    if(err != GLOBUS_SUCCESS) 
    {
		printf("CP %d %d FAILURE (%s->%s)\n", 
		       xfr->jid, 
		       xfr->current_xfr.cp_xfr_id,
		       xfr->current_xfr.src_url,
		       xfr->current_xfr.dst_url);		       
    }
    else
    {	
    	if ( xfr->current_xfr.cp_type == GW_TM_FTP_MODEX )
    	{
    		xfr->current_xfr.cp_type = GW_TM_FTP_NONE;
    		
    		_xfr_id  = ( int *) malloc (sizeof(int));		
		    *_xfr_id = xfr->jid;    
    		
    		grc = globus_ftp_client_chmod (&(xfr->handle),
        						           xfr->current_xfr.dst_url, 
                                           0766, 
                                           &(xfr->op_attr), 
                                           gw_tm_ftp_transfer_chmod_cb,
                                           (void *)  _xfr_id); 
                                   
			if (grc != GLOBUS_SUCCESS)
			{
				printf("CP %d %d FAILURE chmod %s\n", 
				            xfr->jid, 
				            xfr->current_xfr.cp_xfr_id,
				            xfr->current_xfr.dst_url);
				            
				free(_xfr_id);
			}
			else
				return;
    	}
    	else
    	{
			printf("CP %d %d SUCCESS (%s->%s)\n", 
			       xfr->jid, 
			       xfr->current_xfr.cp_xfr_id,
			       xfr->current_xfr.src_url, 
			       xfr->current_xfr.dst_url);
    	}
    }
    /* Check for pending transfers and start them */
    
    pending =  gw_tm_ftp_queue_get (&(xfr->url_queue));
   
	if ( pending == NULL )
		xfr->handle_in_use = GW_FALSE;
	else
	{
		end = 0;
		
		while (!end)
		{
			_xfr_id  = ( int *) malloc (sizeof(int));		
			*_xfr_id = xfr->jid;    

			if (xfr->current_xfr.src_url != NULL )
				free(xfr->current_xfr.src_url);			
			xfr->current_xfr.src_url = strdup(pending->src_url);
		
			if (xfr->current_xfr.dst_url != NULL )
				free(xfr->current_xfr.dst_url);
			xfr->current_xfr.dst_url = strdup(pending->dst_url);
		
			xfr->current_xfr.cp_xfr_id = pending->cp_xfr_id;
		
			xfr->current_xfr.cp_type   = pending->cp_type;
		
			grc = globus_gass_copy_register_url_to_url (
	        	&(xfr->gass_handle), 
	        	xfr->current_xfr.src_url,
	        	&(xfr->src_gass_attr), 
	        	xfr->current_xfr.dst_url,
	        	&(xfr->gass_attr),
				gw_tm_ftp_transfer_url_to_url_cb, 
	            (void *) _xfr_id );

	        free(pending->src_url);
	        free(pending->dst_url);    	    
		    free(pending);
            
			if (grc != GLOBUS_SUCCESS)
			{
	    		printf("CP %d %d FAILURE url2url_cb %s -> %s\n", 
	    		       xfr->jid,
	    		       xfr->current_xfr.cp_xfr_id,
	    		       xfr->current_xfr.src_url,
	    		       xfr->current_xfr.dst_url);
		    	
	    	    pending =  gw_tm_ftp_queue_get (&(xfr->url_queue));
	    	    
	    	    if ( pending == NULL )
	    	    {
					xfr->handle_in_use = GW_FALSE;
	    	    	end = 1;
	    	    }
	    	    
	    	    free(_xfr_id);
			}
			else
				end = 1;
		}
	}
}


/* -------------------------------------------------------------------------- */

void gw_tm_ftp_transfer_chmod_cb( void *                         user_arg,
							      globus_ftp_client_handle_t *   handle,
                                  globus_object_t *              err)
{
	gw_tm_ftp_transfer_t *  xfr;
	gw_tm_ftp_queue_t *     pending;
    globus_result_t  		grc;	
	int 					xfr_id;
	int *                   _xfr_id;
	int                     end;
	
	if ( user_arg == NULL )
		return;
		
	xfr_id = *( (int *) user_arg );
	
	if ( ( xfr_id < GW_TM_FTP_XFR_POOL_MAX ) && (xfr_id >= 0 ) )
	    if ( gw_tm_ftp_xfr_pool[xfr_id] != NULL )
	    	xfr = gw_tm_ftp_xfr_pool[xfr_id];
	    else
	    {
	    	/*printf("CP %d - FAILURE transfer does not exisit (in chmod).\n", xfr_id);*/
	    	
	    	free(user_arg);
	    	return;
	    }
	else
	{
    	/*printf("CP %d - FAILURE invalid transfer id in callback (in chmod).\n", xfr_id);*/
    			
    	free(user_arg);
		return;	
	}	

   	free(user_arg);
	
    if(err != GLOBUS_SUCCESS) 
    {
		printf("CP %d %d FAILURE chmod (%s)\n", 
		       xfr->jid, 
		       xfr->current_xfr.cp_xfr_id,
			   xfr->current_xfr.dst_url);			
    }
	else
	{
		printf("CP %d %d SUCCESS X (%s->%s)\n", 
		       xfr->jid, 
		       xfr->current_xfr.cp_xfr_id,
		       xfr->current_xfr.src_url, 
		       xfr->current_xfr.dst_url);
	}
		       
    /* Check for pending transfers and start them */
    
    pending =  gw_tm_ftp_queue_get (&(xfr->url_queue));
   
	if ( pending == NULL )
		xfr->handle_in_use = GW_FALSE;
	else
	{
		end = 0;
		
		while (!end)
		{
			_xfr_id  = ( int *) malloc (sizeof(int));		
			*_xfr_id = xfr->jid;    

			if (xfr->current_xfr.src_url != NULL )
				free(xfr->current_xfr.src_url);			
			xfr->current_xfr.src_url = strdup(pending->src_url);
		
			if (xfr->current_xfr.dst_url != NULL )
				free(xfr->current_xfr.dst_url);
			xfr->current_xfr.dst_url = strdup(pending->dst_url);
		
			xfr->current_xfr.cp_xfr_id = pending->cp_xfr_id;
		
			xfr->current_xfr.cp_type   = pending->cp_type;
		
			grc = globus_gass_copy_register_url_to_url (
	        	&(xfr->gass_handle), 
	        	xfr->current_xfr.src_url,
	        	&(xfr->src_gass_attr), 
	        	xfr->current_xfr.dst_url,
	        	&(xfr->gass_attr),
				gw_tm_ftp_transfer_url_to_url_cb, 
	            (void *) _xfr_id );

	        free(pending->src_url);
	        free(pending->dst_url);    	    
		    free(pending);
            
			if (grc != GLOBUS_SUCCESS)
			{
	    		printf("CP %d %d FAILURE url2url_cb %s -> %s\n", 
	    		       xfr->jid,
	    		       xfr->current_xfr.cp_xfr_id,
	    		       xfr->current_xfr.src_url,
	    		       xfr->current_xfr.dst_url);
	    		       
	    		free(_xfr_id);
	    		_xfr_id = NULL;
		    	
	    	    pending =  gw_tm_ftp_queue_get (&(xfr->url_queue));
	    	    
	    	    if ( pending == NULL )
	    	    {
					xfr->handle_in_use = GW_FALSE;
	    	    	end = 1;
	    	    }
			}
			else
				end = 1;
		}
	}	
}

