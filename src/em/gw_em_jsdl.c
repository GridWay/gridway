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

#include "gw_em_rsl.h"
#include "gw_job.h"
#include "gw_template.h"

char* gw_split_arguments_jsdl(const char *arguments);

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/*char* gw_generate_nowrapper_jsdl2 (gw_job_t *job)
{
}*/


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
	int rc;
	int i;
        char *arguments;
	char *xml_arguments;

    /* ---------------------------------------------------------------------- */
    /* 1.- Create dynamic job data environment                                */
    /* ---------------------------------------------------------------------- */
  
    job_environment = gw_em_jsdl_environment(job);
    
    if ( job_environment == NULL )
        return NULL;

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
            "%s"
	    "  <jsdl:Application>\n"
	    "   <jsdl-posix:POSIXApplication>\n",
            job_environment);

        jsdl = snprintf(tmp_buffer, sizeof(char) * GW_RSL_LENGTH,
            "    <jsdl-posix:Executable>%s</jsdl-posix:Executable>\n",
            job->template.executable);
        strcat(jsdl_buffer,tmp_buffer);

	if (job->template.arguments != NULL)
	{
		xml_arguments = gw_split_arguments_jsdl(job->template.arguments);
        	strncat(jsdl_buffer,xml_arguments,
              		GW_RSL_LENGTH-strlen(jsdl_buffer));
        }

	jsdl = snprintf(tmp_buffer, sizeof(char) * GW_RSL_LENGTH,
	    "    <jsdl-posix:Input>%s</jsdl-posix:Input>\n"
            "    <jsdl-posix:Output>%s</jsdl-posix:Output>\n"
            "    <jsdl-posix:Error>%s</jsdl-posix:Error>\n",
            job->template.stdin_file,
            job->template.stdout_file,
            job->template.stderr_file);
	strcat(jsdl_buffer,tmp_buffer);
	
	for (i=0;i<job->template.num_env;i++)
        {
        	jsdl = snprintf(tmp_buffer, sizeof(char) * GW_RSL_LENGTH,
		    "    <jsdl-posix:Environment name=\"%s\">"
		    "%s"
		    "</jsdl-posix:Environment>\n",
            	    job->template.environment[i][0],
                    job->template.environment[i][1]);
        strcat(jsdl_buffer,tmp_buffer);
        }

        jsdl = snprintf(tmp_buffer, sizeof(char) * GW_RSL_LENGTH,
	    "   </jsdl-posix:POSIXApplication>\n"
            "  </jsdl:Application>\n"
            "  <jsdl:DataStaging>\n"
            "   <jsdl:FileName>%s</jsdl:FileName>\n"
	    "   <jsdl:CreationFlag>overwrite</jsdl:CreationFlag>\n"
            "  </jsdl:DataStaging>\n",
            job->template.file);
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

/*char* gw_generate_pre_wrapper_jsdl2 (gw_job_t *job)
{
}*/

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

char *gw_em_jsdl_environment(gw_job_t *job)
{
    char jsdl_buffer[640];
    char *jsdl_env;
    int  rc;

    rc = snprintf(jsdl_buffer, 640,
            "  <jsdl:JobIdentification>\n"
            "   <jsdl:JobName>%s</jsdl:JobName>\n"
            "  </jsdl:JobIdentification>\n",
            job->template.name);

    if ((rc >= 640 ) || ( rc < 0 ) )
        return NULL;

    jsdl_env = strdup(jsdl_buffer);

    return jsdl_env;
}
