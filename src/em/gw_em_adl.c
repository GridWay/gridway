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


char *gw_em_adl_environment(gw_job_t *job);
char *gw_em_adl_staging(gw_job_t *job);

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

char* gw_generate_wrapper_adl(gw_job_t *job)
{
    char *adl;
    char *job_environment;
    char *staging;
    char adl_buffer[4*GW_RSL_LENGTH];
    char tmp_buffer[4*GW_RSL_LENGTH];
    char *wrapper;

    wrapper = strrchr(job->template.wrapper, '/');
    wrapper = strdup(strtok(wrapper, "/"));

    /* ---------------------------------------------------------------------- */
    /* 1.- Create dynamic job data environment                                */
    /* ---------------------------------------------------------------------- */

    job_environment = gw_em_adl_environment(job);

    if ( job_environment == NULL )
        return NULL;

    /* ---------------------------------------------------------------------- */
    /* 2.- Build JSDL String & Return it                                       */
    /* ---------------------------------------------------------------------- */

    snprintf(adl_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
            "<ActivityDescription xmlns=\"http://www.eu-emi.eu/es/2010/12/adl\">\n"
            " <ActivityIdentification>\n"
            "  <Name>%s</Name>\n"
            " </ActivityIdentification>\n"
            " <Application>\n",
            job->template.name);

    snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
            "  <Executable>\n"
            "   <Path>%s</Path>\n"
            "  </Executable>\n"
            "  <Output>stdout.wrapper.%d</Output>\n"
            "  <Error>stderr.wrapper.%d</Error>\n"
            "%s",
            wrapper,
            job->restarted,
            job->restarted,
            job_environment);
    strcat(adl_buffer, tmp_buffer);
    free(wrapper);

    snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
            " </Application>\n"
            " <Resources>\n");
    strcat(adl_buffer, tmp_buffer);

    if ((job->max_time > 0) && (job->max_walltime == 0))
    {
            snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
                    "    <WallTime>%d</WallTime>\n",
                    job->max_time*60);
            strcat(adl_buffer, tmp_buffer);
    }
    if (job->max_walltime > 0)
    {
            snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
                    "    <WallTime>%d</WallTime>\n",
                    job->max_walltime*60);
            strcat(adl_buffer, tmp_buffer);
    }
    if (job->max_memory > 0)
    {
            snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
                    "    <IndividualPhysicalMemory>%d</IndividualPhysicalMemory>\n",
                    job->max_memory*1024);
            strcat(adl_buffer, tmp_buffer);
    }
    if (job->max_cpu_time > 0)
    {
            snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
                    "    <TotalCPUTime>%d</TotalCPUTime>\n",
                    job->max_cpu_time*60);
            strcat(adl_buffer, tmp_buffer);
    }

    snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
            " </Resources>\n");
    strcat(adl_buffer, tmp_buffer);

    staging = gw_em_adl_staging(job);
    snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
            "%s\n"
            "</ActivityDescription>\n",
            staging);
    strcat(adl_buffer, tmp_buffer);

    if (strlen(adl_buffer) + 6 > 4*GW_RSL_LENGTH)
        return NULL;

    adl = strdup(adl_buffer);
    free(job_environment);
    free(staging);
    return adl;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

