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

void gw_scheduler_add_host(gw_scheduler_t * sched, 
                           int              hid,
                           int              uslots,
                           int              rjobs,
                           char *           hostname)
{
  	int i,j;

#ifdef HAVE_LIBDB
	gw_acct_t    acct;	
	int          rc;
	time_t       from_time;
	gw_boolean_t usedb;
#endif

	/* -------------------------------- */
	/*  Look for this host in the list  */
	/* -------------------------------- */

	for (i=0;i<sched->num_hosts;i++)
	    if (sched->hosts[i].hid == hid)
	    {
#ifdef GWSCHEDDEBUG
	    	gw_scheduler_print('D',"Updating Host %i USLOTS:%i, RJOBS:%i\n",
	    	    hid,uslots,rjobs);
#endif
	    	sched->hosts[i].used_slots   = uslots;
	    	sched->hosts[i].running_jobs = rjobs;
	    	
	        return;	
	    }
	    
	/* ----------------------------- */
	/*  Add a new entry to the list  */
	/* ----------------------------- */
	
	i                = sched->num_hosts;
	sched->num_hosts = sched->num_hosts + 1;
		        
    sched->hosts     = realloc((void *)sched->hosts,
                               (sched->num_hosts) * sizeof(gw_sch_host_t));
		        
    sched->hosts[i].hid          = hid;
    
    sched->hosts[i].running_jobs = rjobs;
    sched->hosts[i].used_slots   = uslots;
    
    sched->hosts[i].dispatched   = 0;

    strncpy(sched->hosts[i].name, hostname, GW_MSG_STRING_SHORT-1);

#ifdef GWSCHEDDEBUG
   	gw_scheduler_print('D',"Host %s (%i) added, updating user information:\n",
	    	    hostname,hid,uslots,rjobs);
#endif
    
    /* --------------------------------------- */
    /*  Add a new entry to the user host list  */
    /* --------------------------------------- */
        	
#ifdef HAVE_LIBDB
   	/* ------------------------------------ */
   	/*  Get usage information from database */
   	/* ------------------------------------ */

	from_time = time(NULL) - 86400 * sched->sch_conf.ug_window;
		
    rc = gw_acct_db_open(GW_FALSE);

    if ( rc != 0 )
        usedb = GW_FALSE;
   	else
        usedb = GW_TRUE;
#endif
    for (j=0;j<sched->num_users;j++)
    {
        sched->users[j].hosts = realloc((void *) sched->users[j].hosts,
                                (sched->num_hosts)*sizeof(gw_sch_user_host_t));
                                
    	sched->users[j].hosts[i].ha_id           = i;
    	sched->users[j].hosts[i].hid             = hid;
    	
    	sched->users[j].hosts[i].banned          = 0;
    	sched->users[j].hosts[i].last_banned     = 0;
    	
#ifdef HAVE_LIBDB
        if (usedb == GW_TRUE)
        {            
    	    rc = gw_acct_join_search(hostname, 
    	                             sched->users[j].name, 
    	                             &acct, 
    	                             from_time);
        }
        else
            rc = -1;
            
    	if ((rc == 0) && (acct.tot >= 1))
    	{
    		sched->users[j].hosts[i].avrg_execution  = (float) acct.execution / (float) acct.tot;
    		sched->users[j].hosts[i].avrg_suspension = (float) acct.suspension/ (float) acct.tot;
    		sched->users[j].hosts[i].avrg_transfer   = (float) acct.transfer  / (float) acct.tot;
    	}
    	else
    	{
    	    sched->users[j].hosts[i].avrg_execution  = 0;
    	    sched->users[j].hosts[i].avrg_suspension = 0;
    	    sched->users[j].hosts[i].avrg_transfer   = 0;
    	}
#else
    	sched->users[j].hosts[i].avrg_execution  = 0;
    	sched->users[j].hosts[i].avrg_suspension = 0;
    	sched->users[j].hosts[i].avrg_transfer   = 0;
#endif
    	sched->users[j].hosts[i].last_execution  = 0;
    	sched->users[j].hosts[i].last_suspension = 0;
    	sched->users[j].hosts[i].last_transfer   = 0;
    	
#ifdef GWSCHEDDEBUG
        gw_scheduler_print('D',"\t%-15s (%-15s): avg_xfr = %8.2f  avg_que = %8.2f  avg_exe = %8.2f\n",
	    	               hostname,sched->users[j].name,
                           sched->users[j].hosts[i].avrg_transfer,
                           sched->users[j].hosts[i].avrg_suspension,
                           sched->users[j].hosts[i].avrg_execution);
#endif                           
    }

#ifdef HAVE_LIBDB    
	gw_acct_db_close();
#endif

}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

#ifdef HAVE_LIBDB

void gw_scheduler_update_usage_host(gw_scheduler_t * sched)
{
	int                  i,j;
	gw_acct_t            acct;	
	int                  rc;
	time_t               from_time;
	gw_sch_user_host_t * host;


    rc = gw_acct_db_open(GW_FALSE);
    
    if ( rc != 0 )
    {
    	gw_scheduler_print('E',"Opening accounting database when updating host usage\n");
    	return;	
    }

    from_time = time(NULL) - (86400 * sched->sch_conf.ug_window);
    	
	for (i=0;i<sched->num_users;i++)
	{
		for (j=0;j<sched->num_hosts;j++)
		{
			host = &(sched->users[i].hosts[j]);
			
			rc   = gw_acct_join_search(sched->hosts[host->ha_id].name,
    	                               sched->users[i].name, 
    	                               &acct, 
    	                               from_time);
    	    
    	    if (( rc == 0 )&& (acct.tot >= 1))
    	    {
    			host->avrg_execution  = (float) acct.execution / (float) acct.tot;
	    		host->avrg_suspension = (float) acct.suspension/ (float) acct.tot;
	    		host->avrg_transfer   = (float) acct.transfer  / (float) acct.tot;
    	    }    	                               
        }
	}

	gw_acct_db_close();
		
	return;
}

