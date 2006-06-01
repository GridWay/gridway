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


#include "gw_client.h"
#include "gw_scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

static int check_history_records(int hid, int rank, int jid);

static int resource_cmp(const void *r1, const void *r2);
                                
static int gw_scheduler_set_up(gw_sch_job_t **    sched_jobs,
                               gw_sch_queue_t *** match_queue,
  						       gw_sch_user_t **   users,
  						       gw_sch_host_t **   hosts);
                                
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */



void gw_scheduler_loop(gw_scheduler_function_t scheduler, void *user_arg)
{
	int     end = 0;
    fd_set  in_pipes;
    int     rc,j,i;
    char    c;
    char    str[80];
    char    action[10];
	char    opt1[5];
    char    opt2[5];
    char    opt3[5];	

	gw_sch_job_t *    sched_jobs;
    gw_sch_queue_t ** match_queues;
  	gw_sch_user_t *   users;
  	gw_sch_host_t *   hosts;
  	
  	setbuf(stdout,NULL);	
	
    while (!end)
    {
        FD_ZERO(&in_pipes);
        FD_SET (0,&in_pipes);                         

        rc = select(1, &in_pipes, NULL, NULL, NULL);
        if (rc == -1)
            end = 1;
                               
        j = 0;

        do
        {
            rc = read(0, (void *) &c, sizeof(char));
            str[j++] = c;    
        }while ( rc > 0 && c != '\n' );

        str[j] = '\0';    

        if (rc <= 0)
        {
            end = 1;
        }

        rc = sscanf(str,"%s %s %s %[^\n]",action,opt1,opt2,opt3);
        
        if (strcmp(action, "INIT") == 0 )
        {
        	printf("INIT - SUCCESS -\n");
        }
        else if (strcmp(action, "FINALIZE") == 0 )
        {
        	printf("FINALIZE - SUCCESS -");
        	end = 1;
        }
        else if (strcmp(action, "SCHEDULE") == 0 )
        {
        	rc = gw_scheduler_set_up (&sched_jobs, &match_queues, &users, &hosts);
        	
        	if ( rc == 0)
        	{
        		scheduler(sched_jobs,match_queues,users,hosts,user_arg);
        		
        		printf("SCHEDULE_END - SUCCESS -\n");
        		
        		for (i=0; sched_jobs[i].jid != -1; i++)
        			free(match_queues[i]);
        		
        		free(match_queues);
        		free(sched_jobs);
        		free(users);
        		free(hosts);
        	}
        }                               
    }	
	
}



/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

