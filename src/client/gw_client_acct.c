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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <time.h>

#include "gw_client.h"

#ifdef HAVE_LIBDB

extern gw_client_t gw_client;

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

gw_return_code_t gw_client_user_accts(const char *user, gw_acct_t ***user_accts, int *num_users, time_t from_time)
{
	int rc;
	
	rc = gw_acct_db_open(GW_FALSE);
	
	if ( rc != 0 )
	{
		*user_accts = NULL;
		*num_users  = 0;
		
		return GW_RC_FAILED;
	}
	
	
	rc = gw_acct_join_search_by_user(user, user_accts, num_users, from_time);
	
 	gw_acct_db_close();
	
	if ( rc == 0 )
		return GW_RC_SUCCESS;	
	else 
		return GW_RC_FAILED;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

gw_return_code_t gw_client_host_accts(const char *host, gw_acct_t ***host_accts,  int *num_hosts, time_t from_time)
{
	int rc;
	
	rc = gw_acct_db_open(GW_FALSE);
	
	if ( rc != 0 )
	{
		*host_accts = NULL;
		*num_hosts  = 0;
		
		return GW_RC_FAILED;
	}

	rc = gw_acct_join_search_by_host(host, host_accts, num_hosts, from_time);
	
 	gw_acct_db_close();
	
	if ( rc == 0 )
		return GW_RC_SUCCESS;	
	else 
		return GW_RC_FAILED;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

gw_return_code_t gw_client_host_and_user_accts(const char *host, const char *user, gw_acct_t ***accts, int *num_records, time_t from_time)
{
	int rc;
	rc = gw_acct_db_open(GW_FALSE);
	
	if ( rc != 0 )
	{
		*accts = NULL;
		*num_records  = 0;
		
		return GW_RC_FAILED;
	}
	
	rc = gw_acct_join_search_by_host_and_user(host, user, accts, num_records, from_time);

 	gw_acct_db_close();

	if ( rc == 0 )
		return GW_RC_SUCCESS;	
	else 
		return GW_RC_FAILED;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/


#endif

