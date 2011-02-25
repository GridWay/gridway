/* -------------------------------------------------------------------------- */
/* Copyright 2002-2010, GridWay Project Leads (GridWay.org)                   */
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
#include <string.h>

#include "gw_em_rsl.h"
#include "gw_job.h"
#include "gw_template.h"
#include "gw_user_pool.h"

char* gw_split_arguments_jsdl(const char *arguments);


/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

char* gw_generate_wrapper_jsdl (gw_job_t *job)
{
    char *jsdl;
    char *job_environment; 
    char jsdl_buffer[GW_RSL_LENGTH];
    char tmp_buffer[GW_RSL_LENGTH];
    int print_queue = 0; 
    int i;
    char *arguments;
    char *xml_arguments;

    char *jobtype;
    char *staging_url;
    gw_conf_t gw_conf;
    char *wrapper;

    wrapper = strrchr(job->template.wrapper, '/');
    wrapper = strtok(wrapper,"/");
    /* ---------------------------------------------------------------------- */
    /* 1.- Create dynamic job data environment                                */
    /* ---------------------------------------------------------------------- */
  
    job_environment = gw_em_jsdl_environment(job);
    
    if ( job_environment == NULL )
        return NULL;

    if (job->history->tm_mad->url != NULL)
    {
        /* Perform staging with the URL provided by the TM MAD */
        staging_url = job->history->tm_mad->url;
    }
    else
    {
        return NULL;
    }

    /* ---------------------------------------------------------------------- */
    /* 2.- Build JSDL String & Return it                                       */
    /* ---------------------------------------------------------------------- */

  	jsdl = snprintf(jsdl_buffer, sizeof(char) * GW_RSL_LENGTH,
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

        jsdl = snprintf(tmp_buffer, sizeof(char) * GW_RSL_LENGTH,
            "    <jsdl-posix:Executable>./%s.wrapper</jsdl-posix:Executable>\n",
            wrapper);
        strcat(jsdl_buffer,tmp_buffer);

        gw_conf.gw_location = getenv("GW_LOCATION");
        jsdl = snprintf(tmp_buffer, sizeof(char) * GW_RSL_LENGTH,
            "    <jsdl-posix:Argument>%s%s/" GW_VAR_DIR "/%d/job.env</jsdl-posix:Argument>\n",
            staging_url, gw_conf.gw_location, job->id);
        strcat(jsdl_buffer,tmp_buffer);


	jsdl = snprintf(tmp_buffer, sizeof(char) * GW_RSL_LENGTH,
            "    <jsdl-posix:Output>stdout.wrapper.%d</jsdl-posix:Output>\n"
            "    <jsdl-posix:Error>stderr.wrapper.%d</jsdl-posix:Error>\n"
            "%s",
            job->restarted,
            job->restarted,
            job_environment);
	strcat(jsdl_buffer,tmp_buffer);

        jsdl = snprintf(tmp_buffer, sizeof(char) * GW_RSL_LENGTH,
	    "   </jsdl-posix:POSIXApplication>\n"
            "  </jsdl:Application>\n");
        strcat(jsdl_buffer,tmp_buffer);

        jsdl = snprintf(tmp_buffer, sizeof(char) * GW_RSL_LENGTH,
            "  <jsdl:DataStaging>\n"
            "   <jsdl:FileName>%s.wrapper</jsdl:FileName>\n"
            "   <jsdl:CreationFlag>overwrite</jsdl:CreationFlag>\n"
            "   <jsdl:Source>\n"
            "    <jsdl:URI>%s/%s</jsdl:URI>\n"
            "   </jsdl:Source>\n"
            "  </jsdl:DataStaging>\n",
            wrapper,
            staging_url, job->template.wrapper);
        strcat(jsdl_buffer,tmp_buffer);
        
        jsdl = snprintf(tmp_buffer, sizeof(char) * GW_RSL_LENGTH,
            "  <jsdl:DataStaging>\n"
            "   <jsdl:FileName>stdout.wrapper.%d</jsdl:FileName>\n"
            "   <jsdl:CreationFlag>overwrite</jsdl:CreationFlag>\n"
            "   <jsdl:Target>\n"
            "    <jsdl:URI>%s/%s" GW_VAR_DIR "/%d/stdout.wrapper.%d</jsdl:URI>\n"
            "   </jsdl:Target>\n"
            "  </jsdl:DataStaging>\n",
            job->restarted,
            staging_url, gw_conf.gw_location, job->id, job->restarted);
        strcat(jsdl_buffer,tmp_buffer);
        jsdl = snprintf(tmp_buffer, sizeof(char) * GW_RSL_LENGTH,
            "  <jsdl:DataStaging>\n"
            "   <jsdl:FileName>stderr.wrapper.%d</jsdl:FileName>\n"
            "   <jsdl:CreationFlag>overwrite</jsdl:CreationFlag>\n"
            "   <jsdl:Target>\n"
            "    <jsdl:URI>%s/%s" GW_VAR_DIR "/%d/stderr.wrapper.%d</jsdl:URI>\n"
            "   </jsdl:Target>\n"
            "  </jsdl:DataStaging>\n",
            job->restarted,
            staging_url, gw_conf.gw_location, job->id, job->restarted);
        strcat(jsdl_buffer,tmp_buffer);


	free(job_environment);        

    if (strlen(jsdl_buffer) + 6 > GW_RSL_LENGTH)
        return NULL;

    strcat(jsdl_buffer,
	    " </jsdl:JobDescription>\n"
            "</jsdl:JobDefinition>\n");

    jsdl = strdup(jsdl_buffer);
    return jsdl;
}


/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

char* gw_split_arguments_jsdl (const char *arguments)
{
        char jsdl_buffer[GW_RSL_LENGTH];
        char *argument;
        char *xml_arguments;
        int length,i,arg_i,ignore_space=0;
       
	length   = strlen(arguments);
	argument = (char *) malloc (length * sizeof(char));
        arg_i    = 0;

        jsdl_buffer[0] = '\0';
        
	for (i=0;i<length;i++)
        {
		if (arguments[i] != ' ')
                        argument[arg_i++] = arguments[i];

                else
                {
                        argument[arg_i]='\0';
                        strcat(jsdl_buffer,"    <jsdl-posix:Argument>");
                        strcat(jsdl_buffer,argument);
                        strcat(jsdl_buffer,"</jsdl-posix:Argument>\n");
                        arg_i = 0;
                }
        }

        argument[arg_i]='\0';
        strcat(jsdl_buffer,"    <jsdl-posix:Argument>");
        strcat(jsdl_buffer,argument);
        strcat(jsdl_buffer,"</jsdl-posix:Argument>\n");

        xml_arguments = strdup (jsdl_buffer);

        return xml_arguments;
}


/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

char *gw_em_jsdl_environment(gw_job_t *job)
{
    char jsdl_buffer[640];
    char tmp_buffer[640];
    char *jsdl_env;
    int rc;
    int i;

    for (i=0;i<job->template.num_env;i++)
    {
            rc = snprintf(tmp_buffer, sizeof(char) * GW_RSL_LENGTH,
            "    <jsdl-posix:Environment name=\"%s\">%s</jsdl-posix:Environment>\n",
            job->template.environment[i][0],
            job->template.environment[i][1]);
            strcat(jsdl_buffer,tmp_buffer);
    }

    rc = snprintf(tmp_buffer, 640,
            "    <jsdl-posix:Environment name=\"GW_HOSTNAME\">%s</jsdl-posix:Environment>\n"
            "    <jsdl-posix:Environment name=\"GW_USER\">%s</jsdl-posix:Environment>\n"
            "    <jsdl-posix:Environment name=\"GW_JOB_ID\">%i</jsdl-posix:Environment>\n"
            "    <jsdl-posix:Environment name=\"GW_TASK_ID\">%i</jsdl-posix:Environment>\n"
            "    <jsdl-posix:Environment name=\"GW_ARRAY_ID\">%i</jsdl-posix:Environment>\n"
            "    <jsdl-posix:Environment name=\"GW_TOTAL_TASKS\">%i</jsdl-posix:Environment>\n"
            "    <jsdl-posix:Environment name=\"GW_RESTARTED\">%i</jsdl-posix:Environment>\n",
            job->history->host->hostname,
            job->owner,
            job->id,
            job->task_id,
            job->array_id,
            job->total_tasks,
            job->restarted);
    strcat(jsdl_buffer,tmp_buffer);

    if ((rc >= 640 ) || ( rc < 0 ) )
        return NULL;

    jsdl_env = strdup(jsdl_buffer);

    return jsdl_env;
}
