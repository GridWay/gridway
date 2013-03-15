/* -------------------------------------------------------------------------- */
/* Copyright 2002-2013, GridWay Project Leads (GridWay.org)                   */
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
#include <limits.h>

#include "gw_em_rsl.h"
#include "gw_job.h"
#include "gw_user_pool.h"


char *gw_em_jsdl_environment(gw_job_t *job);
char *gw_em_jsdl_staging(gw_job_t *job);

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

char* gw_generate_wrapper_jsdl(gw_job_t *job)
{
    char *jsdl;
    char *job_environment;
    char *staging;
    char jsdl_buffer[4*GW_RSL_LENGTH];
    char tmp_buffer[4*GW_RSL_LENGTH];
    char *wrapper;

    wrapper = strrchr(job->template.wrapper, '/');
    wrapper = strdup(strtok(wrapper, "/"));

    /* ---------------------------------------------------------------------- */
    /* 1.- Create dynamic job data environment                                */
    /* ---------------------------------------------------------------------- */

    job_environment = gw_em_jsdl_environment(job);

    if ( job_environment == NULL )
        return NULL;

    /* ---------------------------------------------------------------------- */
    /* 2.- Build JSDL String & Return it                                       */
    /* ---------------------------------------------------------------------- */

    snprintf(jsdl_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<jsdl:JobDefinition xmlns=\"http://www.example.org/\"\n"
            "   xmlns:jsdl=\"http://schemas.ggf.org/jsdl/2005/11/jsdl\"\n"
            "   xmlns:jsdl-posix=\"http://schemas.ggf.org/jsdl/2005/11/jsdl-posix\"\n"
            "   xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">\n"
            " <jsdl:JobDescription>\n"
            "  <jsdl:JobIdentification>\n"
            "   <jsdl:JobName>%s</jsdl:JobName>\n"
            "  </jsdl:JobIdentification>\n"
            "  <jsdl:Application>\n"
            "   <jsdl-posix:POSIXApplication>\n",
            job->template.name);

    snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
            "    <jsdl-posix:Executable>./%s</jsdl-posix:Executable>\n",
            wrapper);
    strcat(jsdl_buffer, tmp_buffer);
    free(wrapper);

    snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
            "    <jsdl-posix:Output>stdout.wrapper.%d</jsdl-posix:Output>\n"
            "    <jsdl-posix:Error>stderr.wrapper.%d</jsdl-posix:Error>\n"
            "%s",
            job->restarted,
            job->restarted,
            job_environment);
    strcat(jsdl_buffer, tmp_buffer);

    if ((job->max_time > 0) && (job->max_walltime == 0))
    {
            snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
                    "    <jsdl-posix:WallTimeLimit>%d</jsdl-posix:WallTimeLimit>\n",
                    job->max_time*60);
            strcat(jsdl_buffer, tmp_buffer);
    }
    if (job->max_walltime > 0)
    {
            snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
                    "    <jsdl-posix:WallTimeLimit>%d</jsdl-posix:WallTimeLimit>\n",
                    job->max_walltime*60);
            strcat(jsdl_buffer, tmp_buffer);
    }
    if (job->max_memory > 0)
    {
            snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
                    "    <jsdl-posix:MemoryLimit>%d</jsdl-posix:MemoryLimit>\n",
                    job->max_memory*1024);
            strcat(jsdl_buffer, tmp_buffer);
    }
    if (job->max_cpu_time > 0)
    {
            snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
                    "    <jsdl-posix:CPUTimeLimit>%d</jsdl-posix:CPUTimeLimit>\n",
                    job->max_cpu_time*60);
            strcat(jsdl_buffer, tmp_buffer);
    }

    snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
            "   </jsdl-posix:POSIXApplication>\n"
            "  </jsdl:Application>\n");
    strcat(jsdl_buffer, tmp_buffer);

    staging = gw_em_jsdl_staging(job);
    snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
            "%s\n"
            " </jsdl:JobDescription>\n"
            "</jsdl:JobDefinition>\n",
            staging);
    strcat(jsdl_buffer, tmp_buffer);

    if (strlen(jsdl_buffer) + 6 > 4*GW_RSL_LENGTH)
        return NULL;

    jsdl = strdup(jsdl_buffer);
    free(job_environment);
    free(staging);
    return jsdl;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

