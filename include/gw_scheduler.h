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

#define  GW_SCHED_TINF    3600
#define  GW_SCHED_C       650
#define  GW_SCHED_DELTA0  180
#define  GW_SCHED_DMEM    3

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */

typedef struct gw_sch_host_s
{
	int  hid;
	char name[GW_MSG_STRING_SHORT];

    int  used_slots;
	int  running_jobs;

	int  dispatched;

} gw_sch_host_t;

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */

typedef struct gw_sch_user_host_s
{
    int    ha_id;
    int    hid;
  
    time_t banned;
    time_t last_banned;
  
    float  avrg_transfer;
    float  avrg_execution;
    float  avrg_suspension;
  
    float  migration_ratio;

} gw_sch_user_host_t;

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */

typedef struct gw_sch_user_s
{
	int  uid;
	char name[GW_MSG_STRING_SHORT];
	
	int  running_jobs;
	int  active_jobs;

	int  dispatched;
	int  priority;
	int  share;
	
	gw_sch_user_host_t * hosts;
	
} gw_sch_user_t;

/* -------------------------------------------------------------------- */

typedef struct gw_sch_queue_s
{
	int  ha_id;
	int  uha_id;
	
	char qname[GW_MSG_STRING_SHORT];
	int  slots;

	int  nice;	
	int  rank;
	
} gw_sch_queue_t;

/* -------------------------------------------------------------------- */

typedef struct gw_sch_job_s
{
	int ua_id;
		
	int jid;
	int aid;
	
	int tasks;
    gw_migration_reason_t reason;
	
	int nice;
	
	int              num_mhosts;
    gw_sch_queue_t * mhosts;
	
} gw_sch_job_t;

/* -------------------------------------------------------------------- */

typedef struct gw_scheduler_s
{
  int             num_users;
  gw_sch_user_t * users;
  
  int             num_hosts;
  gw_sch_host_t * hosts;
  
  int             num_jobs;
  gw_sch_job_t *  jobs;
  
} gw_scheduler_t;

/* -------------------------------------------------------------------- */

typedef void (*gw_scheduler_function_t) (gw_scheduler_t * sched,
									     void *           user_arg);

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */

void gw_scheduler_loop(gw_scheduler_function_t scheduler, void *user_arg);

void gw_scheduler_print (const char mode, const char *str_format,...);

inline void gw_scheduler_ctime(time_t the_time, char *str);

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */

void gw_scheduler_add_host(gw_scheduler_t * sched, 
                           int              hid,
                           int              uslots,
                           int              rjobs,
                           char *           hostname);
                           
/* -------------------------------------------------------------------- */

void gw_scheduler_add_user(gw_scheduler_t * sched, 
                           int              uid, 
                           int              ajobs, 
                           int              rjobs, 
                           char *           name);

void gw_scheduler_del_user(gw_scheduler_t * sched, int uid);
                               
/* -------------------------------------------------------------------- */

void gw_scheduler_job_failed(gw_scheduler_t *      sched, 
                             int                   hid, 
                             int                   uid, 
                             gw_migration_reason_t reason);
                             
void gw_scheduler_job_success(gw_scheduler_t * sched, 
                              int              hid, 
                              int              uid, 
                              float            txfr,
                              float            tsus,
                              float            texe);

void gw_scheduler_job_add(gw_scheduler_t *      sched,
                          int                   jid, 
                          int                   aid, 
                          gw_migration_reason_t reason,
                          int                   nice,
                          int                   uid);
                          
void gw_scheduler_array_add(gw_scheduler_t *      sched,
                            int                   jid, 
                            int                   aid, 
                            gw_migration_reason_t reason,
                            int                   nice,
                            int                   uid,
                            int                   tasks);                          

void gw_scheduler_job_del(gw_scheduler_t *      sched,
                          int                   jid);
                          
void gw_scheduler_array_del(gw_scheduler_t *      sched,
                            int                   aid,
                            int                   tasks);
                          
                              
void gw_scheduler_matching_arrays(gw_scheduler_t * sched);

/* -------------------------------------------------------------------- */

#endif /*GW_DM_SCHEDULER_H_*/
