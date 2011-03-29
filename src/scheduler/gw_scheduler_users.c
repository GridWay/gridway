/* -------------------------------------------------------------------------- */
/* Copyright 2002-2011, GridWay Project Leads (GridWay.org)                   */
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

#ifdef  HAVE_LIBDB
#include "gw_acct.h"
#endif

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_scheduler_add_user(gw_scheduler_t * sched, 
                           int              uid, 
                           int              ajobs, 
                           int              rjobs, 
                           char *           name)
{
	int i,j,depth;
	
#ifdef HAVE_LIBDB
	gw_acct_t    acct;	
	int          rc;
	time_t       from_time;
	gw_boolean_t usedb;
#endif

	depth = sched->sch_conf.window_depth;
	
	/* -------------------------------- */
	/*  Look for this user in the list  */
	/* -------------------------------- */	
	
	for (i=0;i<sched->num_users;i++)
	    if ( sched->users[i].uid == uid )
	    {
#ifdef GWSCHEDDEBUG	    	
	    	gw_scheduler_print('D',"Updating user %i AJOBS:%i, RJOBS:%i TJOBS:%i\n",
	    	    uid,ajobs,rjobs,sched->users[i].total_jobs);
#endif	    	    	    	    
	        sched->users[i].active_jobs  = ajobs;
	        sched->users[i].running_jobs = rjobs;
	        
	        return;
	    }
	    
	/* ----------------------------- */
	/*  Add a new entry to the list  */
	/* ----------------------------- */
		    
	i                = sched->num_users;
	sched->num_users = sched->num_users + 1;

    sched->users = realloc((void *)sched->users,
                           (sched->num_users) * sizeof(gw_sch_user_t));
    
    
    strncpy(sched->users[i].name, name, GW_MSG_STRING_SHORT-1);
    	
	sched->users[i].uid          = uid;
	
	sched->users[i].active_jobs  = ajobs;
	sched->users[i].running_jobs = rjobs;
	sched->users[i].total_jobs   = 0;

    sched->users[i].dispatched = 0;
    sched->users[i].share      = 1;
    sched->users[i].next_ujid  = 0;
	
	if ( depth > 0 )
	{
		sched->users[i].sub_windows = (int *) malloc (sizeof(int)*depth);
	
    	for (j=0; j<depth; j++)    
	    	sched->users[i].sub_windows[j] = 0;
	}
	else
		sched->users[i].sub_windows = NULL;
		
#ifdef GWSCHEDDEBUG	  	    
    gw_scheduler_print('D',"User (%i) %s added, building host list...\n"
        ,uid,name);
#endif        
    
    /* --------------------------------------- */
    /*  Add the list of hosts for this user    */
    /* --------------------------------------- */

    if (sched->num_hosts == 0)
    {
    	sched->users[i].hosts = NULL;
    	return;
    }
    
    sched->users[i].hosts = (gw_sch_user_host_t *)
            malloc (sched->num_hosts * sizeof (gw_sch_user_host_t));

#ifdef HAVE_LIBDB            
	rc = gw_acct_db_open(GW_FALSE);
	
	if ( rc != 0 )
	    usedb = GW_FALSE;
	else
	    usedb = GW_TRUE;
	    
    from_time = time(NULL) - 86400*sched->sch_conf.ug_window;	    
#endif

    for (j=0; j<sched->num_hosts; j++)
    {
    	sched->users[i].hosts[j].ha_id       = j;
    	sched->users[i].hosts[j].hid         = sched->hosts[j].hid;
    	
    	sched->users[i].hosts[j].banned      = 0;
    	sched->users[i].hosts[j].last_banned = 0;
    	
#ifdef HAVE_LIBDB
        if (usedb == GW_TRUE)
        {
    	    rc = gw_acct_join_search(sched->hosts[j].name, 
    	                             sched->users[i].name, 
    	                             &acct, 
    	                             from_time);
        }
        else
            rc = -1;
            
    	if ((rc == 0)&& (acct.tot >= 1))
    	{
    		sched->users[i].hosts[j].avrg_execution  = (float) acct.execution / (float) acct.tot;
    		sched->users[i].hosts[j].avrg_suspension = (float) acct.suspension/ (float) acct.tot;
    		sched->users[i].hosts[j].avrg_transfer   = (float) acct.transfer  / (float) acct.tot;
    	}
    	else
    	{
    	    sched->users[i].hosts[j].avrg_execution  = 0;
    	    sched->users[i].hosts[j].avrg_suspension = 0;
    	    sched->users[i].hosts[j].avrg_transfer   = 0;
    	}
#else
    	sched->users[i].hosts[j].avrg_execution  = 0;
    	sched->users[i].hosts[j].avrg_suspension = 0;
    	sched->users[i].hosts[j].avrg_transfer   = 0;
#endif
    	sched->users[i].hosts[j].last_execution  = 0;
    	sched->users[i].hosts[j].last_suspension = 0;
    	sched->users[i].hosts[j].last_transfer   = 0;

#ifdef GWSCHEDDEBUG	  
        gw_scheduler_print('D',"\t%-30s: avg_xfr = %8.2f  avg_que = %8.2f  avg_exe = %8.2f\n",
	    	               sched->hosts[j].name,
                           sched->users[i].hosts[j].avrg_transfer,
                           sched->users[i].hosts[j].avrg_suspension,
                           sched->users[i].hosts[j].avrg_execution);
#endif
    }

#ifdef HAVE_LIBDB
    if (usedb == GW_TRUE)
        gw_acct_db_close();
#endif
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_scheduler_del_user(gw_scheduler_t * sched, int uid)
{
    int                  i;
    gw_boolean_t         found;

    
    
    if ( sched->num_users == 1 )
    {    	
#ifdef GWSCHEDDEBUG	      	
        gw_scheduler_print('D',"Removing user %s.\n",sched->users[0].name);
#endif        
    	if (sched->users[0].hosts != NULL)
   	        free(sched->users[0].hosts);
   	        
    	free (sched->users);
    	
    	sched->users     = NULL;
    	sched->num_users = 0;
        
        return;
    }
        
    for (i=0, found=GW_FALSE; i<sched->num_users; i++)
    {
    	if (found == GW_TRUE)
    	{
    		memcpy(&(sched->users[i-1]),&(sched->users[i]),sizeof(gw_sch_user_t));    		
    	}
    	else if ( sched->users[i].uid == uid )
    	{
    	    found = GW_TRUE;
#ifdef GWSCHEDDEBUG	      	    
            gw_scheduler_print('D',"Removing user %s.\n",sched->users[i].name);
#endif            
            
    	    if (sched->users[i].sub_windows != NULL)            
    	    	free(sched->users[i].sub_windows);
    	    	
    	    if (sched->users[i].hosts != NULL)
    	        free(sched->users[i].hosts);
    	}
    }
    
    sched->num_users--;
    
    sched->users = realloc((void *)sched->users,
                           (sched->num_users) * sizeof(gw_sch_user_t));
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_scheduler_user_update_windows(gw_scheduler_t * sched)
{
	int i,j,depth;
	
	depth = sched->sch_conf.window_depth;
	
	if (depth == 0)
		return;
		
	for ( i=0; i<sched->num_users; i++ )
	{
		for (j=depth-1; j>0; j--)
			sched->users[i].sub_windows[j] = sched->users[i].sub_windows[j-1];
			
		sched->users[i].sub_windows[0] = 0;
	}
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
