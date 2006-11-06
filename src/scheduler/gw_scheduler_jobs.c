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
#include <math.h>

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_scheduler_job_failed(gw_scheduler_t *      sched, 
                             int                   hid, 
                             int                   uid, 
                             gw_migration_reason_t reason)
{
    int                  i;
    gw_sch_user_host_t * hosts;
    gw_sch_user_host_t * th;
    char *               uname;
    
    time_t               the_time;	
	double               t;
	time_t               delta;
	
	char                 str[26];

    /* ----------- Just check, to be used -------------- */

    if (  (reason != GW_REASON_SUSPENSION_TIME)
        &&(reason != GW_REASON_PERFORMANCE    )
        &&(reason != GW_REASON_EXECUTION_ERROR))
    {
    	gw_scheduler_print('E',"Wrong reason in gw_scheduler_job_failed()\n");	
        return;	
    }
    
    /* ------------------------------------------------- */
    
    hosts = NULL;
    th    = NULL;
    
    for (i=0;i<sched->num_users;i++)
       if (sched->users[i].uid == uid )
       {
          hosts = sched->users[i].hosts;
          uname = sched->users[i].name;
          
          break;
       }	
	
	if (hosts == NULL)
	{
		gw_scheduler_print('E',"Job from user %i failed, but it does not exist.\n",uid);
	    return;
	}
	
	for (i=0;i<sched->num_hosts;i++)
	    if (hosts[i].hid == hid)
	    {
	    	th = &(hosts[i]);
	    	break;
	    }
	    
	if (th == NULL)
	{
		gw_scheduler_print('E',"Job failed on host %i, but it does not exist.\n",hid);
	    return;
	}
	
    the_time = time(NULL);
     
    if (th->last_banned == 0)
    {
        th->last_banned = the_time;
        delta           = GW_SCHED_DELTA0;
    }
    else
    {
        t = the_time - th->last_banned;
        
        if ( t <= 10 )
            delta = 0;
        else
        {
        	t     = - t / GW_SCHED_C;
            delta = (time_t) (GW_SCHED_TINF*(1 - exp(t)));
        }
    }

    if ( delta != 0 )
    {
        th->banned = the_time + delta;
    
        gw_scheduler_ctime(th->banned,str);
    
        gw_scheduler_print('W',"Host %i will not be used for user %s until %s\n",
            hid,uname,str);
    }    
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_scheduler_job_success(gw_scheduler_t * sched, 
                              int              hid, 
                              int              uid, 
                              float            txfr,
                              float            tsus,
                              float            texe)
{
    int                  i;
    gw_sch_user_host_t * hosts;
    gw_sch_user_host_t * th;
    char *               uname;    
        
    hosts = NULL;
    th    = NULL;
    
    for (i=0;i<sched->num_users;i++)
       if (sched->users[i].uid == uid )
       {
          hosts = sched->users[i].hosts;
          uname = sched->users[i].name;
                    
          break;
       }	
	
	if (hosts == NULL)
	{
		gw_scheduler_print('E',"Job done from user %i, but it does not exist.\n",uid);
	    return;
	}
	
	for (i=0;i<sched->num_hosts;i++)
	    if (hosts[i].hid == hid)
	    {
	    	th = &(hosts[i]);
	    	break;
	    }
	    
	if (th == NULL)
	{
		gw_scheduler_print('E',"Job done on host %i, but it does not exist.\n",hid);	
	    return;
	}

	if (th->banned != 0)
    {
        gw_scheduler_print('I',"Clearing host %i for user %s\n",th->hid,uname);    		
    }
    	 
    th->last_banned = 0;
    th->banned      = 0;     

    /* Update usage statistics, LOOK HOW TO DO THIS RIGHT (MATHEMATECALLY) */
    /* NOW JUST WEIGTH                                                     */
    
    th->avrg_execution = (0.9 * texe) + (0.1 * th->avrg_execution);
    th->avrg_transfer  = (0.9 * txfr) + (0.1 * th->avrg_transfer);    
    th->avrg_suspension= (0.9 * tsus) + (0.1 * th->avrg_suspension);    
    
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
                                       
static inline gw_boolean_t isinhistory(int                hid,
                                       gw_msg_history_t * history_list, 
                                       int                records)
{
	int i;	
	int found;
	
	found = GW_FALSE;
		
	for (i=0;i<records;i++)	
	{
		if (history_list[i].host_id == hid)
		{
			found = GW_TRUE;
			break;			
		}
	}
	
	return found;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

static int gw_scheduler_filter_select_queues(gw_scheduler_t * sched,
                                             gw_sch_job_t *   job,
                                             gw_msg_match_t * match,    
                                             int              num,
                                             int *            hosts)
{
	int                   i,j,jid,aid;
	
	int                   hid;
    gw_sch_user_host_t *  uhosts;
    	
    gw_msg_history_t *    hlist;
    int                   hnum;
    
    gw_return_code_t      rc;
    gw_boolean_t          addhost, found;
    
    int                   rank, mrank, qnum;
    int                   fnc,  mfnc;
    int                   queues;
	time_t                the_time;
    
  	hlist    = NULL;
    the_time = time(NULL);
    uhosts   = sched->users[job->ua_id].hosts;
    queues   = 0;
    
	for (i=0;i<num;i++)
    {
        addhost = GW_FALSE;        
        hid     = match[i].host_id;
        jid     = job->jid;
        aid     = job->aid;
        
        /* ----------------------------------- */
        /*           Host Filtering            */
        /* ----------------------------------- */        
	    /* 1. (ERROR, SUSP, PERF) banned list  */
	    /* 2. (USER) Hosts in job history      */
	    /* 3. (DISCOVER) Host with low rank    */
	    /* ----------------------------------- */
        
        found = GW_FALSE;
        
        for (j=0;j<sched->num_hosts;j++)
        {
        	if (uhosts[j].hid == hid)
        	{
        		found = GW_TRUE;
        		
        		if (the_time <= uhosts[j].banned)
        		    addhost = GW_FALSE;
        		else
        		    addhost = GW_TRUE;
        		
        		break;
        	}
        }
        
        if ( found == GW_FALSE )
        {
#ifdef GWSCHEDDEBUG        	
            gw_scheduler_print('W',"Host %i is not ready for the scheduler.\n",
                hid, jid);
#endif                             
            hosts[i] = -1;                
        }
        else if ( addhost == GW_FALSE )
        {
#ifdef GWSCHEDDEBUG
            gw_scheduler_print('D',"Host %i will not be used for job/array %i/%i (BANNED)\n",
                hid, jid, aid);
#endif                           
            hosts[i] = -1;
        }
        else if ((job->reason==GW_REASON_USER_REQUESTED)||(job->reason == GW_REASON_SUSPENSION_TIME))
        {
   	        if ( hlist == NULL )
    	    {
    	        rc = gw_client_job_history(jid, &hlist, &hnum);	
    	            
    	        if ( rc != GW_RC_SUCCESS )
    	            continue;
   	    	}
   	    	
    	    found = isinhistory(hid, hlist, hnum);
    	    	
    	    if (found == GW_TRUE)
    	    {
    	    	hosts[i] = -1;
#ifdef GWSCHEDDEBUG
                gw_scheduler_print('D',"Host %i will not be used for job %i/%i (HISTORY_USER/SUSP)\n",
                    hid, jid,aid);
#endif
    	    }        	
        }
        else if ( job->reason == GW_REASON_RESCHEDULING_TIMEOUT )
        {
   	        if ( hlist == NULL )
    	    {
    	        rc = gw_client_job_history(jid, &hlist, &hnum);	
    	            
    	        if ( rc != GW_RC_SUCCESS )
    	            continue;
   	    	}

    	    found = isinhistory(hid, hlist, hnum);
    	    	
    	    if (found == GW_TRUE)
    	    {
    	    	hosts[i] = -1;
#ifdef GWSCHEDDEBUG
                gw_scheduler_print('D',"Host %i will not be used for job %i/%i (HISTORY_RESCHED)\n",
                    hid, jid,aid);
#endif    	    	
    	    }   	    	
        	else
        	{
               /* -------------------------------------------------- */
               /* ! THIS HAS TO BE SCALED, ALSO THE MIGRATION      ! */
	    	   /* ! OVERHEADMUST BE CONSIDERED.                    ! */
               /* -------------------------------------------------- */
        		
	            found = GW_FALSE;
   	            rank  = hlist[0].rank;
                       
                for (j=0; j<match[i].number_of_queues; j++)
                {               
                    if (match[i].rank[j] > rank)
                    {
                    	found = GW_TRUE;
                    	break;
                    }
                }
                
                if (found == GW_FALSE)
                {
                    hosts[i] = -1;        		
#ifdef GWSCHEDDEBUG
                    gw_scheduler_print('D',"Host %i will not be used for job/array %i/%i (RANK)\n",
                        hid, jid, aid);
#endif                    
                }
        	}        	
        }
              		
        /* ----------------------------------- */
        /*           Queue Selection           */
        /* ----------------------------------- */
        /* 1. By queue rank                    */
        /* 2. By free slots in queue           */
        /* ----------------------------------- */
        
        if (hosts[i] != -1)
        {
        	mrank = match[i].rank[0];
        	mfnc  = match[i].slots[0];
        	qnum  = 0;
        
            for (j=1;j<match[i].number_of_queues;j++)
            {
            	rank = match[i].rank[j];
            	fnc  = match[i].slots[j];
            	
            	if ( rank > mrank )
            	{
            	    mrank = match[i].rank[j];
            	    qnum  = j;
            	}
            	else if (rank == mrank)
            	{
            	    if ( fnc > mfnc )
            	    {
            	    	mfnc = fnc;
            	    	qnum = j;
            	    }  
            	}
            }
        	
        	hosts[i] = qnum;
           	queues++;        	
        }
    }
    
    if ( hlist != NULL )
        free(hlist);

    return queues;
}
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

static int queue_cmp(const void *r1, const void *r2)
{
	gw_sch_queue_t * rr1;
	gw_sch_queue_t * rr2;
	
	rr1 = (gw_sch_queue_t *) r1;
	rr2 = (gw_sch_queue_t *) r2;
	
	if ( (rr1->rank + rr1->nice) < (rr2->rank + rr2->nice))
		return 1;
	else if ( (rr1->rank + rr1->nice) > (rr2->rank + rr2->nice))
		return -1;
	else
		return 0;	
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_scheduler_matching_arrays(gw_scheduler_t * sched)
{    
    int              i,j,k,l,idx,tqs,jid,aid;
    
    gw_msg_match_t * match;    
    int              num;
    int *            hosts;
        
    gw_return_code_t rc;
    
    gw_sch_user_host_t * uhosts;   
    
 	gw_client_t *    gw_session;

	gw_session = gw_client_init();
	
	if (gw_session == NULL)
	{
		gw_scheduler_print('E',"Error creating a GW session.\n");
		    		
		return;
	}
	            
    for (i=0;i<sched->num_jobs;i++)
    {
    	match  = NULL;    	
    	jid    = sched->jobs[i].jid;    	
    	aid    = sched->jobs[i].aid;
    	    	
        rc = gw_client_match_job(jid,aid,&match,&num);

	    if ((rc != GW_RC_SUCCESS)||(num == 0))
	    {	    	
   	        if (aid == -1)
   	            gw_scheduler_print('W',"No matching hosts found for job"
   	                " %i - %s\n",jid,gw_ret_code_string(rc));
   	        else
   	            gw_scheduler_print('W',"No matching hosts found for array"
   	                " %i - %s\n",aid,gw_ret_code_string(rc));
   	        
	    	continue;
	    }
    	    
	    hosts = (int *) malloc (sizeof(int) * num);
	    memset((void *)hosts,0,sizeof(int) * num);

        /* ------------------------------- */
        /* Queue Filtering & Selection     */
	    /* ------------------------------- */
	            
        tqs = gw_scheduler_filter_select_queues(sched,
                                               &(sched->jobs[i]),
                                               match,
                                               num,
                                               hosts);

        if (aid == -1)
            gw_scheduler_print('I',"%i hosts match job %i:\n",tqs,jid);
        else
            gw_scheduler_print('I',"%i hosts match array %i:\n",tqs,aid);
        
        /* ------------------------------- */
        /* Build the Matching Hosts Array  */
	    /* ------------------------------- */
	    	    
	    if ( tqs > 0 )
	    {
            sched->jobs[i].num_mhosts = tqs;
	        sched->jobs[i].mhosts     = malloc (tqs * sizeof(gw_sch_queue_t));
	        uhosts                    = sched->users[sched->jobs[i].ua_id].hosts;
	        
	        for (j=0, k=0; j<num; j++)
	        {
                idx = hosts[j];
                
                if ( idx != -1 )
                {
                	for (l=0;l<sched->num_hosts;l++)
                	{
                	    if (uhosts[l].hid == match[j].host_id)
                	    {
                	        sched->jobs[i].mhosts[k].ha_id  = uhosts[l].ha_id;
                	        sched->jobs[i].mhosts[k].uha_id = l;
                	        sched->jobs[i].mhosts[k].nice   = match[j].nice;
                	        
                	        strncpy(sched->jobs[i].mhosts[k].qname,
						            match[j].queue_name[idx],
						            GW_MSG_STRING_SHORT-1);
						                            	        
                	        sched->jobs[i].mhosts[k].rank = match[j].rank[idx];
                	        sched->jobs[i].mhosts[k].slots= match[j].slots[idx];
                	        
                            gw_scheduler_print('I',"\t %-30s: rank = %-5i, slots = %-5i queue = %-15s\n",
                                sched->hosts[uhosts[l].ha_id].name, 
                                match[j].rank[idx],
                                match[j].slots[idx],
                                match[j].queue_name[idx]);

                	    	k++;
                	    	break;
                	    }	
                	} 
                }
	        }
	        
            qsort(sched->jobs[i].mhosts,
                  sched->jobs[i].num_mhosts,
                  sizeof(gw_sch_queue_t),
                  queue_cmp);	        
	    }
	    else if ((tqs == 0)&&(sched->jobs[i].reason != GW_REASON_NONE))
	    {
            gw_scheduler_print('W',"No hosts found to re-schedule job %i\n",
                sched->jobs[i].jid);
                    
            printf("SCHEDULE_JOB %i FAILED reschedule\n",sched->jobs[i].jid);	    	
	    }
	    
	    free(hosts);
	    free(match);
    }	        
    
	gw_client_finalize();
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

static inline void gw_scheduler_job_add_i(gw_scheduler_t *      sched,
                                          int                   jid,
                                          int                   aid,
                                          gw_migration_reason_t reason,
                                          int                   nice,
                                          int                   uid,
                                          int                   tasks)
{
	int k,j;
			   	   
	for (k=0;k<sched->num_users;k++)
	    if (sched->users[k].uid == uid)
		{
            j               = sched->num_jobs;
		    sched->num_jobs = sched->num_jobs + 1;
			   	   
		    sched->jobs = realloc((void *) sched->jobs, 
		                          (sched->num_jobs) * sizeof(gw_sch_job_t));
		                          
		    sched->jobs[j].ua_id  = k;
		    sched->jobs[j].aid    = aid;
		    sched->jobs[j].jid    = jid;
		    sched->jobs[j].tasks  = tasks;
		    sched->jobs[j].reason = reason;
    	 	sched->jobs[j].nice   = nice;

            sched->jobs[j].num_mhosts = 0;
            sched->jobs[j].mhosts     = NULL;             
            
#ifdef GWSCHEDDEBUG
            if ( aid == -1 )
	            gw_scheduler_print('D',"Job Added, ID:%i AID:%i NI:%i RS:%i UID:%i\n",
	                jid,aid,nice,reason,uid);
	        else
	            gw_scheduler_print('D',"Array Added, ID:%i AID:%i TK:%i NI:%i RS:%i UID:%i\n",
	                jid,aid,tasks,nice,reason,uid);	        
#endif
    	}
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_scheduler_job_add(gw_scheduler_t *      sched,
                          int                   jid, 
                          int                   aid, 
                          gw_migration_reason_t reason,
                          int                   nice,
                          int                   uid)
{   
	int          i;
	gw_boolean_t found;
	
    if (( aid    == -1) || 
        ( reason == GW_REASON_USER_REQUESTED ) ||
        ( reason == GW_REASON_RESCHEDULING_TIMEOUT) ||
        ( reason == GW_REASON_SUSPENSION_TIME))
    {
        gw_scheduler_job_add_i(sched, jid, -1, reason, nice, uid, 1);
    }
    else if (reason == GW_REASON_SELF_MIGRATION)
    {
    	gw_scheduler_job_add_i(sched, jid, -1, GW_REASON_NONE, nice, uid, 1);
    }
    else
    {
    	found = GW_FALSE;
    	
    	for (i=0;i<sched->num_jobs;i++)
    	{
    	    if ( sched->jobs[i].aid == aid )
    	    {
    	    	found = GW_TRUE;
    	        sched->jobs[i].tasks++;
    	    }    	
    	}
    	
    	if ( found == GW_FALSE )
    	{
   		    gw_scheduler_job_add_i(sched, -1, aid, GW_REASON_NONE, nice, uid, 1);
    	}
    }	
}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_scheduler_array_add(gw_scheduler_t *      sched,
                            int                   jid, 
                            int                   aid, 
                            gw_migration_reason_t reason,
                            int                   nice,
                            int                   uid,
                            int                   tasks)
{   
	gw_scheduler_job_add_i(sched, jid, aid, GW_REASON_NONE, nice, uid, tasks);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_scheduler_job_del(gw_scheduler_t *      sched,
                          int                   jid)
{   
	int          i;
	gw_boolean_t found;

   	for (i=0, found = GW_FALSE;i<sched->num_jobs;i++)
   	{
   	    if ( found == GW_TRUE )
   	    {
   	        memcpy(&(sched->jobs[i-1]),&(sched->jobs[i]),sizeof(gw_sch_job_t)); 	
   	    }
   	    else if ( sched->jobs[i].jid == jid )
   	    {
    		found = GW_TRUE;
   	    		
    	    if (sched->jobs[i].mhosts != NULL)
    	        free(sched->jobs[i].mhosts);   	    	           	    	    	    	    
   	    }    	
   	}
	
	if (found == GW_TRUE)
	{
	    sched->num_jobs--;
	    
	    if (sched->num_jobs==0)
	    {
	        free(sched->jobs);
	        sched->jobs = NULL;
	    }
	    else
            sched->jobs = realloc((void *)sched->jobs,
                                  (sched->num_jobs) * sizeof(gw_sch_job_t));
	}
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_scheduler_array_del(gw_scheduler_t *      sched,
                            int                   aid,
                            int                   tasks)
{   
	int          i;
	gw_boolean_t found;

   	for (i=0, found = GW_FALSE;i<sched->num_jobs;i++)
   	{
   	    if ( found == GW_TRUE )
   	    {
   	        memcpy(&(sched->jobs[i-1]),&(sched->jobs[i]),sizeof(gw_sch_job_t)); 	
   	    }
   	    else if ( sched->jobs[i].aid == aid )
   	    {
   	    	sched->jobs[i].tasks-=tasks;
   	    	
   	    	if (sched->jobs[i].tasks <= 0)
   	    	{
   	    		found = GW_TRUE;
   	    		
   	    	    if (sched->jobs[i].mhosts != NULL)
   	    	        free(sched->jobs[i].mhosts);   	    	           	    	    	    	    
   	    	}
   	    	else
   	    	    break;
   	    }    	
   	}
	
	if (found == GW_TRUE)
	{
	    sched->num_jobs--;
	    
	    if (sched->num_jobs==0)
	    {
	        free(sched->jobs);
	        sched->jobs = NULL;
	    }
	    else
            sched->jobs = realloc((void *)sched->jobs,
                                  (sched->num_jobs) * sizeof(gw_sch_job_t));
	}
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
