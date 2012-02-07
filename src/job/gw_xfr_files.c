/* -------------------------------------------------------------------------- */
/* Copyright 2002-2012, GridWay Project Leads (GridWay.org)                   */
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
#include <time.h>

#include "gw_xfr_files.h"

/* -------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------*/
/* -------------------------------------------------------------------------- */

void gw_xfr_init (gw_xfrs_t *xfrs, int number_of_xfrs, int tries)
{
	int i;
	
	xfrs->xfrs           = (gw_xfr_t *) malloc(number_of_xfrs*sizeof(gw_xfr_t));
	xfrs->number_of_xfrs = number_of_xfrs;
	xfrs->failure_limit  = 0;
	
	for (i = 0; i<number_of_xfrs ; i++)
	{
		xfrs->xfrs[i].src_url       = NULL;
		xfrs->xfrs[i].dst_url       = NULL;
		xfrs->xfrs[i].alt_src_url   = NULL;
		xfrs->xfrs[i].tries         = tries;
		xfrs->xfrs[i].done          = GW_FALSE;
		xfrs->xfrs[i].success       = GW_FALSE;
		xfrs->xfrs[i].counter       = -1;
	}
}

/* -------------------------------------------------------------------------- */

void gw_xfr_destroy (gw_xfrs_t *xfrs)
{
	int i;

	if 	( xfrs->xfrs != NULL )
	{		
		for (i = 0; i<xfrs->number_of_xfrs ; i++)
		{
			if ( xfrs->xfrs[i].src_url != NULL )
				free(xfrs->xfrs[i].src_url);
			
			if ( xfrs->xfrs[i].dst_url != NULL )
				free(xfrs->xfrs[i].dst_url);
			
			if ( xfrs->xfrs[i].alt_src_url != NULL )
				free(xfrs->xfrs[i].alt_src_url);
		}
	
		free(xfrs->xfrs);
	}
	
	xfrs->xfrs           = NULL;
	xfrs->number_of_xfrs = 0;
	xfrs->failure_limit  = 0;
}

/* -------------------------------------------------------------------------- */

int  gw_xfr_pending (gw_xfrs_t *xfrs)
{
	int pending_xfrs;
	int i;
	
	pending_xfrs = 0;
	
	if ( xfrs->xfrs != NULL )
	{
		for (i = 0; i<xfrs->number_of_xfrs ; i++)
		{
			if (( xfrs->xfrs[i].src_url != NULL ) && 
			    ( xfrs->xfrs[i].done    == GW_FALSE ))
				pending_xfrs++;
		}
	}
	
	return pending_xfrs;	
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