char *gw_em_adl_staging(gw_job_t *job)
{
    char *staging;
    char adl_buffer[4*GW_RSL_LENGTH];
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

    strcpy(adl_buffer, "");
    snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
            " <DataStaging>\n"
            "  <InputFile>\n"
            "   <Name>%s</Name>\n"
            "   <Source>\n"
            "    <URI>%s/%s</URI>\n"
            "   </Source>\n"
            "  </InputFile>\n",
            wrapper,
            job->history->tm_mad->url, job->template.wrapper);
    strcat(adl_buffer, tmp_buffer);
    free(wrapper);

    /* ---------------------------------------------------------------------- */
    /*     job.env                                                            */
    /* ---------------------------------------------------------------------- */

    gw_conf.gw_location = getenv("GW_LOCATION");
    snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
        "  <InputFile>\n"
        "   <Name>job.env</Name>\n"
        "   <Source>\n"
        "    <URI>%s/%s/" GW_VAR_DIR "/%d/job.env</URI>\n"
        "   </Source>\n"
        "  </InputFile>\n",
        job->history->tm_mad->url, gw_conf.gw_location, job->id);
    strcat(adl_buffer, tmp_buffer);

    /* ---------------------------------------------------------------------- */
    /*     stdout.wrapper and stderr.wrapper                                  */
    /* ---------------------------------------------------------------------- */

    snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
            "  <OutputFile>\n"
            "   <Name>stdout.wrapper.%d</Name>\n"
            "   <Target>\n"
            "    <URI>%s/%s/" GW_VAR_DIR "/%d/stdout.wrapper.%d</URI>\n"
            "   </Target>\n"
            "  </OutputFile>\n",
            job->restarted,
            job->history->tm_mad->url, gw_conf.gw_location, job->id, job->restarted);
    strcat(adl_buffer, tmp_buffer);

    snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
            "  <OutputFile>\n"
            "   <Name>stderr.wrapper.%d</Name>\n"
            "   <Target>\n"
            "    <URI>%s/%s/" GW_VAR_DIR "/%d/stderr.wrapper.%d</URI>\n"
            "   </Target>\n"
            "  </OutputFile>\n",
            job->restarted,
            job->history->tm_mad->url, gw_conf.gw_location, job->id, job->restarted);
    strcat(adl_buffer, tmp_buffer);

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
            "  <InputFile>\n"
            "   <Name>%s</Name>\n"
            "   <Source>\n"
            "    <URI>%s</URI>\n"
            "   </Source>\n"
            "  </InputFile>\n",
            dest_file,
            gw_job_substitute(job->template.stdin_file, job));
    }
    else if (strncmp(job->template.stdin_file, "file", 4) == 0)
    {
        snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
            "  <InputFile>\n"
            "   <Name>%s</Name>\n"
            "   <Source>\n"
            "    <URI>%s/%s</URI>\n"
            "   </Source>\n"
            "  </InputFile>\n",
            dest_file,
            job->history->tm_mad->url, gw_job_substitute(&job->template.stdin_file[7], job));
    }
    else if (strncmp(job->template.stdin_file, "/", 1) != 0)
    {
        snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
            "  <InputFile>\n"
            "   <Name>%s</Name>\n"
            "   <Source>\n"
            "    <URI>%s/%s/%s</URI>\n"
            "   </Source>\n"
            "  </InputFile>\n",
            dest_file,
            job->history->tm_mad->url, job->template.job_home, gw_job_substitute(job->template.stdin_file, job));
    }
    if (strncmp(job->template.stdin_file, "/", 1) != 0)
        strcat(adl_buffer, tmp_buffer);

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
            "  <InputFile>\n"
            "   <Name>%s</Name>\n"
            "   <Source>\n"
            "    <URI>%s</URI>\n"
            "   </Source>\n"
            "   <IsExecutable>true</IsExecutable>\n"
            "  </InputFile>\n",
            dest_file,
            gw_job_substitute(job->template.executable, job));
    }
    else if (strncmp(job->template.executable, "file", 4) == 0)
    {
        snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
            "  <InputFile>\n"
            "   <Name>%s</Name>\n"
            "   <Source>\n"
            "    <URI>%s/%s</URI>\n"
            "   </Source>\n"
            "   <IsExecutable>true</IsExecutable>\n"
            "  </InputFile>\n",
            dest_file,
            job->history->tm_mad->url, gw_job_substitute(&job->template.executable[7], job));
    }
    else if (strncmp(job->template.executable, "/", 1) != 0)
    {
        snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
            "  <InputFile>\n"
            "   <Name>%s</Name>\n"
            "   <Source>\n"
            "    <URI>%s/%s/%s</URI>\n"
            "   </Source>\n"
            "   <IsExecutable>true</IsExecutable>\n"
            "  </InputFile>\n",
            dest_file,
            job->history->tm_mad->url, job->template.job_home, gw_job_substitute(job->template.executable, job));
    }
    if (strncmp(job->template.executable, "/", 1) != 0)
        strcat(adl_buffer, tmp_buffer);

    /* ---------------------------------------------------------------------- */
    /*     stdout.execution and stderr.execution                              */
    /* ---------------------------------------------------------------------- */

    if (strncmp(job->template.stdout_file, "gsiftp", 6) == 0)
    {
        snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
            "  <OutputFile>\n"
            "   <Name>stdout.execution</Name>\n"
            "   <Target>\n"
            "    <URI>%s</URI>\n"
            "   </Target>\n"
            "  </OutputFile>\n",
            gw_job_substitute(job->template.stdout_file, job));
    }
    else if (strncmp(job->template.stdout_file, "file", 4) == 0)
    {   
        snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
            "  <OutputFile>\n"
            "   <Name>stdout.execution</Name>\n"
            "   <Target>\n"
            "    <URI>%s/%s</URI>\n"
            "   </Target>\n"
            "  </OutputFile>\n",
            job->history->tm_mad->url, gw_job_substitute(&job->template.stdout_file[7], job));
    }
    else if (strncmp(job->template.stdout_file, "/", 1) == 0)
    {
        snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
            "  <OutputFile>\n"
            "   <Name>stdout.execution</Name>\n"
            "   <Target>\n"
            "    <URI>%s/%s</URI>\n"
            "   </Target>\n"
            "  </OutputFile>\n",
            job->history->tm_mad->url, gw_job_substitute(job->template.stdout_file, job));
    }
    else
    {
        snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
            "  <OutputFile>\n"
            "   <Name>stdout.execution</Name>\n"
            "   <Target>\n"
            "    <URI>%s/%s/%s</URI>\n"
            "   </Target>\n"
            "  </OutputFile>\n",
            job->history->tm_mad->url, job->template.job_home, gw_job_substitute(job->template.stdout_file, job));

    }
    strcat(adl_buffer, tmp_buffer);

    if (strncmp(job->template.stderr_file, "gsiftp", 6) == 0)
    {
        snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
            "  <OutputFile>\n"
            "   <Name>stderr.execution</Name>\n"
            "   <Target>\n"
            "    <URI>%s</URI>\n"
            "   </Target>\n"
            "  </OutputFile>\n",
            gw_job_substitute(job->template.stderr_file, job));
    }
    else if (strncmp(job->template.stderr_file, "file", 4) == 0)
    {
        snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
            "  <OutputFile>\n"
            "   <Name>stderr.execution</Name>\n"
            "   <Target>\n"
            "    <URI>%s/%s</URI>\n"
            "   </Target>\n"
            "  <OutputFile>\n",
            job->history->tm_mad->url, gw_job_substitute(&job->template.stderr_file[7], job));
    }
    else if (strncmp(job->template.stderr_file, "/", 1) == 0)
    {
        snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
            "  <OutputFile>\n"
            "   <Name>stderr.execution</Name>\n"
            "   <Target>\n"
            "    <URI>%s/%s</URI>\n"
            "   </Target>\n"
            "  </OutputFile>\n",
            job->history->tm_mad->url, gw_job_substitute(job->template.stderr_file, job));
    }
    else
    {
        snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
            "  <OutputFile>\n"
            "   <Name>stderr.execution</Name>\n"
            "   <Target>\n"
            "    <URI>%s/%s/%s</URI>\n"
            "   </Target>\n"
            "  </OutputFile>\n",
            job->history->tm_mad->url, job->template.job_home, gw_job_substitute(job->template.stderr_file, job));
    }
    strcat(adl_buffer, tmp_buffer);

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
            "  <InputFile>\n"
            "   <Name>%s</Name>\n"
            "   <Source>\n"
            "    <URI>%s</URI>\n"
            "   </Source>\n"
            "  </InputFile>\n",
            dest_file,
            gw_job_substitute(job->template.input_files[i][0], job));
        }
        else if (strncmp(job->template.input_files[i][0], "file", 4) == 0)
        {
            snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
            "  <InputFile>\n"
            "   <Name>%s</Name>\n"
            "   <Source>\n"
            "    <URI>%s/%s</URI>\n"
            "   </Source>\n"
            "  </InputFile>\n",
            dest_file,
            job->history->tm_mad->url, gw_job_substitute(&job->template.input_files[i][0][7], job));
        }
        else if (strncmp(job->template.input_files[i][0], "/", 1) != 0)
        {
            snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
            "  <InputFile>\n"
            "   <Name>%s</Name>\n"
            "   <Source>\n"
            "    <URI>%s/%s/%s</URI>\n"
            "   </Source>\n"
            "  </InputFile>\n",
            dest_file,
            job->history->tm_mad->url, job->template.job_home, gw_job_substitute(job->template.input_files[i][0], job));
        }
        if (strncmp(job->template.input_files[i][0], "/", 1) != 0)
            strcat(adl_buffer, tmp_buffer);
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
            "  <OutputFile>\n"
            "   <Name>%s</Name>\n"
            "   <Target>\n"
            "    <URI>%s/%s/%s</URI>\n"
            "   </Target>\n"
            "  </OutputFile>\n",
            gw_job_substitute(job->template.output_files[i][0], job),
            job->history->tm_mad->url, job->template.job_home, gw_job_substitute(job->template.output_files[i][0], job)); 
        }
        else if (strncmp(job->template.output_files[i][1], "gsiftp", 6) == 0)
        {
            snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
            "  <OutputFile>\n"
            "   <Name>%s</Name>\n"
            "   <Target>\n"
            "    <URI>%s</URI>\n"
            "   </Target>\n"
            "  </OutputFile>\n",
            gw_job_substitute(job->template.output_files[i][0], job),
            gw_job_substitute(job->template.output_files[i][1], job));
        }
        else if (strncmp(job->template.output_files[i][1], "file", 4) == 0)
        {
            snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
            "  <OutputFile>\n"
            "   <Name>%s</Name>\n"
            "   <Target>\n"
            "    <URI>%s/%s</URI>\n"
            "   </Target>\n"
            "  </OutputFile>\n",
            gw_job_substitute(job->template.output_files[i][0], job),
            job->history->tm_mad->url, gw_job_substitute(&job->template.output_files[i][1][7], job));
        }
        else if (strncmp(job->template.output_files[i][1], "/", 1) == 0)
        {
            snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
            "  <OutputFile>\n"
            "   <Name>%s</Name>\n"
            "   <Target>\n"
            "    <URI>%s/%s</URI>\n"
            "   </Target>\n"
            "  </OutputFile>\n",
            gw_job_substitute(job->template.output_files[i][0], job),
            job->history->tm_mad->url, gw_job_substitute(job->template.output_files[i][1], job));
        }
        else
        {
            snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
            "  <OutputFile>\n"
            "   <Name>%s</Name>\n"
            "   <Target>\n"
            "    <URI>%s/%s/%s</URI>\n"
            "   </Target>\n"
            "  </OutputFile>\n",
            gw_job_substitute(job->template.output_files[i][0], job),
            job->history->tm_mad->url, job->template.job_home, gw_job_substitute(job->template.output_files[i][1], job));
        }
        strcat(adl_buffer, tmp_buffer);
    }

    snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
    " </DataStaging>");
    strcat(adl_buffer, tmp_buffer);

    if (strlen(adl_buffer) + 6 > 4*GW_RSL_LENGTH)
        return NULL;

    staging = strdup(adl_buffer);
    return staging;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

