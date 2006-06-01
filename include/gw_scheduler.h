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

#ifndef GW_DM_SCHEDULER_H_
#define GW_DM_SCHEDULER_H_

#include "gw_client.h"

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */

typedef struct gw_sch_job_s
{
	int jid;
	int aid;
	int uid;
	
	int tasks;
	int resch;
	
} gw_sch_job_t;

/* -------------------------------------------------------------------- */

typedef struct gw_sch_queue_s
{
	int  hid;
	char name[GW_MSG_STRING_SHORT];
	int  slots;
	int  rank;
		
} gw_sch_queue_t;

/* -------------------------------------------------------------------- */

typedef struct gw_sch_host_s
{
	int  hid;
	char name[GW_MSG_STRING_SHORT];
	
    int used_slots;
	int running_jobs;
	
	int dispatched;
	
} gw_sch_host_t;

/* -------------------------------------------------------------------- */

typedef struct gw_sch_user_s
{
	int  uid;
	char name[GW_MSG_STRING_SHORT];
	
	int  running_jobs;
	int  active_jobs;			

	int  dispatched;
			
} gw_sch_user_t;

/* -------------------------------------------------------------------- */

typedef void (*gw_scheduler_function_t) (gw_sch_job_t *    jobs,
									     gw_sch_queue_t ** match_queues,
									     gw_sch_user_t *   users,
									     gw_sch_host_t *   hosts,
									     void *            user_arg);

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */

void gw_scheduler_loop(gw_scheduler_function_t scheduler, void *user_arg);


#endif /*GW_DM_SCHEDULER_H_*/
