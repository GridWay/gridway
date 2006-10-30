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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <pwd.h>
#include <time.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>

#include "gw_job.h"
#include "gw_dm.h"
#include "gw_em.h"
#include "gw_em_mad.h"
#include "gw_common.h"
#include "gw_log.h"
#include "gw_user_pool.h"
#include "gw_host_pool.h"

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int gw_job_recover(gw_job_t *job)
{
    time_t timestamp;

    int    user_id;
    int    rc;
    int    pinc, pstart;
        
    char   job_state_name[2048];
    char   user_name[2048];
    char   job_home[2048];
    char   history_filename[2048];
    char   state_filename[2048];
    char   template_filename[2048];
    char   conf_filename[2048];
    
    FILE * file;
    FILE * state_file;
    FILE * history_file;
   
    gw_template_t  template;
    gw_job_state_t job_state;
    gw_job_state_t previous_job_state;
    
    struct passwd * pw_ent;

    /*------------------------------------------------------------------------*/

    file = fopen(job->directory, "r");

    if (file == NULL)
        return 1;

    /*------------------------------------------------------------------------*/

    gw_log_print("DM",'I', "Recovering job %d.\n", job->id);

    sprintf(template_filename, "%s/job.template", job->directory);
    
    rc = gw_template_init(&template, template_filename);
    
    if (rc != 0)
    {
        gw_log_print("DM",'E',"Parse error, template file of job %d: %s.\n",
                job->id, template_filename);
        return -1;
    }
    
    gw_job_template_init(&(job->template), &template);
    
    /*------------------------------------------------------------------------*/
    
    sprintf(conf_filename, "%s/job.conf", job->directory);
    
    file = fopen(conf_filename, "r");

    if (file == NULL)
    {
        gw_log_print("DM",'E',"Could not open configuration file of job %d: %s\n",
                job->id, conf_filename); 
        return -1;
    }

    rc = fscanf(file, "%ld %s %s %i %i", &timestamp, user_name, job_home,&pstart,&pinc);
    
    if (rc != 5)
    {
        gw_log_print("DM",'E',"Bad filed number (%d) in job %d configuration file.\n",
                     rc,
                     job->id);
        return -1;
    }
    
    fclose(file);
    
    /*------------------------------------------------------------------------*/

    if (gw_user_pool_exists(user_name, &user_id) == GW_FALSE)
    {
#ifdef GWJOBDEBUG    	
        gw_log_print("DM",'D',"Registering user %s.\n", user_name);
#endif    
        rc = gw_user_pool_user_allocate(user_name, &user_id);

        if ( rc != 0 )
        {
            gw_log_print("DM",'E',"Could not register user %s.\n",
                    user_name);
            return -1;
        }
    }
    
#ifdef GWJOBDEBUG
    gw_log_print("DM",'D',"User %s registered with UID %d.\n", user_name,
            user_id);
#endif

    /*------------------------------------------------------------------------*/

    job->start_time = timestamp;
    job->owner      = strdup(user_name);
    job->user_id    = user_id;
    job->pstart     = pstart;
    job->pinc       = pinc;
    
    pw_ent = getpwnam(user_name);
    if (pw_ent != NULL)
    	job->template.user_home = strdup(pw_ent->pw_dir);
    else
    {
    	gw_log_print("DM",'E',"Could not get home for user %s.\n", user_name);
    	return -1;
    }
    	
    job->template.job_home = strdup(job_home);

    /*------------------------------------------------------------------------*/

#ifdef GWJOBDEBUG
    gw_log_print("DM",'D',"Recovering state transitions of job %d.\n", job->id); 
#endif

    sprintf(state_filename, "%s/job.state", job->directory);
    sprintf(history_filename, "%s/job.history", job->directory);

    state_file = fopen(state_filename, "r");
    history_file = fopen(history_filename, "r");

    if (state_file == NULL)
    {
        gw_log_print("DM",'E',"Could not open state file of job %d: %s\n",
                     job->id, 
                     state_filename);
        return -1;
    }

    /* If history file does not exits, generate an error only if we need to
      access a history record */

    /* Perform again state transitions */
    previous_job_state = GW_JOB_STATE_LIMIT;
    
    while (fscanf(state_file, "%ld %s", &timestamp, job_state_name) == 2)
    {
        if (previous_job_state == GW_JOB_STATE_LIMIT)
        {
            previous_job_state = GW_JOB_STATE_INIT;
        }
        else 
        {
            previous_job_state = job_state;
        }
        
        job_state = gw_job_get_state_code(job_state_name);

        /* Re-construct job lifecycle (states, history & statistics) */

        rc = gw_job_recover_state_transition(job, 
                                             previous_job_state, 
                                             job_state,
                                             timestamp, 
                                             history_file);
        if (rc == -1)
        {
            gw_log_print("DM",'E',
                    "Recovering state transition (%s->%s) of job %d.\n",
                    gw_job_get_state_name(previous_job_state),
                    gw_job_get_state_name(job_state), job->id);
        }
    }

    if (!feof(state_file))
    {
        gw_log_print("DM",'E',"Bad number of fields in job state file of job %d.\n",
                     job->id);
        return -1;
    }

    fclose(state_file);

    if (history_file != NULL)
        fclose(history_file);

    /*------------------------------------------------------------------------*/
    
#ifdef GWJOBDEBUG
    gw_log_print("DM",'D',"Recovering last state of job %d\n", job->id); 
#endif

    gw_job_set_state(job, previous_job_state, GW_TRUE);

    rc = gw_job_recover_last_state_transition(job,
                                              previous_job_state,
                                              job_state, 
                                              timestamp);
    if (rc == -1)
    {
        gw_log_print("DM",'E',
                "Could not recover last state transition (%s->%s) of job %d.\n",
                gw_job_get_state_name(previous_job_state),
                gw_job_get_state_name(job_state), job->id);
    }
    
    /* ----- Update user stats ------- */
    
	gw_user_pool_inc_jobs(user_id,1);
    
    return 0;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int gw_job_recover_state_transition(gw_job_t *job,
        gw_job_state_t previous_job_state, gw_job_state_t job_state,
        time_t timestamp, FILE *history_file)
{
    int rc;
    
    switch (previous_job_state)
    {
    case GW_JOB_STATE_PROLOG:
    case GW_JOB_STATE_MIGR_PROLOG:
         job->history->stats[PROLOG_EXIT_TIME] = timestamp;
         break;

    case GW_JOB_STATE_WRAPPER:
        job->history->stats[WRAPPER_EXIT_TIME] = timestamp;
        break;

    case GW_JOB_STATE_EPILOG:
    case GW_JOB_STATE_EPILOG_FAIL:
    case GW_JOB_STATE_EPILOG_RESTART:
    case GW_JOB_STATE_STOP_EPILOG:
    case GW_JOB_STATE_KILL_EPILOG:
        job->history->stats[EPILOG_EXIT_TIME] = timestamp;
        break;

    case GW_JOB_STATE_MIGR_EPILOG:
        job->history->next->stats[EPILOG_EXIT_TIME] = timestamp;
        job->history->next->stats[MIGRATION_EXIT_TIME] = timestamp;
        break;

    case GW_JOB_STATE_INIT:
    case GW_JOB_STATE_PENDING:
    case GW_JOB_STATE_HOLD:
    case GW_JOB_STATE_PRE_WRAPPER:
    case GW_JOB_STATE_STOP_CANCEL:
    case GW_JOB_STATE_STOPPED:
    case GW_JOB_STATE_KILL_CANCEL:
    case GW_JOB_STATE_MIGR_CANCEL:
    case GW_JOB_STATE_EPILOG_STD:
    case GW_JOB_STATE_FAILED:
    case GW_JOB_STATE_ZOMBIE:
        break;

    case GW_JOB_STATE_LIMIT:
        return -1;
    }

    /* ---------------------------------------------------------------------- */

    switch (job_state)
    {
    case GW_JOB_STATE_PROLOG:
#ifdef GWJOBDEBUG
        gw_log_print("DM",'D',"Recovering history record of job %d\n",
                job->id);
#endif     
        rc = gw_job_recover_history_record(history_file, job);
        
        if (rc == -1)
        {
            gw_log_print("DM",'E',"Could not recover history record of job %d\n",
                    job->id);
        }

        job->history->stats[START_TIME]        = timestamp;
        job->history->stats[PROLOG_START_TIME] = timestamp;
        break;

    case GW_JOB_STATE_MIGR_CANCEL:
#ifdef GWJOBDEBUG
        gw_log_print("DM",'D',"Recovering history record of job %d.\n",
                     job->id);
#endif
        rc = gw_job_recover_history_record(history_file, job);

        if (rc == -1)
        {
            gw_log_print("DM",'E',"Could not recover history record of job %d\n",
                    job->id);
        }
        break;
        
    case GW_JOB_STATE_MIGR_PROLOG:
        job->history->stats[START_TIME] = timestamp;
        job->history->stats[PROLOG_START_TIME] = timestamp;
        break;

    case GW_JOB_STATE_MIGR_EPILOG:
        job->history->next->stats[EPILOG_START_TIME] = timestamp;
        break;
        
    case GW_JOB_STATE_WRAPPER:
        job->history->stats[WRAPPER_START_TIME] = timestamp;
        break;
        
    case GW_JOB_STATE_EPILOG_STD:
    case GW_JOB_STATE_KILL_EPILOG:
    case GW_JOB_STATE_STOP_EPILOG:
    case GW_JOB_STATE_EPILOG_RESTART:
    case GW_JOB_STATE_EPILOG_FAIL:
        job->history->stats[EPILOG_START_TIME] = timestamp;
        break;

    case GW_JOB_STATE_PENDING:    	
    case GW_JOB_STATE_HOLD:
    case GW_JOB_STATE_PRE_WRAPPER:
    case GW_JOB_STATE_EPILOG:
    case GW_JOB_STATE_STOP_CANCEL:
    case GW_JOB_STATE_KILL_CANCEL:
        break;

    case GW_JOB_STATE_STOPPED:
    case GW_JOB_STATE_FAILED:
        job->history->stats[EXIT_TIME] = timestamp;
        job->exit_time = timestamp;
        break;

    case GW_JOB_STATE_ZOMBIE:
        job->history->stats[EXIT_TIME] = timestamp;
        job->exit_time = timestamp;
        job->history->reason = GW_REASON_NONE;
        break;

    case GW_JOB_STATE_INIT:
    case GW_JOB_STATE_LIMIT:
        return -1;
    }
    
    return 0;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int gw_job_recover_last_state_transition(gw_job_t *job,
        gw_job_state_t previous_job_state, gw_job_state_t job_state,
        time_t timestamp)
{
    int *id;
    int rc;
    
    id = (int *)malloc(sizeof(int));
    *id = job->id;

    switch(job_state)
    {
    case GW_JOB_STATE_PENDING:
        gw_dm_mad_job_schedule(&gw_dm.dm_mad[0],
                               *id,
                               -1,
                               GW_REASON_NONE,
                               job->nice,
                               job->user_id);
    case GW_JOB_STATE_HOLD:
    case GW_JOB_STATE_STOPPED:
    case GW_JOB_STATE_ZOMBIE:
    case GW_JOB_STATE_FAILED:
        free(id);
        gw_job_set_state(job, job_state, GW_TRUE);
        break;

    case GW_JOB_STATE_PROLOG:
   	
    	gw_user_pool_inc_running_jobs(job->user_id, 1);
    	
    	gw_host_inc_slots_nb(job->history->host);
       
        gw_am_trigger(&(gw_dm.am), "GW_DM_STATE_PROLOG", (void *)id);
        break;

    case GW_JOB_STATE_WRAPPER:
    	
    	gw_user_pool_inc_running_jobs(job->user_id, 1);
        
       	gw_host_inc_slots_nb(job->history->host);
        
        gw_job_set_state(job, GW_JOB_STATE_WRAPPER, GW_TRUE);

        gw_log_print("DM",'I',"Recovering GRAM contact for job %d.\n", job->id); 

        rc = gw_job_recover_job_contact(job);
        
        if (rc == -1)
        {
            gw_job_set_state(job, previous_job_state, GW_TRUE);
            
            gw_am_trigger(&(gw_dm.am), "GW_DM_STATE_WRAPPER", (void *)id);
        }
        else
        	free(id);
        break;

    case GW_JOB_STATE_EPILOG:
    	
    	gw_user_pool_inc_running_jobs(job->user_id, 1);
    	
        gw_host_inc_rjobs_nb(job->history->host);
                
        gw_am_trigger(&(gw_dm.am), "GW_DM_STATE_EPILOG", (void *)id);
        break;

    case GW_JOB_STATE_EPILOG_FAIL:

    	gw_user_pool_inc_running_jobs(job->user_id, 1);
               
        gw_host_inc_rjobs_nb(job->history->host);
        
        gw_am_trigger(&(gw_dm.am), "GW_DM_STATE_EPILOG_FAIL", (void *)id);
        break;

    case GW_JOB_STATE_EPILOG_RESTART:
    	
    	gw_user_pool_inc_running_jobs(job->user_id, 1);
               
        gw_host_inc_rjobs_nb(job->history->host);        

        gw_am_trigger(&(gw_dm.am), "GW_DM_STATE_EPILOG_RESTART", (void *)id);
        break;

    case GW_JOB_STATE_EPILOG_STD:
       	
    	gw_user_pool_inc_running_jobs(job->user_id, 1);
               
        gw_host_inc_rjobs_nb(job->history->host);

        gw_am_trigger(&(gw_dm.am), "GW_DM_STATE_EPILOG_STD", (void *)id);
        break;

    case GW_JOB_STATE_KILL_CANCEL:
        gw_job_set_state(job, job_state, GW_TRUE);

    case GW_JOB_STATE_KILL_EPILOG:                
    	
    	gw_user_pool_inc_running_jobs(job->user_id, 1);

        gw_host_inc_rjobs_nb(job->history->host);
        
        gw_am_trigger(&(gw_dm.am), "GW_DM_STATE_KILL_EPILOG", (void *)id);
        break;

    case GW_JOB_STATE_MIGR_CANCEL:
        gw_job_set_state(job, job_state, GW_TRUE);
            
    case GW_JOB_STATE_MIGR_PROLOG:    
    	
    	gw_user_pool_inc_running_jobs(job->user_id, 1);
    	
    	gw_host_inc_slots_nb(job->history->host);
        
    	gw_host_inc_rjobs_nb(job->history->next->host);
        				
        gw_am_trigger(&(gw_dm.am), "GW_DM_STATE_MIGR_PROLOG", (void *)id);
        break;

    case GW_JOB_STATE_MIGR_EPILOG:

    	gw_user_pool_inc_running_jobs(job->user_id, 1);

    	gw_host_inc_slots_nb(job->history->host);
  
    	gw_host_inc_rjobs_nb(job->history->next->host);
    
        gw_am_trigger(&(gw_dm.am), "GW_DM_STATE_MIGR_EPILOG", (void *)id);
        break;

    case GW_JOB_STATE_PRE_WRAPPER:
   	
    	gw_user_pool_inc_running_jobs(job->user_id, 1);

    	gw_host_inc_slots_nb(job->history->host);
    	            
        gw_am_trigger(&(gw_dm.am), "GW_DM_STATE_PRE_WRAPPER", (void *)id);
        break;

    case GW_JOB_STATE_STOP_CANCEL:
        gw_job_set_state(job, job_state, GW_TRUE);
        
    case GW_JOB_STATE_STOP_EPILOG:

    	gw_user_pool_inc_running_jobs(job->user_id, 1);

    	gw_host_inc_rjobs_nb(job->history->host);

        gw_am_trigger(&(gw_dm.am), "GW_DM_STATE_STOP_EPILOG", (void *)id);
        break;

    case GW_JOB_STATE_INIT:
    case GW_JOB_STATE_LIMIT:
        return -1;
    }

    return 0;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int gw_job_recover_history_record(FILE *history_file, gw_job_t *job)
{
    int rc;
    char hostname[2048], queue_name[2048], fork_name[2048], lrms_name[2048];
    int rank, nice;
    char em_mad_name[2048], tm_mad_name[2048], im_mad_name[2048];
    int host_id;
    gw_host_t *host;
    
    if (history_file == NULL)
        return -1;

    rc = fscanf(history_file, "%s %d %s %s %s %d %s %s %s", hostname,
            &rank, queue_name, fork_name, lrms_name, &nice,
            em_mad_name, tm_mad_name, im_mad_name);

    if (rc != 9)
    {
        gw_log_print("DM",'E',"Wrond field number (%d) in history record of job %d.\n",
                     rc,
                     job->id); 
        return -1;
    }

    host = gw_host_pool_search(hostname, GW_FALSE);

    if (host == NULL)
    {
#ifdef GWJOBDEBUG    	
        gw_log_print("DM",'D',"Registering host %s.\n", hostname);
#endif            
        host_id = gw_host_pool_host_allocate(hostname, 
                                             nice,
                                             em_mad_name, 
                                             tm_mad_name, 
                                             im_mad_name);
            
        host = gw_host_pool_get_host(host_id, GW_FALSE);
    }
    
#ifdef GWJOBDEBUG
    gw_log_print("DM",'D',"Host %s registered with HID %d.\n", hostname,
            host->host_id);
#endif

    /*------------------------------------------------------------------------*/

	if (job->history != NULL) /* Not the first record */
		job->restarted++;
		
    rc = gw_job_history_add(&(job->history),
                            host, 
                            rank, 
                            queue_name, 
                            fork_name, 
                            lrms_name, 
                            job->owner,
                            job->template.user_home, 
                            job->id, 
                            job->user_id, 
                            GW_TRUE);
    if (rc == -1)
    {
        gw_log_print("DM",'E',"Could not add history record.\n");
        return -1;
    }
    
    return 0;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int gw_job_recover_job_contact(gw_job_t *job)
{
    char filename[2048];
    FILE *file;
    int rc;
    char job_contact[2048];
    gw_em_mad_t *em_mad;
    
    sprintf(filename, "%s/job.contact", job->directory);

    file = fopen(filename, "r");

    if (file == NULL)
    {
        gw_log_print("DM",'E',"Could not open GRAM contact file of job %d.\n",
                     job->id);
        return -1;
    }
    else
    {
        rc = fscanf(file, "%s", job_contact);
            
        if (rc != 1)
        {
            gw_log_print("DM",'E',"Could not read GRAM contact of job %d.\n",
                         job->id);
            return -1;
        }

#ifdef GWJOBDEBUG
        gw_log_print("DM",'D',"Job contact for job %d is %s.\n", 
                     job->id,
                     job_contact);
#endif                         
        em_mad = gw_user_pool_get_em_mad(job->user_id,
                                         job->history->host->em_mad);
                
        gw_em_mad_recover(em_mad, job->id, job_contact);

        fclose(file);
    }

    return 0;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