#endif

inline float gw_scheduler_host_estimated_time(gw_sch_user_host_t * th,gw_scheduler_t * sched)
{
	float avrg;
	float last;

	avrg = th->avrg_execution + th->avrg_suspension + th->avrg_transfer;
	last = th->last_execution + th->last_suspension + th->last_transfer;
	
	if ( avrg == 0 ) /* No database information available */
	{
		return last;	
	}
	else if (last == 0 ) /* No last execution info available */
	{
		return avrg;
	}
	else
	{
		return ( ((1-sched->sch_conf.ug_ratio)*avrg) + 
		         (sched->sch_conf.ug_ratio * last  )  );
	}
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
    
    if ( rr1->priority < rr2->priority)
        return 1;
    else if ( rr1->priority > rr2->priority)
        return -1;
    else
        return 0;   
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

inline void gw_scheduler_host_policies (gw_scheduler_t * sched, int jid)
{
    int j,u;
    gw_sch_queue_t *     mhosts;
    gw_sch_user_host_t * uhosts;       
    int                  nhosts;

    float wfixed;
    float wrank;
    float wusage;

    int   max_fixed;
    int   max_rank, min_rank;
    float min_usage;
    float max_usage;   
        
    wfixed = sched->sch_conf.wrfixed;
    wrank  = sched->sch_conf.wrank;
    wusage = sched->sch_conf.wusage;
    
    mhosts  = sched->jobs[jid].mhosts;
    nhosts  = sched->jobs[jid].num_mhosts;
    uhosts  = sched->users[sched->jobs[jid].ua_id].hosts;
        
    max_fixed = 0;
    max_rank  = INT_MIN;
    min_rank  = INT_MAX;
    min_usage = FLT_MAX;
    max_usage = 0;
        
    for (j=0; j<nhosts ; j++)
    {
        u = mhosts[j].uha_id;
            
        if ( wusage > 0 )
            mhosts[j].usage = gw_scheduler_host_estimated_time(&(uhosts[u]),sched);
                                        
        if ( mhosts[j].fixed > max_fixed )
            max_fixed = mhosts[j].fixed;

        if ( mhosts[j].rank > max_rank )
            max_rank = mhosts[j].rank;

        if ( mhosts[j].rank < min_rank )
            min_rank = mhosts[j].rank;

        if ((mhosts[j].usage < min_usage ) && (mhosts[j].usage != 0))
            min_usage = mhosts[j].usage;
                
        if ( mhosts[j].usage > max_usage ) 
            max_usage = mhosts[j].usage;            
    }
        
    for (j=0; j<nhosts ; j++)
    {
        mhosts[j].nfixed = (float) mhosts[j].fixed / (float) max_fixed;
                
        mhosts[j].nrank  = 0.0;
            
        if ((mhosts[j].rank > 0) && (max_rank != 0)) 
        {
            mhosts[j].nrank  = (float) mhosts[j].rank /(float) max_rank;   
        }
        else if ((mhosts[j].rank < 0) && (min_rank != 0))
        {   
            mhosts[j].nrank  = - (float) mhosts[j].rank/(float) min_rank;                    
        }

        if ( mhosts[j].usage == 0 )
        {
            if ( max_usage == 0 )
                mhosts[j].nusage = 0;
            else 
                mhosts[j].nusage = min_usage / max_usage;
        }
        else 
            mhosts[j].nusage = min_usage / mhosts[j].usage;
                
                
        mhosts[j].priority = wfixed * mhosts[j].nfixed +
                             wrank  * mhosts[j].nrank  +
                             wusage * mhosts[j].nusage;
    }

    qsort(mhosts,nhosts,sizeof(gw_sch_queue_t),queue_cmp);
                  
    gw_scheduler_print('I',"---------------------------------------------------\n");                
    gw_scheduler_print('I',"               HOST LIST for JOB %i\n",sched->jobs[jid].jid);
    gw_scheduler_print('I',"---------------------------------------------------\n");
    gw_scheduler_print('I',"%-3s %-9s %-6s %-5s %-6s %-5s %9s %5s\n" ,"HID","TOTAL    ","FP    ","NFP  ","RK    ","NRK  ","UG       ","NUG");
    gw_scheduler_print('I',"%-3s-%-9s-%-6s-%-5s-%-6s-%-5s-%9s-%5s\n" ,"---","---------","------","-----","------","-----","---------","-----");
                
    for (j=0; j<nhosts ; j++)
    {
        gw_scheduler_print('I',"%-3i %-9.2f %-6i %.3f %-6i %.3f %-9.3f %.3f\n",
                    sched->hosts[mhosts[j].ha_id].hid,
                    mhosts[j].priority,
                    mhosts[j].fixed,
                    mhosts[j].nfixed,
                    mhosts[j].rank,
                    mhosts[j].nrank,
                    mhosts[j].usage,
                    mhosts[j].nusage);
    }
}

