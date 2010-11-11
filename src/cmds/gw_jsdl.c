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

#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "gw_job.h"
#include "gw_template.h"
#include "gw_job_template.h"
#include "gw_em_jsdl.h"

#include "gw_conf.h"

void gw_job_template_set_str(char **dst, const char *src);

int main(int argc, char **argv)
{
    gw_job_t job;
    int j,job_id=0;
    char *jsdl_buffer;
    gw_template_t template;
    int rc;
    FILE *jsdl_file,*jt;
    char   str_buffer[PATH_MAX];

    if (argc != 3)
    {
	fprintf(stderr,"Invalid number of arguments.");
	exit(0);
    }

    j = gw_job_init2(&job,job_id);

    rc = gw_template_init(&template,argv[1]);
    if (rc==1)
    { 
        fprintf(stderr,"File does not exist.");
    }
    gw_job_template_init2(&(job.template),&template);
    jsdl_buffer = gw_generate_wrapper_jsdl(&job);
    
    if (!(jsdl_file=fopen(argv[2],"w")))
    {
        fprintf(stderr,"Error opening the JSDL file."); 
        exit(0);
    }
    fprintf(jsdl_file, "%s", jsdl_buffer);
    exit(0);   
}

int gw_job_init2(gw_job_t *job, int job_id)
{
    char   str_buffer[PATH_MAX];

    //pthread_mutex_init(&(job->mutex), (pthread_mutexattr_t *) NULL);

    //pthread_mutex_lock(&(job->mutex));

/* -------------------------------------------------------------------------- */

        snprintf(str_buffer,
             PATH_MAX -1 ,
             "%s/" GW_VAR_DIR "/%i",
	     "%s%i",
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

    //pthread_mutex_unlock(&(job->mutex));

    return 0;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

void gw_job_history_init(gw_history_t **job_history)
{
    *job_history = NULL;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_job_template_init2 (gw_job_template_t *jt, const gw_template_t *ct)
{
        int i;

        gw_job_template_set_str(&(jt->file), ct->file);
    gw_job_template_set_str(&(jt->name), ct->name);

        gw_job_template_set_str(&(jt->job_home), ct->job_home);
        gw_job_template_set_str(&(jt->user_home), ct->user_home);

        gw_job_template_set_str(&(jt->executable), ct->executable);
        gw_job_template_set_str(&(jt->arguments), ct->arguments);

        gw_job_template_set_str(&(jt->pre_wrapper), ct->pre_wrapper);
        gw_job_template_set_str(&(jt->pre_wrapper_arguments), ct->pre_wrapper_arguments);

        jt->num_input_files = ct->num_input_files;

        if ( jt->num_input_files > 0 )
        {
          jt->input_files = (char ***) malloc (sizeof(char **)*jt->num_input_files);

          for (i = 0; i<jt->num_input_files; i++)
          {
                jt->input_files[i] = (char **) malloc ( sizeof(char *) * 2 );
                gw_job_template_set_str(&(jt->input_files[i][0]), ct->input_files[i][0]);
                gw_job_template_set_str(&(jt->input_files[i][1]), ct->input_files[i][1]);
          }
        }
        else
                jt->input_files = NULL;

        jt->num_output_files = ct->num_output_files;

        if ( jt->num_output_files > 0 )
        {
          jt->output_files = (char ***) malloc (sizeof(char **)*jt->num_output_files);

          for (i = 0; i<jt->num_output_files; i++)
          {
                jt->output_files[i] = (char **) malloc ( sizeof(char *) * 2 );
                gw_job_template_set_str(&(jt->output_files[i][0]), ct->output_files[i][0]);
                gw_job_template_set_str(&(jt->output_files[i][1]), ct->output_files[i][1]);
          }
        }
else
                jt->output_files = NULL;

        jt->num_restart_files = ct->num_restart_files;

        if ( jt->num_restart_files > 0 )
        {
          jt->restart_files = (char **) malloc (sizeof(char *)*jt->num_restart_files);

          for (i = 0; i<jt->num_restart_files; i++)
          {
                gw_job_template_set_str(&(jt->restart_files[i]), ct->restart_files[i]);
          }
        }
        else
                jt->restart_files = NULL;

        jt->num_env = ct->num_env;

        if ( jt->num_env > 0 )
        {
          jt->environment = (char ***) malloc (sizeof(char **)*jt->num_env);

          for (i = 0; i<jt->num_env; i++)
          {
                jt->environment[i] = (char **) malloc ( sizeof(char *) * 2 );
                gw_job_template_set_str(&(jt->environment[i][0]), ct->environment[i][0]);
                gw_job_template_set_str(&(jt->environment[i][1]), ct->environment[i][1]);
          }
        }
        else
                jt->environment = NULL;

        gw_job_template_set_str(&(jt->stdin_file), ct->stdin_file);
        gw_job_template_set_str(&(jt->stdout_file), ct->stdout_file);
        gw_job_template_set_str(&(jt->stderr_file), ct->stderr_file);

        gw_job_template_set_str(&(jt->requirements), ct->requirements);
        gw_job_template_set_str(&(jt->rank), ct->rank);

        jt->rescheduling_interval  = ct->rescheduling_interval;
        jt->rescheduling_threshold = ct->rescheduling_threshold;
    jt->checkpoint_interval    = ct->checkpoint_interval;

    gw_job_template_set_str(&(jt->checkpoint_url), ct->checkpoint_url);

        jt->suspension_timeout = ct->suspension_timeout;
        jt->cpuload_threshold  = ct->cpuload_threshold;

        jt->reschedule_on_failure = ct->reschedule_on_failure;
        jt->number_of_retries     = ct->number_of_retries;

    gw_job_template_set_str(&(jt->wrapper), ct->wrapper);
    gw_job_template_set_str(&(jt->monitor), ct->monitor);

    gw_job_pool_dep_cp(ct->job_deps, &(jt->job_deps));

    jt->type = ct->type;
    jt->np = ct->np;

    jt->deadline = ct->deadline;
}

void gw_job_template_set_str(char **dst, const char *src)
{
        if (src[0] == '\0')
                *dst = NULL;
        else
                *dst = strdup(src);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_job_pool_dep_cp (const int * src, int **dst)
{
        int i = 0;

        if (src == NULL)
        {
                *dst = NULL;
                return;
        }

        while (src[i]!=-1)
        i++;

        if ( i == 0 )
                *dst = NULL;
        else
        {
                *dst = (int *) malloc (sizeof(int)*(i+1));

                i = 0;
                while (src[i] != -1)
                {
                        (*dst)[i] = src[i];
                        i++;
                }

                (*dst)[i] = -1;
        }
}
