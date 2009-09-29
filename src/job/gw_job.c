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
#include <limits.h>

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

int gw_job_fill(gw_job_t *job, const gw_msg_t *msg)
{
    int    rc;
    FILE   *log, *template_file, *file;
    char   str_buffer[PATH_MAX];
    char   sh_command[PATH_MAX];
    char   conf_filename[PATH_MAX], template_filename[PATH_MAX];
    
/* -------------------------------------------------------------------------- */

    sprintf(sh_command,"rm -rf %s",job->directory);

    rc = system(sh_command);
    
    if ( rc != 0 )
        return -1;
    
/* -------------------------------------------------------------------------- */

    sprintf(sh_command,"mkdir -m 775 -p %s",job->directory);

    rc = system(sh_command);
    
    if ( rc != 0 )
        return -1;
        
/* -------------------------------------------------------------------------- */
    	
	if (job->owner != NULL )
		free(job->owner);
		
    job->owner = strdup(msg->owner);	           

/* -------------------------------------------------------------------------- */
    
    gw_job_template_init( &(job->template), &(msg->jt) );
        
/* -------------------------------------------------------------------------- */
    
    log = fopen(job->log_file,"a");
    
    gw_job_template_print(log, &(job->template));
    
    fclose(log);

/* -------------------------------------------------------------------------- */

    sprintf(template_filename, "%s/job.template", job->directory);

    template_file = fopen(template_filename, "a");
    
    gw_job_template_to_file(template_file, &(job->template));
    
    fclose(template_file);

/* -------------------------------------------------------------------------- */
    
    if ( job->template.checkpoint_url == NULL )
    {
        snprintf(str_buffer,
                 PATH_MAX-1,
                 "file://%s/" GW_VAR_DIR "/%d/", 
                 gw_conf.gw_location, 
                 job->id);
                 
        job->template.checkpoint_url = strdup(str_buffer);
    }

    sprintf(conf_filename, "%s/job.conf", job->directory);
    
    file = fopen(conf_filename, "w");

    if (file == NULL)
    {
        gw_log_print("DM",'E',"Opening configuration file of job %d.\n",
                job->id); 
        return -1;
    }

    if (msg->proxy_path[0] == '\0'){
        fprintf(file, "%ld %s - %s %i %i\n", job->start_time, job->owner,
                job->template.job_home, msg->pstart, msg->pinc);
	}
    else {
        fprintf(file, "%ld %s %s %s %i %i\n", job->start_time, job->owner, msg->proxy_path,
                job->template.job_home, msg->pstart, msg->pinc);
	}
    
    fclose(file);
    
    return 0;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int gw_job_init(gw_job_t *job, int job_id)
{
    char   str_buffer[PATH_MAX];
	
    pthread_mutex_init(&(job->mutex), (pthread_mutexattr_t *) NULL);

    pthread_mutex_lock(&(job->mutex));

/* -------------------------------------------------------------------------- */
       
	snprintf(str_buffer, 
             PATH_MAX -1 , 
             "%s/" GW_VAR_DIR "/%i", 
             gw_conf.gw_location,
             job_id);
                      
    job->directory = strdup(str_buffer);

/* -------------------------------------------------------------------------- */

    snprintf(str_buffer, 
             PATH_MAX - 1, 
             "%s/job.log", 
             job->directory);
    
    job->log_file = strdup(str_buffer);
    
/* -------------------------------------------------------------------------- */

    snprintf(str_buffer,
             PATH_MAX - 1,
             "%s/job.env",
             job->directory);
	
    job->env_file = strdup(str_buffer);
	
/* -------------------------------------------------------------------------- */

    job->owner    = NULL;
    job->user_id  = -1;

/* -------------------------------------------------------------------------- */
    job->fixed_priority = GW_JOB_MIN_PRIORITY;
    
    job->id           = job_id;
    job->array_id     = -1;
    job->task_id      = 0;
    job->total_tasks  = 0;
    
    job->pinc         = 0;
    job->pstart       = 0;

/* -------------------------------------------------------------------------- */

    gw_job_history_init(&(job->history));

/* -------------------------------------------------------------------------- */
        
    job->em_state  = GW_EM_STATE_INIT;
    job->tm_state  = GW_TM_STATE_INIT;
    job->job_state = GW_JOB_STATE_INIT;

/* -------------------------------------------------------------------------- */
   
    job->start_time = time(NULL);
    job->exit_time  = 0;

    job->last_poll_time          = 0;
    job->last_rescheduling_time  = 0;
    job->last_checkpoint_time    = 0;
    
/* -------------------------------------------------------------------------- */
    
    job->exit_code      = 0;    
    job->restarted      = 0;
    job->client_waiting = GW_FALSE;
    job->reschedule     = GW_FALSE;
    
/* -------------------------------------------------------------------------- */
	
	job->xfrs.xfrs           = NULL;
	job->xfrs.number_of_xfrs = 0;
	job->xfrs.failure_limit  = 0;
    
   	job->chk_xfrs.xfrs           = NULL;
	job->chk_xfrs.number_of_xfrs = 0;
	job->chk_xfrs.failure_limit  = 0;
	
    memset((void *) &(job->template), 0 , sizeof(gw_job_template_t));

/* -------------------------------------------------------------------------- */

    pthread_mutex_unlock(&(job->mutex));

    return 0;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_job_destroy(gw_job_t *job)
{   	
	if (job->owner != NULL)
	    free(job->owner);
	    
	if (job->directory != NULL)
	    free(job->directory);
	    
	if (job->log_file != NULL)
        free(job->log_file);

	if (job->env_file != NULL)
		free(job->env_file);        

    gw_job_template_destroy(&(job->template));
	    
    gw_job_history_destroy(&(job->history));
    
    gw_xfr_destroy(&(job->xfrs));
    
    gw_xfr_destroy(&(job->chk_xfrs));
        
    pthread_mutex_unlock(&(job->mutex));

    pthread_mutex_destroy(&(job->mutex));
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void  gw_job_print(gw_job_t *job, const char *module, const char mode, const char *str_format,...)
{
    FILE    *log;
    va_list ap;
    time_t  the_time;
    
    char str[26];

    va_start(ap, str_format);
        
    log = fopen(job->log_file,"a");

    if (log != NULL)
    {
        the_time = time(NULL);

#ifdef GWSOLARIS
        ctime_r(&(the_time),str,sizeof(char)*26);
#else
        ctime_r(&(the_time),str);
#endif

        str[24]='\0';

        fprintf(log,"%s [%s][%c]: ", str, module, mode);
        vfprintf(log,str_format,ap);
        
        fclose(log);
    }
        
    return;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_job_print_history(gw_job_t *job)
{   
    FILE    *log;    
    
    log = fopen(job->log_file,"a");

    if (log != NULL)
    {
        if (job->history != NULL)
            gw_job_history_print(log, job->history);
        
        fclose(log);
    }
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

void gw_job_set_state(gw_job_t *job, gw_job_state_t job_state,
        gw_boolean_t recover)
{
    FILE* file;
    char state_file[PATH_MAX];
    
    if ( job == NULL )
    	return;
    	
    /* Set state */
    job->job_state = job_state;
    
    /* Save persistent state */
    if (!recover)
    {
        sprintf(state_file, "%s/job.state", job->directory);
    
        file = fopen(state_file, "a");

        if (file == NULL)
            return;

        fprintf(file, "%ld %s\n", time(NULL), gw_job_get_state_name(job_state));

        fclose(file);

        /* Generate log file with fork job starter's format (based
         * on SEG's messages):
         *     001;TIMESTAMP;JOBID;STATE;EXIT_CODE
         * where:
         *     JOBID: local scheduler-specific job id
         *     STATE: new job state (integer as per the GRAM protocol constants)
         *     EXIT_CODE: job exit code if STATE is done or failed. 
         */

        file = fopen(gw_conf.gw_globus_seg, "a");

        if (file == NULL)
            return;

        fprintf(file, "001;%ld;%d;%d;%d\n", time(NULL), job->id,
                gw_job_get_gram_state(job_state), job->exit_code);

        fclose(file);
    }
    
    gw_job_print(job, "DM",'I',"New state is %s.\n",gw_job_get_state_name(job_state));
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

gw_job_state_t gw_job_get_state(gw_job_t *job)
{
    return job->job_state;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

static char* gw_job_state_names[21] = {
   "INIT",
   "PENDING",
   "HOLD",
   "PROLOG",
   "PRE_WRAPPER",
   "WRAPPER",
   "EPILOG",
   "EPILOG_STD",
   "EPILOG_RESTART",
   "EPILOG_FAIL",
   "STOP_CANCEL",
   "STOP_EPILOG",
   "STOPPED",
   "KILL_CANCEL",
   "KILL_EPILOG",
   "MIGR_CANCEL",
   "MIGR_PROLOG",
   "MIGR_EPILOG",
   "DONE",
   "FAILED",
   "----"
};

char *gw_job_get_state_name(gw_job_state_t job_state)
{
    if(job_state >= 0 && job_state < GW_JOB_STATE_LIMIT)
        return gw_job_state_names[job_state];
    else 
        return gw_job_state_names[GW_JOB_STATE_LIMIT];  
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

globus_gram_protocol_job_state_t gw_job_state_mapping[21] = {
    GLOBUS_GRAM_PROTOCOL_JOB_STATE_PENDING,     /* INIT */
    GLOBUS_GRAM_PROTOCOL_JOB_STATE_PENDING,     /* PENDING */
    GLOBUS_GRAM_PROTOCOL_JOB_STATE_SUSPENDED,   /* HOLD */
    GLOBUS_GRAM_PROTOCOL_JOB_STATE_ACTIVE,      /* PROLOG */
    GLOBUS_GRAM_PROTOCOL_JOB_STATE_ACTIVE,      /* PRE_WRAPPER */
    GLOBUS_GRAM_PROTOCOL_JOB_STATE_ACTIVE,      /* WRAPPER */
    GLOBUS_GRAM_PROTOCOL_JOB_STATE_ACTIVE,      /* EPILOG */
    GLOBUS_GRAM_PROTOCOL_JOB_STATE_ACTIVE,      /* EPILOG_STD */
    GLOBUS_GRAM_PROTOCOL_JOB_STATE_ACTIVE,      /* EPILOG_RESTART */
    GLOBUS_GRAM_PROTOCOL_JOB_STATE_ACTIVE,      /* EPILOG_FAIL */
    GLOBUS_GRAM_PROTOCOL_JOB_STATE_ACTIVE,      /* STOP_CANCEL */
    GLOBUS_GRAM_PROTOCOL_JOB_STATE_ACTIVE,      /* STOP_EPILOG */
    GLOBUS_GRAM_PROTOCOL_JOB_STATE_SUSPENDED,   /* STOPPED */
    GLOBUS_GRAM_PROTOCOL_JOB_STATE_ACTIVE,      /* KILL_CANCEL */
    GLOBUS_GRAM_PROTOCOL_JOB_STATE_ACTIVE,      /* KILL_EPILOG */
    GLOBUS_GRAM_PROTOCOL_JOB_STATE_ACTIVE,      /* MIGR_CANCEL */
    GLOBUS_GRAM_PROTOCOL_JOB_STATE_ACTIVE,      /* MIGR_PROLOG */
    GLOBUS_GRAM_PROTOCOL_JOB_STATE_ACTIVE,      /* MIGR_EPILOG */
    GLOBUS_GRAM_PROTOCOL_JOB_STATE_DONE,        /* ZOMBIE */
    GLOBUS_GRAM_PROTOCOL_JOB_STATE_FAILED,      /* FAILED */
    GLOBUS_GRAM_PROTOCOL_JOB_STATE_PENDING,     /* LIMIT */
};

globus_gram_protocol_job_state_t gw_job_get_gram_state(gw_job_state_t job_state)
{
    if(job_state >= 0 && job_state < GW_JOB_STATE_LIMIT)
        return gw_job_state_mapping[job_state];
    else 
        return gw_job_state_mapping[GW_JOB_STATE_LIMIT];  
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

gw_job_state_t gw_job_get_state_code(char *job_state)
{
    switch (job_state[0])
    {
        case 'D':
            if (strcmp(job_state, "DONE") == 0)
                return GW_JOB_STATE_ZOMBIE;
            break;
                	
        case 'E':
            if (strcmp(job_state, "EPILOG") == 0)
                return GW_JOB_STATE_EPILOG;
            else if (strcmp(job_state, "EPILOG_STD") == 0)
                return GW_JOB_STATE_EPILOG_STD;
            else if (strcmp(job_state, "EPILOG_RESTART") == 0)
                return GW_JOB_STATE_EPILOG_RESTART;
            else if (strcmp(job_state, "EPILOG_FAIL") == 0)
                return GW_JOB_STATE_EPILOG_FAIL;
            break;
            
        case 'F':
            if (strcmp(job_state, "FAILED") == 0)
                return GW_JOB_STATE_FAILED;
            break;

        case 'H':
            if (strcmp(job_state, "HOLD") == 0)
                return GW_JOB_STATE_HOLD;
            break;
            
        case 'K':
            if (strcmp(job_state, "KILL_CANCEL") == 0)
                return GW_JOB_STATE_KILL_CANCEL;
            else if (strcmp(job_state, "KILL_EPILOG") == 0)
                return GW_JOB_STATE_KILL_EPILOG;
            break;
            
        case 'M':
            if (strcmp(job_state, "MIGR_CANCEL") == 0)
                return GW_JOB_STATE_MIGR_CANCEL;
            else if (strcmp(job_state, "MIGR_PROLOG") == 0)
                return GW_JOB_STATE_MIGR_PROLOG;
            else if (strcmp(job_state, "MIGR_EPILOG") == 0)
                return GW_JOB_STATE_MIGR_EPILOG;
            break;
            
        case 'P':
            if (strcmp(job_state, "PENDING") == 0)
                return GW_JOB_STATE_PENDING;
            else if (strcmp(job_state, "PROLOG") == 0)
                return GW_JOB_STATE_PROLOG;
            else if (strcmp(job_state, "PRE_WRAPPER") == 0)
                return GW_JOB_STATE_PRE_WRAPPER;
            break;
            
        case 'S':
            if (strcmp(job_state, "STOP_CANCEL") == 0)
                return GW_JOB_STATE_STOP_CANCEL;
            else if (strcmp(job_state, "STOP_EPILOG") == 0)
                return GW_JOB_STATE_STOP_EPILOG;
            else if (strcmp(job_state, "STOPPED") == 0)
                return GW_JOB_STATE_STOPPED;
            break;
            
        case 'W':
            if (strcmp(job_state, "WRAPPER") == 0)
                return GW_JOB_STATE_WRAPPER;
            break;
    }

    gw_log_print("DM",'W',"Unknown job state %s.\n", job_state);
    
    return GW_JOB_STATE_LIMIT;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

int gw_job_is_wrapper_based(gw_job_t *job)
{
    int isprews   = 0;
    int ismpi     = 0;
    int isgw      = 0;
    int iswrapper = 0;
    
    int wbe;
    
    if ((job->history != NULL) && 
        (job->history->host != NULL) && 
        (job->history->host->lrms_name != NULL) && 
        (job->history->host->lrms_type != NULL))
    {
        isprews = strncmp(job->history->host->lrms_name, "jobmanager-", 11) == 0;
        isgw    = strcasecmp(job->history->host->lrms_type, "gw") == 0;    
    }
    
    ismpi     = job->template.type == GW_JOB_TYPE_MPI;    
    iswrapper = job->template.wrapper != NULL;
        
    wbe = isprews || ( !ismpi && !isgw && iswrapper );
    
    return wbe;  
}
