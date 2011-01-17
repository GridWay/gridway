/* -------------------------------------------------------------------------- */
/* Copyright 2002-2010 GridWay Project Leads (GridWay.org)                    */
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
#include <stdio.h>
#include <pthread.h>
#include <limits.h>
#include <libgen.h>

#include "gw_em_rsl.h"
#include "gw_job.h"
#include "gw_user_pool.h"

char *gw_template_jobtype_string_jdl(gw_jobtype_t type);
char *gw_job_jdl_environment(gw_job_t *job);

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
 
char* gw_generate_wrapper_jdl(gw_job_t *job)
{
    char *jdl;
    char *job_environment; 
    char  jdl_buffer[GW_RSL_LENGTH];
    char  tmp_buffer[GW_RSL_LENGTH];
    char *jobtype;
    char *staging_url;
  
    /* ---------------------------------------------------------------------- */
    /* 1.- Create dynamic job data environment                                */
    /* ---------------------------------------------------------------------- */
  
    job_environment = gw_job_jdl_environment(job);
    
    if ( job_environment == NULL )
        return NULL;

    if (job->history->tm_mad->url != NULL)
    {
        /* Perform staging with the URL provided by the TM MAD */

        staging_url = job->history->tm_mad->url;

        snprintf(wrapper, PATH_MAX -1, "%s", job->template.wrapper);
    }
    else
    {
        return NULL;
    }

    jobtype = strdup(gw_template_jobtype_string_jdl(job->template.type));

    /* ---------------------------------------------------------------------- */
    /* 2.- Build JSL String & Return it                                       */
    /* ---------------------------------------------------------------------- */

    snprintf(jdl_buffer, sizeof(char) * GW_RSL_LENGTH,
            "[JobType = \"%s\";"
            "Executable = \"%s\";"
            "Arguments=\"%s/%s/" GW_VAR_DIR "/%d/job.env\";"
            "StdOutput = \"stdout.wrapper.%d\";"
            "StdError = \"stderr.wrapper.%d\";"
            "InputSandbox = {\"%s/%s\"};"
            "OutputSandbox = {\"%s/%s/%d/stdout.wrapper.%d\", \"%s/%s/%d/stderr.wrapper.%d\"};"
            "BatchSystem = \"%s\";"
            "QueueName = \"%s\";"
            "CpuNumber = %d;]",
            jobtype, 
            job->template.wrapper,
            staging_url, gw_conf.gw_location, job->id,
            job->restarted,
            job->restarted,
            staging_url, job->template.wrapper,
            staging_url, gw_conf.gw_location, job->id, job->restarted,
            staging_url, gw_conf.gw_location, job->id, job->restarted,
            job->history->host->lrms_type,
            job->history->queue,
            job->template.np);

    free(job_environment);
    free(jobtype);
    
    if ( strlen(jdl_buffer) >= GW_RSL_LENGTH )
        return NULL;
    
    jdl = strdup(jdl_buffer);
    return jdl;
}