char *gw_em_jsdl_staging(gw_job_t *job)
{
    char *staging;
    char jsdl_buffer[4*GW_RSL_LENGTH];
    char tmp_buffer[4*GW_RSL_LENGTH];
    gw_conf_t gw_conf;
    char *wrapper;
    int i;
    char *dest_file;

    wrapper = strrchr(job->template.wrapper, '/');
    wrapper = strdup(strtok(wrapper, "/"));

    if (job->history->tm_mad->url == NULL)
        return NULL;

    /* ---------------------------------------------------------------------- */
    /*     wrapper                                                            */
    /* ---------------------------------------------------------------------- */

    strcpy(jsdl_buffer, "");
    snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
            "  <jsdl:DataStaging>\n"
            "   <jsdl:FileName>%s</jsdl:FileName>\n"
            "   <jsdl:CreationFlag>overwrite</jsdl:CreationFlag>\n"
            "   <jsdl:Source>\n"
            "    <jsdl:URI>%s/%s</jsdl:URI>\n"
            "   </jsdl:Source>\n"
            "  </jsdl:DataStaging>\n",
            wrapper,
            job->history->tm_mad->url, job->template.wrapper);
    strcat(jsdl_buffer, tmp_buffer);
    free(wrapper);

    /* ---------------------------------------------------------------------- */
    /*     job.env                                                            */
    /* ---------------------------------------------------------------------- */

    gw_conf.gw_location = getenv("GW_LOCATION");
    snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
        "  <jsdl:DataStaging>\n"
        "   <jsdl:FileName>job.env</jsdl:FileName>\n"
        "   <jsdl:CreationFlag>overwrite</jsdl:CreationFlag>\n"
        "   <jsdl:Source>\n"
        "    <jsdl:URI>%s/" GW_VAR_DIR "/%d/job.env</jsdl:URI>\n"
        "   </jsdl:Source>\n"
        "  </jsdl:DataStaging>\n",
        job->history->tm_mad->url, job->id);
    strcat(jsdl_buffer, tmp_buffer);

    /* ---------------------------------------------------------------------- */
    /*     stdout.wrapper and stderr.wrapper                                  */
    /* ---------------------------------------------------------------------- */

    snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
            "  <jsdl:DataStaging>\n"
            "   <jsdl:FileName>stdout.wrapper.%d</jsdl:FileName>\n"
            "   <jsdl:CreationFlag>overwrite</jsdl:CreationFlag>\n"
            "   <jsdl:Target>\n"
            "    <jsdl:URI>%s/" GW_VAR_DIR "/%d/stdout.wrapper.%d</jsdl:URI>\n"
            "   </jsdl:Target>\n"
            "  </jsdl:DataStaging>\n",
            job->restarted,
            job->history->tm_mad->url, job->id, job->restarted);
    strcat(jsdl_buffer, tmp_buffer);

    snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
            "  <jsdl:DataStaging>\n"
            "   <jsdl:FileName>stderr.wrapper.%d</jsdl:FileName>\n"
            "   <jsdl:CreationFlag>overwrite</jsdl:CreationFlag>\n"
            "   <jsdl:Target>\n"
            "    <jsdl:URI>%s/" GW_VAR_DIR "/%d/stderr.wrapper.%d</jsdl:URI>\n"
            "   </jsdl:Target>\n"
            "  </jsdl:DataStaging>\n",
            job->restarted,
            job->history->tm_mad->url, job->id, job->restarted);
    strcat(jsdl_buffer, tmp_buffer);

    /* ---------------------------------------------------------------------- */
    /*     stdin.execution                                                    */
    /* ---------------------------------------------------------------------- */

    dest_file = strrchr(job->template.stdin_file, '/');
    if (dest_file == NULL)
        dest_file = strdup(gw_job_substitute(job->template.stdin_file, job));
    else
        dest_file = strdup(strtok(dest_file, "/"));

    if (strncmp(job->template.stdin_file, "gsiftp", 6) == 0)
    {
        snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
            "  <jsdl:DataStaging>\n"
            "   <jsdl:FileName>%s</jsdl:FileName>\n"
            "   <jsdl:CreationFlag>overwrite</jsdl:CreationFlag>\n"
            "   <jsdl:Source>\n"
            "    <jsdl:URI>%s</jsdl:URI>\n"
            "   </jsdl:Source>\n"
            "  </jsdl:DataStaging>\n",
            dest_file,
            gw_job_substitute(job->template.stdin_file, job));
    }
    else if (strncmp(job->template.stdin_file, "file", 4) == 0)
    {
        snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
            "  <jsdl:DataStaging>\n"
            "   <jsdl:FileName>%s</jsdl:FileName>\n"
            "   <jsdl:CreationFlag>overwrite</jsdl:CreationFlag>\n"
            "   <jsdl:Source>\n"
            "    <jsdl:URI>%s/%s</jsdl:URI>\n"
            "   </jsdl:Source>\n"
            "  </jsdl:DataStaging>\n",
            dest_file,
            job->history->tm_mad->url, gw_job_substitute(&job->template.stdin_file[7], job));
    }
    else if (strncmp(job->template.stdin_file, "/", 1) != 0)
    {
        snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
            "  <jsdl:DataStaging>\n"
            "   <jsdl:FileName>%s</jsdl:FileName>\n"
            "   <jsdl:CreationFlag>overwrite</jsdl:CreationFlag>\n"
            "   <jsdl:Source>\n"
            "    <jsdl:URI>%s/%s/%s</jsdl:URI>\n"
            "   </jsdl:Source>\n"
            "  </jsdl:DataStaging>\n",
            dest_file,
            job->history->tm_mad->url, job->template.job_home, gw_job_substitute(job->template.stdin_file, job));
    }
    if (strncmp(job->template.stdin_file, "/", 1) != 0)
        strcat(jsdl_buffer, tmp_buffer);

    /* ---------------------------------------------------------------------- */
    /*     executable                                                         */
    /* ---------------------------------------------------------------------- */

    dest_file = strrchr(job->template.executable, '/');
    if (dest_file == NULL)
        dest_file = strdup(gw_job_substitute(job->template.executable, job));
    else
        dest_file = strdup(strtok(dest_file, "/"));

    if (strncmp(job->template.executable, "gsiftp", 6) == 0)
    {
        snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
            "  <jsdl:DataStaging>\n"
            "   <jsdl:FileName>%s</jsdl:FileName>\n"
            "   <jsdl:CreationFlag>overwrite</jsdl:CreationFlag>\n"
            "   <jsdl:Source>\n"
            "    <jsdl:URI>%s</jsdl:URI>\n"
            "   </jsdl:Source>\n"
            "  </jsdl:DataStaging>\n",
            dest_file,
            gw_job_substitute(job->template.executable, job));
    }
    else if (strncmp(job->template.executable, "file", 4) == 0)
    {
        snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
            "  <jsdl:DataStaging>\n"
            "   <jsdl:FileName>%s</jsdl:FileName>\n"
            "   <jsdl:CreationFlag>overwrite</jsdl:CreationFlag>\n"
            "   <jsdl:Source>\n"
            "    <jsdl:URI>%s/%s</jsdl:URI>\n"
            "   </jsdl:Source>\n"
            "  </jsdl:DataStaging>\n",
            dest_file,
            job->history->tm_mad->url, gw_job_substitute(&job->template.executable[7], job));
    }
    else if (strncmp(job->template.executable, "/", 1) != 0)
    {
        snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
            "  <jsdl:DataStaging>\n"
            "   <jsdl:FileName>%s</jsdl:FileName>\n"
            "   <jsdl:CreationFlag>overwrite</jsdl:CreationFlag>\n"
            "   <jsdl:Source>\n"
            "    <jsdl:URI>%s/%s/%s</jsdl:URI>\n"
            "   </jsdl:Source>\n"
            "  </jsdl:DataStaging>\n",
            dest_file,
            job->history->tm_mad->url, job->template.job_home, gw_job_substitute(job->template.executable, job));
    }
    if (strncmp(job->template.executable, "/", 1) != 0)
        strcat(jsdl_buffer, tmp_buffer);

    /* ---------------------------------------------------------------------- */
    /*     stdout.execution and stderr.execution                              */
    /* ---------------------------------------------------------------------- */

    if (strncmp(job->template.stdout_file, "gsiftp", 6) == 0)
    {
        snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
            "  <jsdl:DataStaging>\n"
            "   <jsdl:FileName>stdout.execution</jsdl:FileName>\n"
            "   <jsdl:CreationFlag>overwrite</jsdl:CreationFlag>\n"
            "   <jsdl:Target>\n"
            "    <jsdl:URI>%s</jsdl:URI>\n"
            "   </jsdl:Target>\n"
            "  </jsdl:DataStaging>\n",
            gw_job_substitute(job->template.stdout_file, job));
    }
    else if (strncmp(job->template.stdout_file, "file", 4) == 0)
    {   
        snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
            "  <jsdl:DataStaging>\n"
            "   <jsdl:FileName>stdout.execution</jsdl:FileName>\n"
            "   <jsdl:CreationFlag>overwrite</jsdl:CreationFlag>\n"
            "   <jsdl:Target>\n"
            "    <jsdl:URI>%s/%s</jsdl:URI>\n"
            "   </jsdl:Target>\n"
            "  </jsdl:DataStaging>\n",
            job->history->tm_mad->url, gw_job_substitute(&job->template.stdout_file[7], job));
    }
    else if (strncmp(job->template.stdout_file, "/", 1) == 0)
    {
        snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
            "  <jsdl:DataStaging>\n"
            "   <jsdl:FileName>stdout.execution</jsdl:FileName>\n"
            "   <jsdl:CreationFlag>overwrite</jsdl:CreationFlag>\n"
            "   <jsdl:Target>\n"
            "    <jsdl:URI>%s/%s</jsdl:URI>\n"
            "   </jsdl:Target>\n"
            "  </jsdl:DataStaging>\n",
            job->history->tm_mad->url, gw_job_substitute(job->template.stdout_file, job));
    }
    else
    {
        snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
            "  <jsdl:DataStaging>\n"
            "   <jsdl:FileName>stdout.execution</jsdl:FileName>\n"
            "   <jsdl:CreationFlag>overwrite</jsdl:CreationFlag>\n"
            "   <jsdl:Target>\n"
            "    <jsdl:URI>%s/%s/%s</jsdl:URI>\n"
            "   </jsdl:Target>\n"
            "  </jsdl:DataStaging>\n",
            job->history->tm_mad->url, job->template.job_home, gw_job_substitute(job->template.stdout_file, job));

    }
    strcat(jsdl_buffer, tmp_buffer);

    if (strncmp(job->template.stderr_file, "gsiftp", 6) == 0)
    {
        snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
            "  <jsdl:DataStaging>\n"
            "   <jsdl:FileName>stderr.execution</jsdl:FileName>\n"
            "   <jsdl:CreationFlag>overwrite</jsdl:CreationFlag>\n"
            "   <jsdl:Target>\n"
            "    <jsdl:URI>%s</jsdl:URI>\n"
            "   </jsdl:Target>\n"
            "  </jsdl:DataStaging>\n",
            gw_job_substitute(job->template.stderr_file, job));
    }
    else if (strncmp(job->template.stderr_file, "file", 4) == 0)
    {
        snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
            "  <jsdl:DataStaging>\n"
            "   <jsdl:FileName>stderr.execution</jsdl:FileName>\n"
            "   <jsdl:CreationFlag>overwrite</jsdl:CreationFlag>\n"
            "   <jsdl:Target>\n"
            "    <jsdl:URI>%s/%s</jsdl:URI>\n"
            "   </jsdl:Target>\n"
            "  </jsdl:DataStaging>\n",
            job->history->tm_mad->url, gw_job_substitute(&job->template.stderr_file[7], job));
    }
    else if (strncmp(job->template.stderr_file, "/", 1) == 0)
    {
        snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
            "  <jsdl:DataStaging>\n"
            "   <jsdl:FileName>stderr.execution</jsdl:FileName>\n"
            "   <jsdl:CreationFlag>overwrite</jsdl:CreationFlag>\n"
            "   <jsdl:Target>\n"
            "    <jsdl:URI>%s/%s</jsdl:URI>\n"
            "   </jsdl:Target>\n"
            "  </jsdl:DataStaging>\n",
            job->history->tm_mad->url, gw_job_substitute(job->template.stderr_file, job));
    }
    else
    {
        snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
            "  <jsdl:DataStaging>\n"
            "   <jsdl:FileName>stderr.execution</jsdl:FileName>\n"
            "   <jsdl:CreationFlag>overwrite</jsdl:CreationFlag>\n"
            "   <jsdl:Target>\n"
            "    <jsdl:URI>%s/%s/%s</jsdl:URI>\n"
            "   </jsdl:Target>\n"
            "  </jsdl:DataStaging>\n",
            job->history->tm_mad->url, job->template.job_home, gw_job_substitute(job->template.stderr_file, job));
    }
    strcat(jsdl_buffer, tmp_buffer);

    /* ---------------------------------------------------------------------- */
    /*     input files                                                        */
    /* ---------------------------------------------------------------------- */

    for (i=0; i<job->template.num_input_files; i++)
    {
        if (job->template.input_files[i][1] == NULL)
        {
            dest_file = strrchr(job->template.input_files[i][0], '/');
            if (dest_file == NULL)
                dest_file = strdup(gw_job_substitute(job->template.input_files[i][0], job));
            else
                dest_file = strdup(strtok(dest_file, "/"));
        }
        else
            dest_file = strdup(gw_job_substitute(job->template.input_files[i][1], job));
   
        if (strncmp(job->template.input_files[i][0], "gsiftp", 6) == 0)
        {
            snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
            "  <jsdl:DataStaging>\n"
            "   <jsdl:FileName>%s</jsdl:FileName>\n"
            "   <jsdl:CreationFlag>overwrite</jsdl:CreationFlag>\n"
            "   <jsdl:Source>\n"
            "    <jsdl:URI>%s</jsdl:URI>\n"
            "   </jsdl:Source>\n"
            "  </jsdl:DataStaging>\n",
            dest_file,
            gw_job_substitute(job->template.input_files[i][0], job));
        }
        else if (strncmp(job->template.input_files[i][0], "file", 4) == 0)
        {
            snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
            "  <jsdl:DataStaging>\n"
            "   <jsdl:FileName>%s</jsdl:FileName>\n"
            "   <jsdl:CreationFlag>overwrite</jsdl:CreationFlag>\n"
            "   <jsdl:Source>\n"
            "    <jsdl:URI>%s/%s</jsdl:URI>\n"
            "   </jsdl:Source>\n"
            "  </jsdl:DataStaging>\n",
            dest_file,
            job->history->tm_mad->url, gw_job_substitute(&job->template.input_files[i][0][7], job));
        }
        else if (strncmp(job->template.input_files[i][0], "/", 1) != 0)
        {
            snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
            "  <jsdl:DataStaging>\n"
            "   <jsdl:FileName>%s</jsdl:FileName>\n"
            "   <jsdl:CreationFlag>overwrite</jsdl:CreationFlag>\n"
            "   <jsdl:Source>\n"
            "    <jsdl:URI>%s/%s/%s</jsdl:URI>\n"
            "   </jsdl:Source>\n"
            "  </jsdl:DataStaging>\n",
            dest_file,
            job->history->tm_mad->url, job->template.job_home, gw_job_substitute(job->template.input_files[i][0], job));
        }
        if (strncmp(job->template.input_files[i][0], "/", 1) != 0)
            strcat(jsdl_buffer, tmp_buffer);
    }
    free(dest_file);

    /* ---------------------------------------------------------------------- */
    /*     output files                                                       */
    /* ---------------------------------------------------------------------- */

    for (i=0; i<job->template.num_output_files; i++)
    {
        if (job->template.output_files[i][1]== NULL)
        {
            snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
            "  <jsdl:DataStaging>\n"
            "   <jsdl:FileName>%s</jsdl:FileName>\n"
            "   <jsdl:CreationFlag>overwrite</jsdl:CreationFlag>\n"
            "   <jsdl:Target>\n"
            "    <jsdl:URI>%s/%s/%s</jsdl:URI>\n"
            "   </jsdl:Target>\n"
            "  </jsdl:DataStaging>\n",
            gw_job_substitute(job->template.output_files[i][0], job),
            job->history->tm_mad->url, job->template.job_home, gw_job_substitute(job->template.output_files[i][0], job)); 
        }
        else if (strncmp(job->template.output_files[i][1], "gsiftp", 6) == 0)
        {
            snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
            "  <jsdl:DataStaging>\n"
            "   <jsdl:FileName>%s</jsdl:FileName>\n"
            "   <jsdl:CreationFlag>overwrite</jsdl:CreationFlag>\n"
            "   <jsdl:Target>\n"
            "    <jsdl:URI>%s</jsdl:URI>\n"
            "   </jsdl:Target>\n"
            "  </jsdl:DataStaging>\n",
            gw_job_substitute(job->template.output_files[i][0], job),
            gw_job_substitute(job->template.output_files[i][1], job));
        }
        else if (strncmp(job->template.output_files[i][1], "file", 4) == 0)
        {
            snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
            "  <jsdl:DataStaging>\n"
            "   <jsdl:FileName>%s</jsdl:FileName>\n"
            "   <jsdl:CreationFlag>overwrite</jsdl:CreationFlag>\n"
            "   <jsdl:Target>\n"
            "    <jsdl:URI>%s/%s</jsdl:URI>\n"
            "   </jsdl:Target>\n"
            "  </jsdl:DataStaging>\n",
            gw_job_substitute(job->template.output_files[i][0], job),
            job->history->tm_mad->url, gw_job_substitute(&job->template.output_files[i][1][7], job));
        }
        else if (strncmp(job->template.output_files[i][1], "/", 1) == 0)
        {
            snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
            "  <jsdl:DataStaging>\n"
            "   <jsdl:FileName>%s</jsdl:FileName>\n"
            "   <jsdl:CreationFlag>overwrite</jsdl:CreationFlag>\n"
            "   <jsdl:Target>\n"
            "    <jsdl:URI>%s/%s</jsdl:URI>\n"
            "   </jsdl:Target>\n"
            "  </jsdl:DataStaging>\n",
            gw_job_substitute(job->template.output_files[i][0], job),
            job->history->tm_mad->url, gw_job_substitute(job->template.output_files[i][1], job));
        }
        else
        {
            snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
            "  <jsdl:DataStaging>\n"
            "   <jsdl:FileName>%s</jsdl:FileName>\n"
            "   <jsdl:CreationFlag>overwrite</jsdl:CreationFlag>\n"
            "   <jsdl:Target>\n"
            "    <jsdl:URI>%s/%s/%s</jsdl:URI>\n"
            "   </jsdl:Target>\n"
            "  </jsdl:DataStaging>\n",
            gw_job_substitute(job->template.output_files[i][0], job),
            job->history->tm_mad->url, job->template.job_home, gw_job_substitute(job->template.output_files[i][1], job));
        }
        strcat(jsdl_buffer, tmp_buffer);
    }

    if (strlen(jsdl_buffer) + 6 > 4*GW_RSL_LENGTH)
        return NULL;

    staging = strdup(jsdl_buffer);
    return staging;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

