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

#include "gw_tm_ftp_transfer.h"

/*----------------------------------------------------------------------------*/


gw_tm_ftp_transfer_t **	gw_tm_ftp_xfr_pool;
int 					GW_TM_FTP_XFR_POOL_MAX;

/*----------------------------------------------------------------------------*/


void gw_tm_ftp_init_xfr_pool(int ids )
{
    int xfr_id;

	gw_tm_ftp_xfr_pool = (gw_tm_ftp_transfer_t **) 
			malloc (sizeof(gw_tm_ftp_transfer_t *) * ids);
			
	GW_TM_FTP_XFR_POOL_MAX = ids;
	
    for (xfr_id = 0; xfr_id<GW_TM_FTP_XFR_POOL_MAX ; xfr_id++)
        gw_tm_ftp_xfr_pool[xfr_id]  = NULL;
}

/*----------------------------------------------------------------------------*/


int gw_tm_ftp_add_xfr(int xfr_id)
{
	if ( ( xfr_id < GW_TM_FTP_XFR_POOL_MAX ) && (xfr_id >= 0 ) )
	{
    	if ( gw_tm_ftp_xfr_pool[xfr_id] != NULL )
        	return 1;
    
	    gw_tm_ftp_xfr_pool[xfr_id] = (gw_tm_ftp_transfer_t *) malloc (sizeof(gw_tm_ftp_transfer_t));
	    gw_tm_ftp_transfer_init(gw_tm_ftp_xfr_pool[xfr_id], xfr_id);
	    
        return 0;
	}
	else 
		return 1;
}

/*----------------------------------------------------------------------------*/

int gw_tm_ftp_del_xfr( int xfr_id )
{
	if ( ( xfr_id < GW_TM_FTP_XFR_POOL_MAX ) && (xfr_id >= 0 ) )
	{
	    if ( gw_tm_ftp_xfr_pool[xfr_id] != NULL )
	    {
	    	gw_tm_ftp_transfer_destroy (gw_tm_ftp_xfr_pool[xfr_id]);
	        return 0;
	    }
	    else
	    	return 1;
	}
	else
		return 1;		
}

