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
						  int              uid);							 

static int reschedule_user (gw_scheduler_t * sched,
						    sched_param_t *  params,
						    int              uid);

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
						  			sched->users[i].uid);
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
			scheds = reschedule_user(sched,
						  			 params,
						  			sched->users[i].uid);
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
						  int              uid)
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
	int rtasks;
	int fslots;
	int dsp;
	int u_id;
	int h_id;
	int remaining;
	
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
		u_id = sched->jobs[i].ua_id;
			        
		if ((sched->jobs[i].reason == GW_REASON_NONE) 
			&& (sched->jobs[i].mhosts != NULL)
			&& (sched->users[u_id].uid == uid))
		{	
			if ((sched->jobs[i].aid == -1) && (sched->jobs[i].tasks == 1))
			{
#ifdef GWSCHEDDEBUG
                gw_scheduler_print('D',"Scheduling job %i on %i suitable hosts\n",sched->jobs[i].jid,sched->jobs[i].num_mhosts);
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
					if (fslots > 0)
					{
						*hdsp = *hdsp + 1;	
						*udsp = *udsp + 1;
						dsp   = dsp   + 1;
												
						printf("SCHEDULE_JOB %i SUCCESS %i:%s:%i\n",
							sched->jobs[i].jid,
							sched->hosts[h_id].hid,
							sched->jobs[i].mhosts[j].qname,
							sched->jobs[i].mhosts[j].rank);
							
                        gw_scheduler_job_del(sched,sched->jobs[i].jid);
                        
                        i = i - 1; /* Next job will be i, not i+1 */
																					
						break;	
					}
				}
			}
			else if (sched->jobs[i].tasks > 0)
			{
#ifdef GWSCHEDDEBUG
                gw_scheduler_print('D',"Scheduling array %i (%i tasks) on %i suitable hosts\n",
                                   sched->jobs[i].aid,
                                   sched->jobs[i].tasks,
                                   sched->jobs[i].num_mhosts);
#endif                								
				for (j=0;j<sched->jobs[i].num_mhosts; j++)
				{
					h_id = sched->jobs[i].mhosts[j].ha_id;
					
					hdsp = &(sched->hosts[h_id].dispatched);
					husd = sched->hosts[h_id].used_slots;
					
					if ( params->host_max != 0 )
					{
    					fslots  = params->host_max - husd - *hdsp;
    					
    					if (fslots > (sched->jobs[i].mhosts[j].slots - *hdsp))
	    					fslots = sched->jobs[i].mhosts[j].slots - *hdsp;
	    					
    					if ((husd + *hdsp) >= params->host_max)
						    continue;    					
					}
					else
					{
						fslots = sched->jobs[i].mhosts[j].slots - *hdsp;						
					}
                    		
					if (fslots <= 0)
					    continue;
					    					
					ruslots = rchunk - dsp;
					
					if ( fslots < ruslots )
						tasks = fslots;
					else
						tasks = ruslots;
					
					if ( tasks > sched->jobs[i].tasks )
							tasks = sched->jobs[i].tasks;

#ifdef GWSCHEDDEBUG
                gw_scheduler_print('D',"\tHost (%s-%s) FSLOTS: %i  DSP:%i  USD:%i TASKS:%i\n",
                                   sched->hosts[h_id].name,
                                   sched->jobs[i].mhosts[j].qname,
                                   fslots,
                                   *hdsp,
                                   husd,
                                   tasks);
#endif					
					if ( tasks > 0 )
					{													
						*hdsp = *hdsp + tasks;	
						*udsp = *udsp + tasks;
						dsp   = dsp   + tasks;
						
						rtasks = sched->jobs[i].tasks - tasks;
						
						printf("SCHEDULE_TASKS %i SUCCESS %i:%s:%i:%i\n",
							sched->jobs[i].aid,
							sched->hosts[h_id].hid,
							sched->jobs[i].mhosts[j].qname,
							sched->jobs[i].mhosts[j].rank,
							tasks);

    					gw_scheduler_array_del(sched,sched->jobs[i].aid,tasks);
						
						if (rtasks <= 0)
						{
							i = i - 1;					
							break;	
						}
					}
				}				
			}
		}
	}
	
	params->dispached += dsp;
	
	return dsp;

}							 

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

static int reschedule_user (gw_scheduler_t * sched,
						    sched_param_t *  params,
						    int              uid)
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
	int u_id;
	int h_id;
    int remaining;
    
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
		
	for (i=0,dsp=0; (i<sched->num_jobs) && (dsp<rchunk) &&(i>=0);i++)
	{
		u_id = sched->jobs[i].ua_id;
		
		if ((sched->jobs[i].reason != GW_REASON_NONE) 
			&& (sched->jobs[i].mhosts != NULL)
			&& (sched->users[u_id].uid == uid))
		{
			if ((sched->jobs[i].aid == -1) && (sched->jobs[i].tasks == 1))
			{
				for (j=0;j<sched->jobs[i].num_mhosts; j++)
				{
					h_id = sched->jobs[i].mhosts[j].ha_id;
					
					hdsp = &(sched->hosts[h_id].dispatched);
					husd = sched->hosts[h_id].used_slots;
					
					if (params->host_max != 0)
					{
					    if ((husd + *hdsp) >= params->host_max)
						    continue;
					}
						
					rslots = sched->jobs[i].mhosts[j].slots - *hdsp;
					
					if (rslots > 0)
					{
						*hdsp = *hdsp + 1;	
						*udsp = *udsp + 1;
						dsp   = dsp   + 1;
						
						sched->jobs[i].tasks = 0;
												
						printf("SCHEDULE_JOB %i SUCCESS %i:%s:%i\n",
							sched->jobs[i].jid,
							sched->hosts[h_id].hid,
							sched->jobs[i].mhosts[j].qname,
							sched->jobs[i].mhosts[j].rank);

                        gw_scheduler_job_del(sched,sched->jobs[i].jid);
                        
					    i = i - 1; /* Next job will be i, not i+1 */
						break;	
					}
				}
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