char *gw_em_jsdl_environment(gw_job_t *job)
{
    char jsdl_buffer[4*GW_RSL_LENGTH];
    char tmp_buffer[4*GW_RSL_LENGTH];
    char *jsdl_env;
    int rc;
    int i;

    strcpy(jsdl_buffer, "");
    for (i=0;i<job->template.num_env;i++)
    {
        if (strcmp(job->template.environment[i][0], "MAXCPUTIME") != 0 && strcmp(job->template.environment[i][0], "MAXTIME") != 0 && strcmp(job->template.environment[i][0], "MAXWALLTIME") != 0 && strcmp(job->template.environment[i][0], "MAXMEMORY") != 0 && strcmp(job->template.environment[i][0], "MINMEMORY") != 0) 
        {
            rc = snprintf(jsdl_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
                "    <jsdl-posix:Environment name=\"%s\">%s</jsdl-posix:Environment>\n",
                job->template.environment[i][0],
                job->template.environment[i][1]);
        }
    }

    rc = snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
            "    <jsdl-posix:Environment name=\"GW_HOSTNAME\">%s</jsdl-posix:Environment>\n"
            "    <jsdl-posix:Environment name=\"GW_USER\">%s</jsdl-posix:Environment>\n"
            "    <jsdl-posix:Environment name=\"GW_JOB_ID\">%i</jsdl-posix:Environment>\n"
            "    <jsdl-posix:Environment name=\"GW_TASK_ID\">%i</jsdl-posix:Environment>\n"
            "    <jsdl-posix:Environment name=\"GW_ARRAY_ID\">%i</jsdl-posix:Environment>\n"
            "    <jsdl-posix:Environment name=\"GW_TOTAL_TASKS\">%i</jsdl-posix:Environment>\n"
            "    <jsdl-posix:Environment name=\"GW_RESTARTED\">%i</jsdl-posix:Environment>\n"
            "    <jsdl-posix:Environment name=\"HOME\" filesystemName=\"HOME\"/>\n",
            job->history->host->hostname,
            job->owner,
            job->id,
            job->task_id,
            job->array_id,
            job->total_tasks,
            job->restarted);
    strcat(jsdl_buffer, tmp_buffer);

    if ((rc >= 4*GW_RSL_LENGTH ) || ( rc < 0 ) )
        return NULL;

    jsdl_env = strdup(jsdl_buffer);
    return jsdl_env;
}