static int gw_scheduler_set_up(gw_sch_job_t **    sched_jobs,
                               gw_sch_queue_t *** match_queue,
  						       gw_sch_user_t **   users,
  						       gw_sch_host_t **   hosts)
{
	gw_client_t *     gw_session;
	gw_return_code_t  rc;
	
	gw_job_state_t    state;
	int               array_id;
	int               re_sched;
	int               schedule;
	int               notresched;
	int               last_array;
	int               i,j,k,l;
	int               jid;

	gw_msg_match_t *  host;
	int               queues;
	int               add_queue;
	
	gw_msg_user_t *   user_status;
	int               num_users;

	gw_msg_match_t *  match_list = NULL;
	int               num_records;
		
	/* ---------------------------------------------------------------- */
	/* Connect to GWD                                                   */
	/* ---------------------------------------------------------------- */

	gw_session = gw_client_init();
	
	if ( gw_session == NULL )
		return -1;
	
	rc = gw_client_job_status_all();

	if ( rc != GW_RC_SUCCESS)
    {
        gw_client_finalize();
        
		printf("SCHEDULE_END - FAILED gw_client_job_status_all()\n");
        return -1;
    }  
    
	rc = gw_client_host_status_all();

	if ( rc != GW_RC_SUCCESS)
    {
        gw_client_finalize();
        
		printf("SCHEDULE_END - FAILED gw_client_host_status_all()\n");
        return -1;
    }      
    
	rc = gw_client_user_status(&user_status, &num_users);
	
	if ( rc != GW_RC_SUCCESS)
	{
        gw_client_finalize();
        
		printf("SCHEDULE_END - FAILED gw_client_user_status()\n");
        return -1;		
	}
		
    /* ---------------------------------------------------------------- */
	/* Build the users array                                            */
	/* ---------------------------------------------------------------- */	
	
	if ( num_users == 0 )
	{
		gw_client_finalize();
	
		printf("SCHEDULE_END - FAILED No users in the system\n");
		return -1;
	}
	
	(*users) = (gw_sch_user_t *) malloc(sizeof(gw_sch_user_t)*(num_users+1));
	
	for (i=0;i<num_users;i++)
	{
		(*users)[i].uid          = user_status[i].user_id;
		(*users)[i].active_jobs  = user_status[i].active_jobs;
		(*users)[i].running_jobs = user_status[i].running_jobs;
		(*users)[i].dispatched   = 0;
		
		strncpy((*users)[i].name,user_status[i].name,GW_MSG_STRING_SHORT-1);
	}

	(*users)[num_users].uid = -1;
	
	free(user_status);

    /* ---------------------------------------------------------------- */
	/* Build the host array                                             */
	/* ---------------------------------------------------------------- */
	
	(*hosts) = NULL;
	j = 0;
	
	for (i=0;i<gw_session->number_of_hosts;i++)
	{
		if (gw_session->host_pool[i] != NULL)
		{
			(*hosts) = realloc((void *)(*hosts),(j+1)*sizeof(gw_sch_host_t));
			
			(*hosts)[j].hid          = gw_session->host_pool[i]->host_id;
			(*hosts)[j].running_jobs = gw_session->host_pool[i]->running_jobs;
			(*hosts)[j].used_slots   = gw_session->host_pool[i]->used_slots;
			(*hosts)[j].dispatched   = 0;
			
			strncpy((*hosts)[j++].name,
			        gw_session->host_pool[i]->hostname,
			        GW_MSG_STRING_SHORT-1);			
		}
	}
		
	if ( j == 0 )
	{
		gw_client_finalize();
		
		free(*users);
		
		printf("SCHEDULE_END - FAILED No hosts discovered\n");
		return -1;
	}
	
	(*hosts) = realloc((void *)(*hosts),(j+1)*sizeof(gw_sch_host_t));	
	(*hosts)[j].hid = -1;
    
    /* ---------------------------------------------------------------- */
	/* Build the jobs array (those that need to be scheduled            */
	/* ---------------------------------------------------------------- */
	
	*sched_jobs = NULL;
	j = 0;
	last_array  = -1;
	
	for (i=0; i<gw_session->number_of_jobs ; i++)
	{
		if (gw_session->job_pool[i] != NULL)
		{			
			array_id = gw_session->job_pool[i]->array_id;
			state    = gw_session->job_pool[i]->job_state;
			re_sched = gw_session->job_pool[i]->reschedule;
			
			schedule   = (state == GW_JOB_STATE_PENDING)||(re_sched == 1);
			notresched = (state == GW_JOB_STATE_PENDING)&&(re_sched == 1);
			
			if (schedule)
			{	
				if ((array_id == -1) || (re_sched == 1))/* a job, or resched task/job */
				{
					(*sched_jobs) = realloc((void *)(*sched_jobs),(j+1)*sizeof(gw_sch_job_t));
					
					(*sched_jobs)[j].jid   = gw_session->job_pool[i]->id;
					(*sched_jobs)[j].tasks =  1;	
					(*sched_jobs)[j].aid   = -1;
					(*sched_jobs)[j].uid   = gw_session->job_pool[i]->uid;

					if (notresched) /*pending jobs will be scheduled*/
						(*sched_jobs)[j++].resch = -1;
					else
						(*sched_jobs)[j++].resch = re_sched;
				}
				else
				{					
					if (array_id != last_array) /* a new array */
					{
						(*sched_jobs) = realloc((void *)(*sched_jobs),(j+1)*sizeof(gw_sch_job_t));
						
						(*sched_jobs)[j].jid     = gw_session->job_pool[i]->id;
						(*sched_jobs)[j].tasks   = 1;	
						(*sched_jobs)[j].uid     = gw_session->job_pool[i]->uid;				
						(*sched_jobs)[j].aid     = array_id;
						(*sched_jobs)[j++].resch = 0;	
							
						last_array = array_id;
					}
					else /* not a new entry */
					{
						for (k=j-1;((*sched_jobs)[k].aid != array_id) && (k>=0);k--)
						{};
						
						if (k>=0)
							(*sched_jobs)[k].tasks++;						
					}
				}
			}
		}
	}

	if ( j == 0 )
	{
		gw_client_finalize();
		
		free(*users);
		free(*hosts);
		
		printf("SCHEDULE_END - FAILED No jobs to schedule\n");
		return -1;
	}

	(*sched_jobs) = realloc((void *)(*sched_jobs),(j+1)*sizeof(gw_sch_job_t));
	(*sched_jobs)[j].jid = -1; 
	
    /* ---------------------------------------------------------------- */
	/* Build the matching matrix                                        */
	/* ---------------------------------------------------------------- */	
	
	*match_queue = (gw_sch_queue_t **) malloc(sizeof(gw_sch_queue_t*)*(j));
	
	for (i=0; (*sched_jobs)[i].jid != -1; i++)
	{		
		k      = 0;
		jid    = (*sched_jobs)[i].jid;
		queues = 0;

		(*match_queue)[i] = NULL;
		
		rc = gw_client_match_job(jid, &match_list, &num_records);
				
		if ((rc != GW_RC_SUCCESS)||(num_records==0))
	    {	    	
	    	continue;
	    }  
		
		for (j=0; j< num_records;j++)
		{
			host = &(match_list[j]);
			
			for (l=0;l<host->number_of_queues;l++)
			{
				if (host->match[l] == GW_TRUE)
				{
					if 	((*sched_jobs)[i].resch != 0)
						add_queue = check_history_records(host->host_id,
						                                  host->rank[l],
						                                  jid);
					else
						add_queue = 1;
						
					if (add_queue)
					{
						(*match_queue)[i] = realloc((*match_queue)[i],
						 	                (queues+1)*sizeof(gw_sch_queue_t));
						 	                
						(*match_queue)[i][queues].hid   = host->host_id;
						(*match_queue)[i][queues].slots = host->slots[l];				 
						(*match_queue)[i][queues].rank  = host->rank[l];
						
						strncpy((*match_queue)[i][queues++].name,
						        host->queue_name[l],
						        GW_MSG_STRING_SHORT-1);
					}
				}	
			}
		}
		
		if ((*sched_jobs)[i].resch == -1)
			(*sched_jobs)[i].resch = 0;
				
		(*match_queue)[i] = realloc((void *)(*match_queue)[i],(queues+1)*sizeof(gw_sch_queue_t));			
		(*match_queue)[i][queues].hid   = -1;
		
		if (queues > 0)
			qsort((*match_queue)[i], queues, sizeof(gw_sch_queue_t), resource_cmp);

		if (match_list!= NULL)
			free(match_list);
				
		match_list = NULL;			
	}	
	
	gw_client_finalize();
	
	return 0;
}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

