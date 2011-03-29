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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "gw_tm_ftp_transfer.h"


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

void   gw_tm_ftp_queue_init    (gw_tm_ftp_queue_t ** queue)
{
    *queue = NULL ;
}

/*----------------------------------------------------------------------------*/

void   gw_tm_ftp_queue_destroy (gw_tm_ftp_queue_t ** queue)
{
    if (*queue != NULL)
    {
		gw_tm_ftp_queue_destroy (&((*queue)->next));
        
        free((*queue)->src_url);
        free((*queue)->dst_url);        
                
        free((*queue));
    }
}

/*----------------------------------------------------------------------------*/

void gw_tm_ftp_queue_put (gw_tm_ftp_queue_t ** queue, 
						 const char *          src,
                         const char *          dst,
                         int                   cp_xfr_id,
                         gw_tm_ftp_cp_t        cp_type)                        
{
	gw_tm_ftp_queue_t ** tmp_queue;
	if ( *queue == NULL )
	{
		*queue = (gw_tm_ftp_queue_t *) malloc (sizeof(gw_tm_ftp_queue_t));
		(*queue)->src_url   = strdup (src);
		(*queue)->dst_url   = strdup (dst);
		(*queue)->cp_xfr_id = cp_xfr_id;
		(*queue)->cp_type   = cp_type;
		(*queue)->next      = NULL;
	}
	else
	{
		tmp_queue = &((*queue)->next);
		while ( *tmp_queue != NULL )
			tmp_queue = &((*tmp_queue)->next);

		*tmp_queue = (gw_tm_ftp_queue_t *) malloc (sizeof(gw_tm_ftp_queue_t));
		(*tmp_queue)->src_url   = strdup (src);
		(*tmp_queue)->dst_url   = strdup (dst);
		(*tmp_queue)->cp_xfr_id = cp_xfr_id;
		(*tmp_queue)->cp_type   = cp_type;
		(*tmp_queue)->next      = NULL;
	}	
}                                        

/*----------------------------------------------------------------------------*/
                                    
gw_tm_ftp_queue_t * gw_tm_ftp_queue_get (gw_tm_ftp_queue_t **  queue)
{
	gw_tm_ftp_queue_t *xfr;
	
	xfr  = *queue;
	
	if (*queue != NULL )
		*queue = (*queue)->next;
	else
		*queue = NULL;
		
	return xfr;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
