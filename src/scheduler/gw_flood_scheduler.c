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

#include "gw_scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

/* ------------------------------------------------------------------------- */
/* GLOBAL VARIABLES & TYPES                                                  */
/* ------------------------------------------------------------------------- */

extern char *optarg;
extern int   optind, opterr, optopt;

typedef struct sched_param_s
{
	int host_max;	
	int user_chunk;
	int user_max;		
} sched_param_t;

/* ------------------------------------------------------------------------- */
/* FUNCTION DEFINICION                                                       */
/* ------------------------------------------------------------------------- */

static void setup_args(char **argv, char ***av, int *ac);

static void flood_scheduler (gw_sch_job_t *    jobs,
							 gw_sch_queue_t ** match_queues,
							 gw_sch_user_t *   users,
							 gw_sch_host_t *   hosts,
							 void *            user_arg);
							 
static int schedule_user (gw_sch_job_t *    jobs,
						  gw_sch_queue_t ** match_queues,
						  gw_sch_user_t *   users,
						  gw_sch_host_t *   hosts,
						  sched_param_t *   params,
						  int uid);							 

static int reschedule_user (gw_sch_job_t *    jobs,
						  	gw_sch_queue_t ** match_queues,
	   					    gw_sch_user_t *   users,
        					gw_sch_host_t *   hosts,
						    sched_param_t *   params,
						    int uid);

