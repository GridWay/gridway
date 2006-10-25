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
	    	gw_scheduler_print('I',"Updating Host %i USLOTS:%i, RJOBS:%i\n",
	    	    hid,uslots,rjobs);

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

   	gw_scheduler_print('I',"Host %s (%i) added, USLOTS:%i, RJOBS:%i\n",
	    	    hostname,hid,uslots,rjobs);
    
    /* --------------------------------------- */
    /*  Add a new entry to the user host list  */
    /* --------------------------------------- */
    
    for (j=0;j<sched->num_users;j++)
    {
        sched->users[j].hosts = realloc((void *) sched->users[j].hosts,
                                (sched->num_hosts)*sizeof(gw_sch_user_host_t));
                                
    	sched->users[j].hosts[i].ha_id           = i;
    	sched->users[j].hosts[i].hid             = hid;
    	
    	sched->users[j].hosts[i].banned          = 0;
    	sched->users[j].hosts[i].last_banned     = 0;
    	
    	/* ------------------------------------ */
    	/*  Get this information from database  */
    	/* ------------------------------------ */
    	
#ifdef HAVE_LIBDB            
	    rc = gw_acct_db_open(GW_FALSE);
	
	    if ( rc != 0 )
	        usedb = GW_FALSE;
    	else
	        usedb = GW_TRUE;
#endif

#ifdef HAVE_LIBDB
        if (usedb == GW_TRUE)
        {
            from_time = time(NULL) - 24*60*60*GW_SCHED_DMEM;
            
    	    rc        = gw_acct_join_search(hostname, 
    	                                sched->users[j].name, 
    	                                &acct, 
    	                                from_time);
        }
        else
            rc = -1;
            
    	if (rc == 0)
    	{
    		sched->users[i].hosts[j].avrg_execution  = (float) acct.execution / (float) acct.tot;
    		sched->users[i].hosts[j].avrg_suspension = (float) acct.suspension/ (float) acct.tot;
    		sched->users[i].hosts[j].avrg_transfer   = (float) acct.transfer  / (float) acct.tot;
    	    sched->users[i].hosts[j].migration_ratio = (1 - (float)acct.succ/(float)acct.tot);    		
    	}
    	else
    	{
    	    sched->users[i].hosts[j].avrg_execution  = 0;
    	    sched->users[i].hosts[j].avrg_suspension = 0;
    	    sched->users[i].hosts[j].avrg_transfer   = 0;

    	    sched->users[i].hosts[j].migration_ratio = 0;
    	}
#else
    	sched->users[i].hosts[j].avrg_execution  = 0;
    	sched->users[i].hosts[j].avrg_suspension = 0;
    	sched->users[i].hosts[j].avrg_transfer   = 0;

    	sched->users[i].hosts[j].migration_ratio = 0;
#endif
    	    	
#ifdef GWSCHEDDEBUG
        gw_scheduler_print('D',"Host %s added for user %s "
                           "(X:%.2f, S:%.2f, E:%.2f, M:%.2f)\n",
	    	               hostname,sched->users[j].name,
                           sched->users[j].hosts[i].avrg_transfer,
                           sched->users[j].hosts[i].avrg_suspension,
                           sched->users[j].hosts[i].avrg_execution,
                           sched->users[j].hosts[i].migration_ratio);
#endif
    }
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