char *gw_template_jobtype_string_jdl(gw_jobtype_t type)
{
     if (type ==  GW_JOB_TYPE_SINGLE)
        return "Normal";
    else if (type ==  GW_JOB_TYPE_MULTIPLE)
        return "unknown";  
    else if (type == GW_JOB_TYPE_MPI)
        return "Mpich";
    else
        return "unknown";
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

// NOT ALLOWED IN CREAM!!!

char* gw_generate_pre_wrapper_jdl(gw_job_t *job)
{
    char *jdl;
    char *job_environment;
    char *pre_wrapper;
    char *pre_wrapper_arguments;    
    char jdl_buffer[GW_RSL_LENGTH];
    int  rc, size;
    
    /* ---------------------------------------------------------------------- */
    /* 1.- Create dynamic job data environment                                */
    /* ---------------------------------------------------------------------- */
  
    job_environment = gw_job_jdl_environment(job);
    
    if ( job_environment == NULL )
        return NULL;
    
    /* ---------------------------------------------------------------------- */
    /* 2.- Build JDL String & Return it                                       */
    /* ---------------------------------------------------------------------- */

    if ( job->template.pre_wrapper[0] == '/' ) /*Absolute path*/
        pre_wrapper = strdup(job->template.pre_wrapper);
    else
        pre_wrapper = gw_job_substitute(job->template.pre_wrapper, job);
        
    if ( pre_wrapper == NULL )
    {
        
        gw_job_print(job,"EM",'E',"Parse error (%s) while generating rsl.\n",job->template.pre_wrapper);    
        free(job_environment);            
        return NULL;
    }


    if ( job->template.pre_wrapper[0] == '/' ) /*Absolute path*/                
        rc = snprintf(jdl_buffer, sizeof(char) * GW_RSL_LENGTH,
            "[JobType = \"Normal\";"
            "Executable = \"%s\";"
            "StdOutput = \".gw_%s_%i/stdout.pre_wrapper\";"
            "StdError = \".gw_%s_%i/stderr.pre_wrapper\";"
            "Environment = %s;"
            "BatchSystem = \"fork\";",
            pre_wrapper,
            job->owner, job->id,
            job->owner, job->id,
            job_environment);

    else
        rc = snprintf(jdl_buffer, sizeof(char) * GW_RSL_LENGTH,
            "[JobType = \"Normal\";"
            "Executable = \".gw_%s_%i/%s\";"
            "Stdoutput = \".gw_%s_%i/stdout.pre_wrapper\")"
            "StdError = \".gw_%s_%i/stderr.pre_wrapper\")"
            "Environment = %s;"
            "BatchSystem = \"fork\";",
            job->owner, job->id, pre_wrapper,
            job->owner, job->id,
            job->owner, job->id,
            job_environment);

    if ((rc >= (GW_RSL_LENGTH * sizeof(char))) || ( rc < 0 ) )          
    {
        free(job_environment);        
        free(pre_wrapper);        
        return NULL;
    }
                     
    if ( job->template.pre_wrapper_arguments != NULL )
    {
        pre_wrapper_arguments = gw_job_substitute(job->template.pre_wrapper_arguments, job);

        if ( pre_wrapper_arguments == NULL )
        {
            gw_job_print(job,"EM",'E',"Parse error (%s) while generating jdl.\n",job->template.pre_wrapper_arguments);    
            free(job_environment);
            free(pre_wrapper);
            return NULL;
        }
        
        size = strlen(jdl_buffer) + strlen(pre_wrapper_arguments) + 13;
        if ( size > GW_RSL_LENGTH )
        {
            free(pre_wrapper_arguments);
            free(job_environment);
            free(pre_wrapper);
            return NULL;
        }
        
        strcat(jdl_buffer,"Arguments = ");
        strcat(jdl_buffer,pre_wrapper_arguments);
        strcat(jdl_buffer,";");
        
        free(pre_wrapper_arguments);
    }
    
    free(job_environment);        
    free(pre_wrapper);
    
    strcat(jdl_buffer,"QueueName =\"cream_1\";]");

    jdl = strdup(jdl_buffer);
    return jdl;
}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

char *gw_job_jdl_environment(gw_job_t *job)
{
    char jdl_buffer[512];
    char *jdl_env; 
    int  rc;    
    
    if (job->history == NULL)
    {
        return NULL;
    }
    
    if (job->history->host == NULL)
    {
        return NULL;
    }
            
    rc = snprintf(jdl_buffer, 512,
                "{\"GW_HOSTNAME=%s\","    
                "\"GW_USER=%s\","
                "\"GW_JOB_ID=%i\","
                "\"GW_TASK_ID=%i\","
                "\"GW_ARRAY_ID=%i\","
                "\"GW_TOTAL_TASKS=%i\","
                "\"GW_RESTARTED=%i\"}",
                job->history->host->hostname,        
                job->owner,
                job->id,
                job->task_id,
                job->array_id,
                job->total_tasks,
                job->restarted);
        
    if ((rc >= 512 ) || ( rc < 0 ) )
        return NULL;
                
    jdl_env = strdup(jdl_buffer);
    
    return jdl_env;
}
