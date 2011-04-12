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

#ifndef GW_TM_FTP_TRANSFER_H_
#define GW_TM_FTP_TRANSFER_H_

#include "globus_ftp_client.h"
#include "globus_gass_copy.h"
#include "gw_common.h"

#define GW_TM_FTP_BUFFER_LENGTH 2048
#define GW_TM_FTP_FILE 0
#define GW_TM_FTP_DIR 1

/* ------------------------------------------------- */

struct gw_tm_ftp_stack_s;

typedef struct gw_tm_ftp_stack_s
{
    char *file_name;
    int type;
    gw_boolean_t expanded;
    
    struct gw_tm_ftp_stack_s *next;
	
}gw_tm_ftp_stack_t;

/* ------------------------------------------------- */

typedef enum {
   GW_TM_FTP_NONE,
   GW_TM_FTP_MODEX
} gw_tm_ftp_cp_t;

struct gw_tm_ftp_queue_s;

typedef struct gw_tm_ftp_queue_s
{
	char *src_url;
	char *dst_url;
	
	int cp_xfr_id;
	gw_tm_ftp_cp_t cp_type;
	
	struct gw_tm_ftp_queue_s *next;
	
}gw_tm_ftp_queue_t;

/* ------------------------------------------------- */

typedef struct gw_tm_ftp_transfer_s
{
    int jid;

    gw_boolean_t handle_in_use;
			
    globus_ftp_client_handle_t handle;
    globus_ftp_client_handleattr_t attr;
    globus_ftp_client_operationattr_t op_attr;
    globus_gass_copy_handle_t gass_handle;
    globus_gass_copy_handleattr_t gass_handel_attr;
    globus_gass_copy_attr_t gass_attr;

    globus_ftp_client_operationattr_t src_op_attr;
    globus_gass_copy_attr_t src_gass_attr;
	    
    globus_byte_t *read_buffer;

    gw_tm_ftp_queue_t *url_queue;

    gw_tm_ftp_queue_t current_xfr;	
	
    char *base_dir;
    int base_dir_length;
		
    char *list_buffer;
    int buffer_length;
	
    gw_tm_ftp_stack_t *file_stack;
	
} gw_tm_ftp_transfer_t;



void gw_tm_ftp_init_xfr_pool(int ids);
int gw_tm_ftp_add_xfr(int xfr_id);
int gw_tm_ftp_del_xfr(int xfr_id);

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

void gw_tm_ftp_queue_init(gw_tm_ftp_queue_t **queue);
void gw_tm_ftp_queue_destroy(gw_tm_ftp_queue_t **queue);

void gw_tm_ftp_queue_put(gw_tm_ftp_queue_t **queue, 
        const char *src,
        const char *dst,
        int cp_xfr_id,
        gw_tm_ftp_cp_t cp_type);                          
                          
gw_tm_ftp_queue_t * gw_tm_ftp_queue_get (gw_tm_ftp_queue_t **  queue);

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


void gw_tm_ftp_stack_init(gw_tm_ftp_stack_t **stack);

void gw_tm_ftp_stack_destroy(gw_tm_ftp_stack_t **stack);

int gw_tm_ftp_stack_push(gw_tm_ftp_stack_t **stack, const char *file_name, int type, gw_boolean_t expanded);

gw_tm_ftp_stack_t * gw_tm_ftp_stack_pop(gw_tm_ftp_stack_t **stack);


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int  gw_tm_ftp_mad_init(int ids);

int  gw_tm_ftp_transfer_init   (gw_tm_ftp_transfer_t *tm_transfer, int jid);
void gw_tm_ftp_transfer_destroy(gw_tm_ftp_transfer_t *tm_transfer);
void gw_tm_ftp_transfer_flush  (gw_tm_ftp_transfer_t *tm_transfer);

void gw_tm_ftp_list_read_callback(void *                             user_arg,
					    		  globus_ftp_client_handle_t *       handle,
								  globus_object_t *                  err,
								  globus_byte_t *                    buffer,
								  globus_size_t                      length,
								  globus_off_t                       offset,
								  globus_bool_t                      eof);
    
int gw_tm_ftp_parse_list(gw_tm_ftp_transfer_t *xfr);

int gw_tm_ftp_transfer_expand_url ( int                                   xfr_id,
									const char *                          url_dir,
									globus_ftp_client_complete_callback_t done_cb);

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int gw_tm_ftp_transfer_rmdir ( int                              xfr_id,
							   const char *                     url_dir);
							   
void gw_tm_ftp_transfer_rmdir_cb(void *                         user_arg,
                                 globus_ftp_client_handle_t *   handle,
                                 globus_object_t *              err);
        
/* -------------------------------------------------------------------------- */

int gw_tm_ftp_transfer_exists_dir (int                              xfr_id,
						   		   const char *                     url_dir);
							   
void gw_tm_ftp_transfer_exists_dir_cb(void *                         user_arg,
							          globus_ftp_client_handle_t *   handle,
                                      globus_object_t *              err);
							   
/* -------------------------------------------------------------------------- */

int gw_tm_ftp_transfer_mkdir(int                                xfr_id,
						   	 const char *                       url_dir);
							   
void gw_tm_ftp_transfer_mkdir_cb(void *                         user_arg,
							     globus_ftp_client_handle_t *   handle,
                                 globus_object_t *              err);

/* -------------------------------------------------------------------------- */
                                  
int gw_tm_ftp_transfer_url_to_url ( int                             xfr_id,
                                    int                             cp_xfr_id,
						            char *                          src_url,
						            char *                          dst_url,
						            char                            modex);

void gw_tm_ftp_transfer_url_to_url_cb(	void *                      user_arg,
                                    	globus_gass_copy_handle_t * handle,
                                    	globus_object_t *           result);
                                    	
void gw_tm_ftp_transfer_chmod_cb( void *                         user_arg,
							      globus_ftp_client_handle_t *   handle,
                                  globus_object_t *              err);                                    	
                                  
/* -------------------------------------------------------------------------- */

#endif /*GW_TM_FTP_TRANSFER_H_*/