char *gw_em_adl_environment(gw_job_t *job)
{
    char adl_buffer[4*GW_RSL_LENGTH];
    char tmp_buffer[4*GW_RSL_LENGTH];
    char *adl_env;
    int rc;
    int i;

    strcpy(adl_buffer, "");
    for (i=0;i<job->template.num_env;i++)
    {
        if (strcmp(job->template.environment[i][0], "MAXCPUTIME") != 0 && strcmp(job->template.environment[i][0], "MAXTIME") != 0 && strcmp(job->template.environment[i][0], "MAXWALLTIME") != 0 && strcmp(job->template.environment[i][0], "MAXMEMORY") != 0 && strcmp(job->template.environment[i][0], "MINMEMORY") != 0) 
        {
            rc = snprintf(adl_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
                "  <Environment>\n" 
                "   <Name>%s</Name>\n"
                "   <Value>%s</Value>\n"
                "  </Environment>\n",
                job->template.environment[i][0],
                job->template.environment[i][1]);
        }
    }

    rc = snprintf(tmp_buffer, sizeof(char) * 4*GW_RSL_LENGTH,
            "  <Environment>\n" 
            "   <Name>GW_HOSTNAME</Name>\n"
            "   <Value>%s</Value>\n"
            "  </Environment>\n"
            "  <Environment>\n" 
            "   <Name>GW_USER</Name>\n"
            "   <Value>%s</Value>\n"
            "  </Environment>\n"
            "  <Environment>\n" 
            "   <Name>GW_JOB_ID</Name>\n"
            "   <Value>%i</Value>\n"
            "  </Environment>\n"
            "  <Environment>\n" 
            "   <Name>GW_TASK_ID</Name>\n"
            "   <Value>%i</Value>\n"
            "  </Environment>\n"
            "  <Environment>\n" 
            "   <Name>GW_ARRAY_ID</Name>\n"
            "   <Value>%i</Value>\n"
            "  </Environment>\n"
            "  <Environment>\n" 
            "   <Name>GW_TOTAL_TASKS</Name>\n"
            "   <Value>%i</Value>\n"
            "  </Environment>\n"
            "  <Environment>\n" 
            "   <Name>GW_RESTARTED</Name>\n"
            "   <Value>%i</Value>\n"
            "  </Environment>\n",
            job->history->host->hostname,
            job->owner,
            job->id,
            job->task_id,
            job->array_id,
            job->total_tasks,
            job->restarted);
    strcat(adl_buffer, tmp_buffer);

    if ((rc >= 4*GW_RSL_LENGTH ) || ( rc < 0 ) )
        return NULL;

    adl_env = strdup(adl_buffer);
    return adl_env;
}