int main(int argc, char **argv)
{
  	char          opt;
	sched_param_t params = {10,5,30};
  	char **       av;
  	int           ac;
  	
	setup_args(argv, &av, &ac);
	
    opterr = 0;
    optind = 1;
	
    while((opt = getopt(ac, av, "h:u:c:")) != -1)
        switch(opt)
        {
            case 'h': params.host_max = atoi(optarg);
                break;

            case 'u': params.user_max = atoi(optarg);
                break;
                
            case 'c': params.user_chunk = atoi(optarg);
                break;                
      	}
     
     free(av);

     gw_scheduler_loop(flood_scheduler, (void *)&params);
     
     return 0;
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

static void flood_scheduler (gw_sch_job_t *    jobs,
							 gw_sch_queue_t ** match_queues,
							 gw_sch_user_t *   users,
							 gw_sch_host_t *   hosts,
							 void *            user_arg)
{
	sched_param_t * params;	
	int             i;
	int             scheds;

	int             end;
	
	params   = (void *) user_arg;
	scheds   = 0;

	end      = 0;
	
	while (!end)
	{
		end = 1;
		
		for ( i=0; users[i].uid!=-1; i++)
		{
			scheds = schedule_user (jobs,
						  			match_queues,
						  			users,
						  			hosts,
						  			params,
						  			users[i].uid);						  			
			if ( scheds > 0 )
				end = 0;
		}
	}

	end = 0;
		
	while (!end)
	{
		end = 1;
		
		for ( i=0; users[i].uid!=-1; i++)
		{
			scheds = reschedule_user (jobs,
						  			match_queues,
						  			users,
						  			hosts,
						  			params,
						  			users[i].uid);						  			
			if ( scheds > 0 )
				end = 0;
		}
	}	
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

static inline int * user_get_dispatched(int uid,gw_sch_user_t * users, int *used)
{
	int i;
	int *disp;
	
	disp  = NULL;
	*used = 0;
		
	for (i=0;users[i].uid!=-1;i++)
		if (users[i].uid == uid)
		{
			disp = &(users[i].dispatched);
			*used = users[i].running_jobs;
			break;
		}
		
	return disp;
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

static inline int * host_get_dispatched(int hid,gw_sch_host_t * hosts, int *used)
{
	int i;
	int *disp;
	
	disp  = NULL;
	*used = 0;
	
	for (i=0;hosts[i].hid!=-1;i++)
		if (hosts[i].hid == hid)
		{
			disp  = &(hosts[i].dispatched);
			*used = hosts[i].used_slots;
			break;
		}
		
	return disp;	
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

static int schedule_user (gw_sch_job_t *    jobs,
						  gw_sch_queue_t ** match_queues,
						  gw_sch_user_t *   users,
						  gw_sch_host_t *   hosts,
						  sched_param_t *   params,
						  int uid)
{
	int *udsp;
	int uusd;
	int max_round;
	int rchunk;
	int i,j;
	int *hdsp;
	int husd;
	int tasks;
	int ruslots;
	int fslots;
	int dsp;
	
	udsp      = user_get_dispatched(uid,users,&uusd);
	max_round = params->user_max - (uusd+*udsp);
	
	if (max_round <= 0)
		return 0;
	else if	( max_round < params->user_chunk)
		rchunk = max_round;
	else
		rchunk = params->user_chunk;
		
	for (i=0,dsp=0; (jobs[i].jid!=-1) && (dsp<rchunk);i++)
	{
		if ((jobs[i].resch == 0) 
			&& (match_queues[i] != NULL)
			&& (jobs[i].uid == uid))
		{
			if ((jobs[i].aid == -1) && (jobs[i].tasks == 1))
			{
				for (j=0;match_queues[i][j].hid != -1; j++)
				{
					hdsp   = host_get_dispatched(match_queues[i][j].hid,hosts,&husd);
					
					if ((husd + *hdsp) >= params->host_max)
						continue;
						
					fslots = match_queues[i][j].slots - *hdsp;
					
					if (fslots > 0)
					{
						*hdsp = *hdsp + 1;	
						*udsp = *udsp + 1;
						dsp   = dsp   + 1;
						jobs[i].tasks = 0;
												
						printf("SCHEDULE_JOB %i SUCCESS %i:%s:%i\n",
							jobs[i].jid,
							match_queues[i][j].hid,
							match_queues[i][j].name,
							match_queues[i][j].rank);
																					
						break;	
					}
				}
			}
			else if (jobs[i].tasks > 0)
			{
				for (j=0;match_queues[i][j].hid != -1; j++)
				{
					hdsp    = host_get_dispatched(match_queues[i][j].hid,hosts,&husd);
					fslots  = params->host_max - husd - *hdsp;
					
					if (((husd + *hdsp) >= params->host_max) || (fslots <= 0))
						continue;
															
					if (fslots > (match_queues[i][j].slots - *hdsp))
						fslots = match_queues[i][j].slots - *hdsp;
					
					ruslots = rchunk - dsp;
					
					if ( fslots < ruslots )
						tasks = fslots;
					else
						tasks = ruslots;
					
					if ( tasks > jobs[i].tasks )
							tasks = jobs[i].tasks;
										
					if ( tasks > 0 )
					{													
						*hdsp = *hdsp + tasks;	
						*udsp = *udsp + tasks;
						dsp   = dsp   + tasks;
						jobs[i].tasks = jobs[i].tasks - tasks;
						
						printf("SCHEDULE_TASKS %i SUCCESS %i:%s:%i:%i\n",
							jobs[i].aid,
							match_queues[i][j].hid,
							match_queues[i][j].name,
							match_queues[i][j].rank,
							tasks);
						
						if (jobs[i].tasks <= 0)
							break;	
					}
				}				
			}
		}
	}
	
	return dsp;

}							 

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

static int reschedule_user (gw_sch_job_t *    jobs,
						  	gw_sch_queue_t ** match_queues,
	   					    gw_sch_user_t *   users,
        					gw_sch_host_t *   hosts,
						    sched_param_t *   params,
						    int uid)
{
	int *udsp;
	int uusd;
	int max_round;
	int rchunk;
	int i,j;
	int *hdsp;
	int husd;
	int rslots;
	int dsp;
	
	udsp      = user_get_dispatched(uid,users,&uusd);
	max_round = params->user_max - (uusd+*udsp);
	
	if (max_round <= 0)
		return 0;
	else if	( max_round < params->user_chunk)
		rchunk = max_round;
	else
		rchunk = params->user_chunk;
		
	for (i=0,dsp=0; (jobs[i].jid!=-1) && (dsp<rchunk);i++)
	{
		if ((jobs[i].resch == 1) 
			&& (match_queues[i] != NULL)
			&& (jobs[i].uid == uid))
		{
			if ((jobs[i].aid == -1) && (jobs[i].tasks == 1))
			{
				for (j=0;match_queues[i][j].hid != -1; j++)
				{
					hdsp   = host_get_dispatched(match_queues[i][j].hid,hosts,&husd);
					
					if ((husd + *hdsp) >= params->host_max)
						continue;
						
					rslots = match_queues[i][j].slots - *hdsp;
					
					if (rslots > 0)
					{
						*hdsp = *hdsp + 1;	
						*udsp = *udsp + 1;
						dsp   = dsp   + 1;
						
						jobs[i].tasks = 0;
												
						printf("SCHEDULE_JOB %i SUCCESS %i:%s:%i\n",
							jobs[i].jid,
							match_queues[i][j].hid,
							match_queues[i][j].name,
							match_queues[i][j].rank);
																					
						break;	
					}
				}
			}
		}
	}

	return dsp;
}					

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

static void setup_args(char **argv, char ***av, int *ac)
{
  char *  arg;
  char *  tk;
  
  if ((argv == NULL) || (argv[0] == NULL) || (argv[1] == NULL))
  {
  	*av = NULL;
  	*ac = 0;
  	return;
  }
  
  *av      = (char **) malloc(sizeof(char *));
  (*av)[0] = strdup(argv[0]);

  arg = strdup(argv[1]);
  tk  = strtok(arg," ");
 
  for(*ac=1; tk != NULL; tk = strtok(NULL," "), (*ac)++)
  {
      *av       = realloc( (void *) (*av), (*ac+1) * sizeof(char *));
      (*av)[*ac] = strdup(tk);
  }

  free(arg);	
}
