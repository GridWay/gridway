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
#include <float.h>
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
        delta           = 0.1 * sched->sch_conf.fr_max_banned;
    }
    else
    {
        t = the_time - th->last_banned;
        
        if ( t <= 10 )
            delta = 0;
        else
        {
        	t     = - t / sched->sch_conf.fr_banned_c;
            delta = (time_t) (sched->sch_conf.fr_max_banned * (1 - exp(t)));
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

#ifdef GWSCHEDDEBUG 
	if (th->banned != 0)
    {
        gw_scheduler_print('D',"Clearing host %i for user %s\n",th->hid,uname);    		
    }
#endif

    th->last_banned = 0;
    th->banned      = 0;     

    th->last_execution = texe;
    th->last_transfer  = txfr;    
    th->last_suspension= tsus;
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
	int                  i,j,jid;
	
	int                  hid;
    gw_sch_user_host_t * uhosts;
    	
    gw_msg_history_t *   hlist;
    int                  hnum;
    
    gw_return_code_t     rc;
    gw_boolean_t         addhost, found;
    
    int                  rank, mrank, qnum;
    int                  fnc,  mfnc;
    int                  queues;
	time_t               the_time;
    
  	hlist    = NULL;
    the_time = time(NULL);
    uhosts   = sched->users[job->ua_id].hosts;
    queues   = 0;
    
	for (i=0;i<num;i++)
    {
        addhost = GW_FALSE;        
        hid     = match[i].host_id;
        jid     = job->jid;

        /* ----------------------------------- */
        /*           Host Filtering            */
        /* ----------------------------------- */        
	    /* 1. (ERROR, SUSP, PERF) banned list  */
	    /* 2. (USER) Hosts in job history      */
	    /* 3. (DISCOVER) Host with low rank    */
	    /* 4. (ADMIN) Priority policy          */
	    /* ----------------------------------- */

        found = GW_FALSE;

        for (j=0;j<sched->num_hosts;j++)
        {
        	if (uhosts[j].hid == hid)
        	{
        		found = GW_TRUE;
        		
        		if (the_time <= uhosts[j].banned) /* 1. (ERROR, SUSP, PERF) banned list*/
        		    addhost = GW_FALSE;
        		else
        		    addhost = GW_TRUE;
        		
        		break;
        	}
        }
        
        if ( found == GW_FALSE )
        {
#ifdef GWSCHEDDEBUG       
			gw_scheduler_print('D',"Host %i is not ready for the scheduler.\n",	
                hid, jid);
#endif                             
            hosts[i] = -1;                
        }
        else if ( addhost == GW_FALSE )
        {
#ifdef GWSCHEDDEBUG
            gw_scheduler_print('D',"Host %i will not be used for job %i (BANNED)\n",
                hid, jid);
#endif                           
            hosts[i] = -1;
        }
        else if ((job->reason==GW_REASON_USER_REQUESTED)||(job->reason == GW_REASON_SUSPENSION_TIME))
        {
   	        if ( hlist == NULL )
    	    {
    	        rc = gw_client_job_history(jid, &hlist, &hnum);	
    	            
    	        if ( rc != GW_RC_SUCCESS )
    	        {
    	        	hosts[i] = -1;
    	            continue;
    	        }
   	    	}
   	    	
    	    found = isinhistory(hid, hlist, hnum);
    	    	
    	    if (found == GW_TRUE) /* 2. (USER) Hosts in job history      */
    	    {
    	    	hosts[i] = -1;
#ifdef GWSCHEDDEBUG
                gw_scheduler_print('D',"Host %i will not be used for job %i (HISTORY_USER/SUSP)\n",
                    hid, jid);
#endif
    	    }        	
        }
        else if ( job->reason == GW_REASON_RESCHEDULING_TIMEOUT )
        {
   	        if ( hlist == NULL )
    	    {
    	        rc = gw_client_job_history(jid, &hlist, &hnum);	
    	            
    	        if ( rc != GW_RC_SUCCESS )
    	        {
    	        	hosts[i] = -1;
    	            continue;
    	        }
   	    	}

    	    found = isinhistory(hid, hlist, hnum);
    	    	
    	    if (found == GW_TRUE) 
    	    {
    	    	hosts[i] = -1;
#ifdef GWSCHEDDEBUG
                gw_scheduler_print('D',"Host %i will not be used for job %i (HISTORY_RESCHED)\n",
                    hid, jid);
#endif    	    	
    	    }   	    	
        	else /* 3. (DISCOVER) Host with low rank    */
        	{
               /* -------------------------------------------------- */
               /* ! THIS HAS TO BE SCALED, ALSO THE MIGRATION      ! */
	    	   /* ! OVERHEAD MUST BE CONSIDERED.                   ! */
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
                    gw_scheduler_print('D',"Host %i will not be used for job %i (RANK)\n",
                        hid, jid);
#endif                    
                }
        	}        	
        }
		else if (match[i].fixed_priority == 0 ) /* 4. (ADMIN) Priority policy          */
		{
        /* ----------------------------------- */
        /*   Remove banned hosts               */
        /* ----------------------------------- */
            hosts[i] = -1;

#ifdef GWSCHEDDEBUG
            gw_scheduler_print('D',"Host %i will not be used for job %i (ADMIN BANNED)\n",
                        hid, jid);
#endif
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

void gw_scheduler_matching_arrays(gw_scheduler_t * sched)
{    
    int              i,j,k,l,idx,tqs,jid,aid;
    int              last_array;

    gw_msg_match_t *     match;
    gw_sch_user_host_t * uhosts;
        
    int              num;
    int *            hosts;
        
    gw_return_code_t rc;

	match  = NULL;		
	hosts  = NULL;

	last_array  = -1;	
	            
    for (i=0;i<sched->num_jobs;i++)
    {
    	jid = sched->jobs[i].jid;    	
    	aid = sched->jobs[i].aid;
    	
    	if (sched->jobs[i].mhosts != NULL)
    	{
    		free(sched->jobs[i].mhosts);
    		
         	sched->jobs[i].num_mhosts = 0;	
         	sched->jobs[i].mhosts     = NULL;
    	}
    	
    	if (( aid == -1 ) || (last_array != aid) || 
    	    (sched->jobs[i].reason == GW_REASON_SELF_MIGRATION))
    	{
    		if (hosts != NULL)
    		{
    			free(hosts);
    			hosts = NULL;
    		}
    		
    		if (match != NULL)
    		{
    			free(match);
    			match = NULL;
    		}
    		
		    rc = gw_client_match_job(jid,aid,&match,&num);

			if ((rc != GW_RC_SUCCESS)||(num == 0))
			{	    	
		        gw_scheduler_print('W',"No matching hosts found for job"
   	    		    " %i - %s\n",jid,gw_ret_code_string(rc));
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

            /* ------------------------------- */
            /* Build the Matching Hosts Array  */
            /* ------------------------------- */
                
            if ( tqs > 0 )
            {
                sched->jobs[i].num_mhosts = tqs;
                sched->jobs[i].mhosts     = malloc (tqs*sizeof(gw_sch_queue_t));
                
                uhosts  = sched->users[sched->jobs[i].ua_id].hosts;
            
                for (j=0, k=0; j<num; j++)
                {
                    idx = hosts[j];
                
                    if ( idx != -1 )
                    {
                        for (l=0;l<sched->num_hosts;l++)
                        {
                            if (uhosts[l].hid == match[j].host_id)
                            {
                                strncpy(sched->jobs[i].mhosts[k].qname,
                                        match[j].queue_name[idx],
                                        GW_MSG_STRING_SHORT-1);
                            
                                sched->jobs[i].mhosts[k].ha_id  = uhosts[l].ha_id;
                                sched->jobs[i].mhosts[k].uha_id = l;
                                sched->jobs[i].mhosts[k].slots  = match[j].slots[idx];
                                sched->jobs[i].mhosts[k].fixed  = match[j].fixed_priority;
                                sched->jobs[i].mhosts[k].rank   = match[j].rank[idx];
                                sched->jobs[i].mhosts[k].usage  = 0.0;
                                
                                k++;
                                break;
                            }
                        }
                    }
                }
                
                if ( sched->sch_conf.disable == 0 )
                    gw_scheduler_host_policies (sched,i);
                            
            }
            else if ((tqs == 0)&&(sched->jobs[i].reason != GW_REASON_NONE))
            {
                gw_scheduler_print('W',"No hosts found to re-schedule job %i\n",
                    sched->jobs[i].jid);
                    
                printf("SCHEDULE_JOB %i FAILED reschedule\n",sched->jobs[i].jid);
            }
            
            if (sched->jobs[i].reason == GW_REASON_SELF_MIGRATION)
                last_array = -1;
            else
                last_array = aid;
    	} 
    	else if ( last_array == aid ) /* Already made match-making */
    	{
#ifdef GWSCHEDDEBUG            
			gw_scheduler_print('D',"Skipping match-making for array %i\n", aid);
#endif
            sched->jobs[i].num_mhosts = sched->jobs[i-1].num_mhosts;
            sched->jobs[i].mhosts     = malloc (
                    sched->jobs[i-1].num_mhosts * sizeof(gw_sch_queue_t));
  
            for (j=0;j<sched->jobs[i].num_mhosts;j++)
            {
                    memcpy((void *) &(sched->jobs[i].mhosts[j]), 
                           (void *) &(sched->jobs[i-1].mhosts[j]),
                           sizeof(gw_sch_queue_t));
            }
    	}
    }
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_scheduler_job_add(gw_scheduler_t *      sched,
                          int                   jid, 
                          int                   aid,
                          int                   np,
                          gw_migration_reason_t reason,
                          int                   fixed_priority,
                          int                   uid,
                          time_t                deadline)
{   
	int          k,j;
	
	for (k=0;k<sched->num_users;k++)
	    if (sched->users[k].uid == uid)
		{
			sched->users[k].total_jobs++;

            j               = sched->num_jobs;
		    sched->num_jobs = sched->num_jobs + 1;
			   	   
		    sched->jobs = realloc((void *) sched->jobs, 
		                          (sched->num_jobs) * sizeof(gw_sch_job_t));
		                          
		    sched->jobs[j].ua_id  = k;

		    sched->jobs[j].jid    = jid;
		    sched->jobs[j].aid    = aid;

            sched->jobs[j].np    = np;
                    		    
		    sched->jobs[j].reason = reason;

            sched->jobs[j].num_mhosts = 0;
            sched->jobs[j].mhosts     = NULL;
            
            sched->jobs[j].schedule_time = time(NULL);
			sched->jobs[j].deadline_time = deadline;
			
            sched->jobs[j].fixed     = fixed_priority;  
			sched->jobs[j].nfixed    = 0.0;
    
			sched->jobs[j].waiting   = 0;
			sched->jobs[j].nwaiting  = 0.0;
    
			sched->jobs[j].deadline  = 0;
			sched->jobs[j].ndeadline = 0.0;
            
#ifdef GWSCHEDDEBUG
            gw_scheduler_print('D',"Job Added, ID:%i AID:%i FP:%i RS:%i UID:%i\n",
                jid,aid,fixed_priority,reason,uid);
#endif
			break;
    	}
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_scheduler_job_del(gw_scheduler_t *      sched,
                          int                   jid,
                          int                   dispatched)
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
   	    	
    		sched->users[sched->jobs[i].ua_id].total_jobs--;
            
            if ( dispatched == 1 )
    		  sched->users[sched->jobs[i].ua_id].sub_windows[0]++;
    		   	    	
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

static int job_cmp(const void *j1, const void *j2)
{
	gw_sch_job_t * jj1;
	gw_sch_job_t * jj2;
	
	jj1 = (gw_sch_job_t *) j1;
	jj2 = (gw_sch_job_t *) j2;
	
	/* Urgent jobs */
	if ( ( jj1->fixed == 20 ) && ( jj2->fixed != 20 ) )
		return -1;
	else if ( ( jj2->fixed == 20 ) && ( jj1->fixed != 20 ) )
		return  1;
	else if ( ( jj2->fixed == 20 ) && ( jj1->fixed == 20 ) )
		return  0;
	else if ( jj1->priority < jj2->priority)
		return 1;
	else if ( jj1->priority > jj2->priority)
		return -1;
	else
		return 0;	
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

static inline int gw_scheduler_user_past_jobs(gw_sch_user_t *user, int depth)
{
	int i;
	int total = 0;
	int window;

#ifdef GWSCHEDDEBUG
	gw_scheduler_print('D',"WD TJS EJS (user %s).\n",user->name);
#endif 			
	
	for ( i=0; i<depth; i++ )
	{
		window = (int) (user->sub_windows[i] * exp (-((float)i/3.5)));
		total += window;
#ifdef GWSCHEDDEBUG
		gw_scheduler_print('D',"%-2i %-3i %-3i\n",i,
		user->sub_windows[i], window);
#endif 				
	}
		
	return total;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_scheduler_job_policies (gw_scheduler_t * sched)
{
    int i;
    int max_fixed    = 0;
    int max_waiting  = 0;
    int max_deadline = 0;
    int max_share    = 0;
    
    int    total_jobs;
    int    ujid;
    int    share;
    int    past_jobs;
    div_t  q;
    
    time_t the_time;
    time_t Dt_w;
    time_t Dt_d;

    float wfixed;
    float wwaiting;
    float wdeadline;    
    float wshare;
    float C_half;
    
    the_time  = time(NULL);
    C_half    = -1.5 * sched->sch_conf.dl_half * 86400;

    wfixed    = sched->sch_conf.wfixed;
    wwaiting  = sched->sch_conf.wwaiting;
    wdeadline = sched->sch_conf.wdeadline;
    wshare    = sched->sch_conf.wshare;
    
    for (i=0; i<sched->num_jobs; i++)
    {    	
    	Dt_w = the_time - sched->jobs[i].schedule_time;
		sched->jobs[i].waiting  = (int) Dt_w;
        
        if ( sched->jobs[i].deadline_time != 0 )
        {
            Dt_d = sched->jobs[i].deadline_time - the_time;
        
            if (Dt_d <= 0)
                sched->jobs[i].deadline = 100;
            else
                sched->jobs[i].deadline = (int) 100 * exp(Dt_d/C_half);
        }
        else
            sched->jobs[i].deadline = 0;
	
    	if (sched->jobs[i].fixed > max_fixed)
	    	max_fixed = sched->jobs[i].fixed;
	    	
    	if (sched->jobs[i].waiting > max_waiting)
	    	max_waiting = sched->jobs[i].waiting;
	    	
    	if (sched->jobs[i].deadline > max_deadline)
	    	max_deadline = sched->jobs[i].deadline;
    }
    
    for (i=0; i<sched->num_jobs; i++)
    {   	
    	if (max_fixed == 0)
    		sched->jobs[i].nfixed = 0.0;
    	else
    		sched->jobs[i].nfixed = (float) sched->jobs[i].fixed /(float) max_fixed;
    	
    	if (max_waiting == 0)
    		sched->jobs[i].nwaiting = 0.0;
    	else
    		sched->jobs[i].nwaiting = (float)sched->jobs[i].waiting/(float)max_waiting;

		if (max_deadline == 0)
    		sched->jobs[i].ndeadline = 0.0;
    	else
    		sched->jobs[i].ndeadline= (float)sched->jobs[i].deadline/(float)max_deadline;

    	sched->jobs[i].priority = (wfixed    * sched->jobs[i].nfixed)   +
                                  (wwaiting  * sched->jobs[i].nwaiting) +
                                  (wdeadline * sched->jobs[i].ndeadline);
    }
	
	/* Pre-sort jobs according fixed, waiting and deadline policies */
	
	qsort(sched->jobs, sched->num_jobs, sizeof(gw_sch_job_t), job_cmp);

    if ( wshare > 0 )
    {
        for (i=0; i<sched->num_users; i++)
        {
        	sched->users[i].share     = gw_sch_get_user_share(&(sched->sch_conf), 
           	                                  sched->users[i].name);
			past_jobs                 = gw_scheduler_user_past_jobs(&(sched->users[i]),
		                                      sched->sch_conf.window_depth);

        	sched->users[i].next_ujid = past_jobs;
        	total_jobs                = sched->users[i].total_jobs + past_jobs;

        	q = div(total_jobs - 1, sched->users[i].share);
        	
        	if (q.quot > max_share )
        		max_share = q.quot;
        		
#ifdef GWSCHEDDEBUG
			gw_scheduler_print('D',"USER:%s, SHARE:%i, TJOBS:%i, MAX_SHARE:%i\n",
			    sched->users[i].name,sched->users[i].share,total_jobs,max_share);
#endif 			
        }
        
	 	if (max_share == 0)
    		max_share = 1; 
               
	    for (i=0; i<sched->num_jobs; i++)
	    {
	    	ujid  = sched->users[sched->jobs[i].ua_id].next_ujid++;
	    	share = sched->users[sched->jobs[i].ua_id].share;
	    	q     = div(ujid, share);
	    	
	    	sched->jobs[i].share  = max_share - q.quot;
	   		sched->jobs[i].nshare = (float)sched->jobs[i].share/(float)max_share;
   		    	
	    	sched->jobs[i].priority = sched->jobs[i].priority + ( wshare * sched->jobs[i].nshare);
	    }

		qsort(sched->jobs, sched->num_jobs, sizeof(gw_sch_job_t), job_cmp);    
    }

	gw_scheduler_print('I',"-----------------------------------------------------------------\n");
	gw_scheduler_print('I',"                     SCHEDULER JOB LIST\n");
	gw_scheduler_print('I',"-----------------------------------------------------------------\n");
    gw_scheduler_print('I',"%-3s %-9s %-6s %-5s %-6s %-5s %-6s %-5s %-6s %-5s\n",
    	"JID","TOTAL    ","FP    ","NFP  ","SH    ","NSH  ","WT    ","NWT  ","DL    ","NDL  ");

    gw_scheduler_print('I',"%-3s-%-9s-%-6s-%-5s-%-6s-%-5s-%-6s-%-5s-%-6s-%-5s\n",
    	"---","---------","------","-----","------","-----","------","-----","------","-----");
    
	for (i=0; i<sched->num_jobs;i++)
	{
	    gw_scheduler_print('I',"%-3i %-9.2f %-6i %.3f %-6i %.3f %-6i %.3f %-6i %.3f\n",
    	    sched->jobs[i].jid,
    	    sched->jobs[i].priority,
    	    sched->jobs[i].fixed,
    	    sched->jobs[i].nfixed,
    	    sched->jobs[i].share,
    	    sched->jobs[i].nshare,
    	    sched->jobs[i].waiting,
    	    sched->jobs[i].nwaiting,
    	    sched->jobs[i].deadline,
    	    sched->jobs[i].ndeadline);
	}
}
