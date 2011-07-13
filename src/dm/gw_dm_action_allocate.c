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

#include "gw_dm.h"
#include "gw_rm_msg.h"
#include "gw_log.h"
#include "gw_user_pool.h"

#include <pthread.h>
#include <stdlib.h>

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_dm_jalloc(void *_msg)
{
    gw_msg_submit_t * msg_submit;
    gw_job_t *   job;
    int          jid;
    int          uid;
    int          rc;
    
    gw_boolean_t   useradd;    
    gw_job_state_t init_state;

    msg_submit  = (gw_msg_submit_t *) _msg;

    /* ------------- Check if user is already registered ------------ */
	    
    useradd = gw_user_pool_exists (msg_submit->msg.owner, msg_submit->msg.proxy_path, &uid) == GW_FALSE;
	
    if (useradd)
    {
    	rc = gw_user_pool_user_allocate (msg_submit->msg.owner, msg_submit->msg.proxy_path, &uid);
 
#ifdef GWDMDEBUG
        gw_log_print("DM",'D',"User %s registered with UID %i.\n", msg_submit->msg.owner, uid);
#endif

    	if ( rc != 0 )
    	{
            gw_log_print("DM",'E',"Could not register user %s.\n", msg_submit->msg.owner);
        
            msg_submit->msg.rc = GW_RC_FAILED_USER;        
            gw_am_trigger(gw_dm.rm_am,"GW_RM_SUBMIT",&(msg_submit->msg));        
            return;
    	}
    }

    /* ------------- Allocate job structure ------------ */

    jid = gw_job_pool_allocate();
    
    if ( jid == -1 )
    {
        gw_log_print("DM",'E',"Could not allocate job.\n");
        
        msg_submit->msg.rc = GW_RC_FAILED;        
        gw_am_trigger(gw_dm.rm_am,"GW_RM_SUBMIT",&(msg_submit->msg));        
        return;
    }
    
    /* ------------------------------------------ */
    /*               Update job data              */
    /* ------------------------------------------ */
		
    job = gw_job_pool_get(jid, GW_TRUE);
    
    if ( job == NULL )
    {
       	msg_submit->msg.rc = GW_RC_FAILED;
        gw_am_trigger(gw_dm.rm_am,"GW_RM_SUBMIT",&(msg_submit->msg));        
    	
      	return;
    }  
    
    /* ------ Fill data using the template ------- */
    
    rc = gw_job_fill(job, msg_submit);

    if ( rc == -1 )
    {
        gw_log_print("DM",'E',"Could not initialize job.\n");

        pthread_mutex_unlock(&(job->mutex));
        gw_job_pool_free(jid);
        
        msg_submit->msg.rc = GW_RC_FAILED;
        gw_am_trigger(gw_dm.rm_am,"GW_RM_SUBMIT",&(msg_submit->msg));
        return;
    }
    
    /* --------- Set the initial state ---------- */
    
    init_state = msg_submit->msg.init_state;
 		 
    if ( init_state == GW_JOB_STATE_PENDING )
        gw_job_set_state(job, GW_JOB_STATE_PENDING, GW_FALSE);
    else
        gw_job_set_state(job, GW_JOB_STATE_HOLD, GW_FALSE);
	    
    job->tm_state = GW_TM_STATE_INIT;
    job->em_state = GW_EM_STATE_INIT;
    job->user_id  = uid;
    
    /* --------- Set the initial priority ---------- */
	
    job->fixed_priority = msg_submit->msg.fixed_priority;
	
    pthread_mutex_unlock(&(job->mutex));

    /* ------------- Set job dependencies ------------ */ 	   
    
    if ( msg_submit->jt.job_deps[0] != -1 )
    	gw_job_pool_dep_set(jid, msg_submit->jt.job_deps); 	   

    if (!useradd)
        gw_user_pool_inc_jobs(uid,1);
	
    /* ------------- Callback msg ------------ */
	
    msg_submit->msg.rc       = GW_RC_SUCCESS;
    msg_submit->msg.array_id = -1;    
    msg_submit->msg.job_id   = jid;
    
    gw_am_trigger(gw_dm.rm_am,"GW_RM_SUBMIT",&(msg_submit->msg));
    
    /* ------------- Notify the scheduler ------------ */
    
    if ( init_state == GW_JOB_STATE_PENDING )
        gw_dm_mad_job_schedule(&gw_dm.dm_mad[0], jid, -1, uid,
                GW_REASON_NONE);

    gw_log_print("DM",'I',"New job %i allocated and initialized.\n", jid);
}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_dm_aalloc     (void *_msg)
{
    gw_msg_submit_t  *msg_submit;
    gw_array_t       *array;
    gw_job_t         *job;
    gw_boolean_t     useradd;
    gw_job_state_t   init_state;
    int              fixed;
    int              tasks;
    
    int  rc;
    int  array_id, i, jid, uid;
    
    msg_submit  = (gw_msg_submit_t *) _msg;

    useradd = gw_user_pool_exists (msg_submit->msg.owner, msg_submit->msg.proxy_path, &uid) == GW_FALSE;
	
    if (useradd)
    {
    	rc = gw_user_pool_user_allocate (msg_submit->msg.owner, msg_submit->msg.proxy_path, &uid);
    	
    	if ( rc != 0 )
    	{
	        gw_log_print("DM",'E',"Could not register user %s.\n", msg_submit->msg.owner);
        
        	msg_submit->msg.rc = GW_RC_FAILED_USER;        
        	gw_am_trigger(gw_dm.rm_am,"GW_RM_SUBMIT",&(msg_submit->msg));        
        	return;
    	}	
    }
    
    rc = gw_array_pool_array_allocate(&(msg_submit->msg), msg_submit->msg.number_of_tasks, &array_id);
        
    if ( rc != 0 )
    {
      gw_log_print("DM",'E',"Could not allocate array.\n");
                
      msg_submit->msg.rc = GW_RC_FAILED;        
      gw_am_trigger(gw_dm.rm_am,"GW_RM_SUBMIT",&(msg_submit->msg));
                
      return;
    }

    array = gw_array_pool_get_array(array_id, GW_TRUE);

    if ( array == NULL )
    {
        msg_submit->msg.rc = GW_RC_FAILED;
        gw_am_trigger(gw_dm.rm_am,"GW_RM_SUBMIT",&(msg_submit->msg));
        
        return;
    }
    
    init_state = msg_submit->msg.init_state;
    tasks      = msg_submit->msg.number_of_tasks;
    
    for (i=0; i<msg_submit->msg.number_of_tasks; i++)
    {
        jid   = array->job_ids[i];
        job   = gw_job_pool_get(jid, GW_TRUE);
        
       	fixed = msg_submit->msg.fixed_priority;
       	
        gw_job_fill(job, msg_submit);
        
        if ( job == NULL )
        {
            pthread_mutex_unlock(&(array->mutex));        
			
            msg_submit->msg.rc = GW_RC_FAILED;
            gw_am_trigger(gw_dm.rm_am,"GW_RM_SUBMIT",&(msg_submit->msg));
		    
            return;
        }        
    
    	/* --------- Set the initial state ---------- */
    
        if ( init_state == GW_JOB_STATE_PENDING )
            gw_job_set_state(job, GW_JOB_STATE_PENDING, GW_FALSE);
        else
            gw_job_set_state(job, GW_JOB_STATE_HOLD, GW_FALSE);
	    
        job->tm_state = GW_TM_STATE_INIT;
        job->em_state = GW_EM_STATE_INIT;        
        job->user_id  = uid;

        /* --------- Set the parameter values ---------- */
		
        job->pstart   = msg_submit->msg.pstart;
        job->pinc     = msg_submit->msg.pinc;
       
        /* --------- Set the initial priority ---------- */
    
        job->fixed_priority = msg_submit->msg.fixed_priority;
        
        pthread_mutex_unlock (&(job->mutex));    	
  
        /* ------------- Notify the scheduler ------------ */
    
        if ( init_state == GW_JOB_STATE_PENDING )
            gw_dm_mad_job_schedule(&gw_dm.dm_mad[0], jid, array_id,
                    uid, GW_REASON_NONE);
                      
        /* ------------- Set job dependencies ------------ */
        
        if ( msg_submit->jt.job_deps[0] != -1 )
            gw_job_pool_dep_set(jid, msg_submit->jt.job_deps);        
    }

    gw_user_pool_inc_jobs(uid,msg_submit->msg.number_of_tasks - useradd);
        
    pthread_mutex_unlock(&(array->mutex));
    
    msg_submit->msg.rc       = GW_RC_SUCCESS;    
    msg_submit->msg.array_id = array_id;  

    gw_am_trigger(gw_dm.rm_am,"GW_RM_SUBMIT",&(msg_submit->msg));
    
    gw_log_print("DM",'I',"New array %i allocated and initialized.\n",array_id);
}