static inline int isinhistory(int hid, gw_msg_history_t * history_list, int records)
{
	int i;	
	int found;
	
	found = 0;
		
	for (i=0;i<records;i++)	
	{
		if (history_list[i].host_id == hid)
		{
			found = 1;
			break;			
		}
	}
	
	return found;
}

static int check_history_records(int hid, int rank, int jid)
{
	int                add_queue;
	gw_return_code_t   grc;
	gw_msg_history_t * history_list;
	int                records;
	
    grc = gw_client_job_history(jid, &history_list, &records);
						 
    if (grc == GW_RC_SUCCESS)
	{
		switch (history_list[0].reason)
		{
			case GW_REASON_USER_REQUESTED:
			case GW_REASON_SUSPENSION_TIME:					 		
			case GW_REASON_SELF_MIGRATION:
			case GW_REASON_PERFORMANCE:
			case GW_REASON_EXECUTION_ERROR:
				
				if (isinhistory(hid, history_list,records))
					add_queue = 0;
				else
					add_queue = 1;				
 			break;
						 			
			case GW_REASON_RESCHEDULING_TIMEOUT: /*SCALE THIS!!*/

				if (isinhistory(hid, history_list,records))
					add_queue = 0;
				else
				{
					if (rank > history_list[0].rank )
						add_queue = 1;
					else
						add_queue = 0;
				}
									
			break;
						 			
	 		case GW_REASON_STOP_RESUME:
	 		case GW_REASON_KILL:
	 			add_queue = 1;
 			break;	
						 		
	 		default:
				add_queue = 0;
	 		break;
		}
	}
	else
		add_queue = 0;
		
	free(history_list);
			
	return add_queue;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

static int resource_cmp(const void *r1, const void *r2)
{
	gw_sch_queue_t * rr1;
	gw_sch_queue_t * rr2;
	
	rr1 = (gw_sch_queue_t *) r1;
	rr2 = (gw_sch_queue_t *) r2;
	
	if ( rr1->rank < rr2->rank)
		return 1;
	else if ( rr1->rank > rr2->rank)
		return -1;
	else
		return 0;	
}
