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

/*
#------------------------------------------------------------------------------
# The flood scheduler. This is a very simple scheduling algorithm. It maximizes
# the number of jobs submitted to the Grid. It's behavior can be modified with
# three parameters:
#    -h   The max number of jobs that the scheduler submits to a given host.
#         Default value is 10, 0 dispatch to each host as many jobs as possible.
#
#    -u   The maximum number of simultaneous RUNNING jobs per user
#         Default value is 30, 0 dispatch as many jobs as possible.
#
#    -c   Scheduling Chunk. Jobs of the same user are submitted in a round-robin
#         fashion with the given chunk. This allows the adiminstration to
#         stablish a basic sharing policy.  Default value is 5
#
#    -s   Dispatch Chunk. The maximum number of jobs that will be dispatched each 
#         scheduling action. 
#         Default value is 15, 0 dispatch as many jobs as possible.
#------------------------------------------------------------------------------
*/

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
	int sched_max;
	
	int dispached;
} sched_param_t;

/* ------------------------------------------------------------------------- */
/* FUNCTION DEFINICION                                                       */
/* ------------------------------------------------------------------------- */

static void setup_args(char **argv, char ***av, int *ac);

static void flood_scheduler (gw_scheduler_t * sched,
							 void *           user_arg);
							 
static int schedule_user (gw_scheduler_t * sched,
						  sched_param_t *  params,
						  int              uid,
						  gw_boolean_t     resched);

int main(int argc, char **argv)
{
  	char          opt;
	sched_param_t params = {10,5,30,15,0};
  	char **       av;
  	int           ac;
  	
	setup_args(argv, &av, &ac);
	
    opterr = 0;
    optind = 1;
	
    while((opt = getopt(ac, av, "h:u:c:s:")) != -1)
        switch(opt)
        {
            case 'h': params.host_max = atoi(optarg);
                break;

            case 'u': params.user_max = atoi(optarg);
                break;
                
            case 'c': params.user_chunk = atoi(optarg);
                break;                
                
            case 's': params.sched_max = atoi(optarg);
                break;                
                
      	}
     
     free(av);

     gw_scheduler_loop(flood_scheduler, (void *)&params);
     
     return 0;
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

static void flood_scheduler (gw_scheduler_t * sched,
							 void *           user_arg)
{
	sched_param_t * params;	
	int             i;
	int             scheds;

	int             end;
	
	params = (void *) user_arg;
	scheds = 0;	
	end    = 0;
	
	params->dispached = 0;
	
	while (!end)
	{
		end = 1;
		
		for ( i=0; i<sched->num_users; i++)
		{
			scheds = schedule_user (sched,
						  			params,
						  			sched->users[i].uid,
						  			GW_FALSE);
			if ( scheds > 0 )
				end = 0;
		}
	}
	
	end = 0;
		
	while (!end)
	{
		end = 1;
		
		for ( i=0; i<sched->num_users; i++)
		{
			scheds = schedule_user(sched,
						  		   params,
						  		   sched->users[i].uid,
						  		   GW_TRUE);
			if ( scheds > 0 )
				end = 0;
		}
	}	
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

static inline int * user_get_dispatched(int              uid, 
                                        gw_scheduler_t * sched, 
                                        int *            used)
{
	int i;
	int *disp;
	
	disp  = NULL;
	*used = 0;
		
	for (i=0;sched->num_users;i++)
		if (sched->users[i].uid == uid)
		{
			disp = &(sched->users[i].dispatched);
			*used = sched->users[i].running_jobs;
			break;
		}
		
	return disp;
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

static int schedule_user (gw_scheduler_t * sched,
						  sched_param_t *  params,
						  int              uid,
						  gw_boolean_t     reschedule)
{
	int *udsp;
	int uusd;
	int max_round;
	int rchunk;
	int i,j;
	int *hdsp;
	int husd;
	int fslots;
	int dsp;
	int u_id;
	int h_id;
	int remaining;
    int not_schedule;
    gw_migration_reason_t reason;
    	
	udsp = user_get_dispatched(uid,sched,&uusd);
	
	if ( params->sched_max == 0 )
	    remaining = sched->num_jobs;
	else
	    remaining = params->sched_max - params->dispached;
		
	if (params->user_max == 0)
        max_round = sched->num_jobs;
	else
	    max_round = params->user_max  - (uusd+*udsp);

	if (max_round <= 0)
		return 0;
	else if ( remaining <= 0)
	    return 0;
	else if	( max_round < params->user_chunk)
		rchunk = max_round;
	else
		rchunk = params->user_chunk;
	
	if (remaining < rchunk)
	    rchunk = remaining;
			
	for (i=0,dsp=0; i<sched->num_jobs && (dsp<rchunk) && (i>=0);i++)
	{
		u_id   = sched->jobs[i].ua_id;
		reason = sched->jobs[i].reason;
		
		not_schedule = ((reschedule == GW_FALSE) && (reason != GW_REASON_NONE)) ||
                	   ((reschedule == GW_TRUE) && (reason == GW_REASON_NONE)) ||
		               (sched->jobs[i].mhosts == NULL) ||
        		       (sched->users[u_id].uid != uid);        		          
			        
		if ( not_schedule )
			continue;
			
#ifdef GWSCHEDDEBUG
        gw_scheduler_print('D',"Scheduling job %i on %i suitable hosts\n",
        	sched->jobs[i].jid,sched->jobs[i].num_mhosts);
#endif  
              								
		for (j=0;j<sched->jobs[i].num_mhosts; j++)
		{
			h_id = sched->jobs[i].mhosts[j].ha_id;
					
			hdsp = &(sched->hosts[h_id].dispatched);
			husd = sched->hosts[h_id].used_slots;
					
			if (params->host_max != 0 )
			{
			    if ((husd + *hdsp) >= params->host_max)
				    continue;
			}
						
			fslots = sched->jobs[i].mhosts[j].slots - *hdsp;
				
#ifdef GWSCHEDDEBUG
            gw_scheduler_print('D',"\tHost (%s-%s) FSLOTS: %i  DSP:%i  USD:%i\n",
                                   sched->hosts[h_id].name,
                                   sched->jobs[i].mhosts[j].qname,
                                   fslots,
                                   *hdsp,
                                   husd);
#endif										
			if (fslots >= sched->jobs[i].np)
			{
				*hdsp = *hdsp + 1;	
				*udsp = *udsp + 1;
				dsp   = dsp   + 1;
											
				printf("SCHEDULE_JOB %i SUCCESS %i:%s:%i\n",
							sched->jobs[i].jid,
							sched->hosts[h_id].hid,
							sched->jobs[i].mhosts[j].qname,
							sched->jobs[i].mhosts[j].rank);

                gw_scheduler_print('I',"Job %i scheduled, host: %-30s queue: %-15s\n",
							sched->jobs[i].jid,
							sched->hosts[h_id].name,
							sched->jobs[i].mhosts[j].qname);
                                       							
                gw_scheduler_job_del(sched,sched->jobs[i].jid,1);
                        
                i = i - 1; /* Next job will be i, not i+1 */
																					
				break;	
			 }
		}
	}

	params->dispached += dsp;
	
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
