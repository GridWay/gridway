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

#include "gw_em_rsl.h"
#include "gw_job.h"
#include "gw_template.h"


/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

char* gw_generate_rsl2 (gw_job_t *job)
{
    if ( job->template.type == GW_JOB_TYPE_MPI
            || strcmp(job->history->host->lrms_type, "gw") == 0
            || job->template.wrapper == NULL )
        return gw_generate_nowrapper_rsl2(job);
    else
        return gw_generate_wrapper_rsl2(job);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

char* gw_generate_nowrapper_rsl2 (gw_job_t *job)
{
    char *rsl;
    char *job_environment; 
    char tmp_buffer[GW_RSL_LENGTH];
    char rsl_buffer[GW_RSL_LENGTH];
    int print_queue = 0;
    int rc;
	
    /* ---------------------------------------------------------------------- */
    /* 1.- Create dynamic job data environment                                */
    /* ---------------------------------------------------------------------- */
  
    job_environment = gw_job_rsl2_environment(job);
    
    if ( job_environment == NULL )
        return NULL;
        
    /* ---------------------------------------------------------------------- */
    /* 2.- Build RSL String & Return it                                       */
    /* ---------------------------------------------------------------------- */

    if ( job->history->queue != NULL )
        if ( strcmp(job->history->queue,"-") != 0 )
            print_queue = 1;

    rc = snprintf(rsl_buffer, sizeof(char) * GW_RSL_LENGTH,
            "<job>"
            " <executable>%s</executable>"
            " <argument>%s</argument>"
            " <directory>.gw_%s_%i</directory>"
            " <count>%d</count>"
            " <jobType>%s</jobType>",
            job->template.executable,         
            job->template.arguments,
            job->owner, job->id,
            job->template.np,
            gw_template_jobtype_string(job->template.type));

    if ( job->template.stdin_file[0] == '/' )
        sprintf(tmp_buffer," <stdin>%s</stdin>",job->template.stdin_file);
    else
        sprintf(tmp_buffer," <stdin>stdin.execution</stdin>");  
	    
    strcat(rsl_buffer,tmp_buffer);
    
    strcat(rsl_buffer," <stdout>stdout.execution</stdout> <stderr>stderr.execution</stderr>");
    
    strcat(rsl_buffer,job_environment);
    free(job_environment);
    
    if ( print_queue )
    {
        sprintf(tmp_buffer," <queue>%s</queue>",job->history->queue);
        strcat(rsl_buffer,tmp_buffer);
    }
	
    strcat(rsl_buffer,"</job>");

    if ((rc >= (GW_RSL_LENGTH * sizeof(char))) || ( rc < 0 ) )
    	return NULL;

    rsl = strdup(rsl_buffer);
    return rsl;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

char* gw_generate_wrapper_rsl2 (gw_job_t *job)
{
    char *rsl;
    char *job_environment; 
	char rsl_buffer[GW_RSL_LENGTH];
	int print_queue = 0;
	int rc;

    /* ---------------------------------------------------------------------- */
    /* 1.- Create dynamic job data environment                                */
    /* ---------------------------------------------------------------------- */
  
    job_environment = gw_job_rsl2_environment(job);
    
    if ( job_environment == NULL )
        return NULL;

    /* ---------------------------------------------------------------------- */
    /* 2.- Build RSL String & Return it                                       */
    /* ---------------------------------------------------------------------- */

	if ( job->history->queue != NULL )
    	if ( strcmp(job->history->queue,"-") != 0 )
	        print_queue = 1;        
        	
	if ( print_queue )
    	rc = snprintf(rsl_buffer, sizeof(char) * GW_RSL_LENGTH,
				"<job>"
                " <executable>.gw_%s_%i/.wrapper</executable>"
                " %s"
                " <stdout>.gw_%s_%i/stdout.wrapper</stdout>"
                " <stderr>.gw_%s_%i/stderr.wrapper</stderr>"
                " <queue>%s</queue>"
                "</job>",
                job->owner, job->id,
                job_environment,
                job->owner, job->id,
                job->owner, job->id,
                job->history->queue);
	else
		rc = snprintf(rsl_buffer, sizeof(char) * GW_RSL_LENGTH,
				"<job>"
                " <executable>.gw_%s_%i/.wrapper</executable>"
                " %s"
                " <stdout>.gw_%s_%i/stdout.wrapper</stdout>"
                " <stderr>.gw_%s_%i/stderr.wrapper</stderr>"
                "</job>",
                job->owner, job->id,
                job_environment,
                job->owner, job->id,
                job->owner, job->id);
	
	free(job_environment);        

	if ((rc >= (GW_RSL_LENGTH * sizeof(char))) || ( rc < 0 ) )          
    	return NULL;
    
    rsl = strdup(rsl_buffer);
    return rsl;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

char * gw_split_arguments (const char *arguments)
{
	char rsl_buffer[GW_RSL_LENGTH];
	char *argument;
	char *xml_arguments;
	int length,i,arg_i;
	
	length   = strlen(arguments);
	argument = (char *) malloc (length * sizeof(char));
	arg_i    = 0;
	
	for (i=0;i<length;i++)
	{
		if (arguments[i] != ' ')
			argument[arg_i++] = arguments[i]; 	
		else
		{
			argument[arg_i]='\0';
			strcat(rsl_buffer,"<argument>");
			strcat(rsl_buffer,argument);
			strcat(rsl_buffer,"</argument>");
			arg_i = 0;
		}
	}
	
	argument[arg_i]='\0';
	strcat(rsl_buffer,"<argument>");
	strcat(rsl_buffer,argument);
	strcat(rsl_buffer,"</argument>");
	
	xml_arguments = strdup (rsl_buffer);
	
	return xml_arguments;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

char* gw_generate_pre_wrapper_rsl2 (gw_job_t *job)
{
    char *rsl;
    char *job_environment;
    char *pre_wrapper;
	char *pre_wrapper_arguments;
	char *xml_pre_wrapper_arguments;
	char rsl_buffer[GW_RSL_LENGTH];
	int  rc, size;

    /* ---------------------------------------------------------------------- */
    /* 1.- Create dynamic job data environment                                */
    /* ---------------------------------------------------------------------- */
  
    job_environment = gw_job_rsl2_environment(job);
    
    if ( job_environment == NULL )
        return NULL;

    /* ---------------------------------------------------------------------- */
    /* 2.- Build RSL String & Return it                                       */
    /* ---------------------------------------------------------------------- */

	if ( job->template.pre_wrapper[0] == '/' ) /*Absolute path*/
		pre_wrapper = strdup(job->template.pre_wrapper);
	else
		pre_wrapper = gw_job_substitute (job->template.pre_wrapper, job);
		
	if ( pre_wrapper == NULL )
	{
		gw_job_print(job,"DM",'E',"Parse error (%s) while generating rsl.\n",job->template.pre_wrapper);	
		free(job_environment);			
		return NULL;
	}
        	
	if ( job->template.pre_wrapper[0] == '/' )
    	rc = snprintf(rsl_buffer, sizeof(char) * GW_RSL_LENGTH,
				"<job>"
                " <executable>%s</executable>"
                " %s"
                " <stdout>.gw_%s_%i/stdout.pre_wrapper</stdout>"
                " <stderr>.gw_%s_%i/stderr.pre_wrapper</stderr>",
                pre_wrapper,
                job_environment,
                job->owner, job->id,
                job->owner, job->id);
	else
		rc = snprintf(rsl_buffer, sizeof(char) * GW_RSL_LENGTH,
				"<job>"
                " <executable>.gw_%s_%i/%s</executable>"
                " %s"
                " <stdout>.gw_%s_%i/stdout.pre_wrapper</stdout>"
                " <stderr>.gw_%s_%i/stderr.pre_wrapper</stderr>",
                job->owner, job->id, pre_wrapper,
                job_environment,
                job->owner, job->id,
                job->owner, job->id);

	if ((rc >= (GW_RSL_LENGTH * sizeof(char))) || ( rc < 0 ) )          
	{
		free(job_environment);        
		free(pre_wrapper);		
    	return NULL;
	}
                
	if (job->template.pre_wrapper_arguments != NULL )
    {
		pre_wrapper_arguments = gw_job_substitute (
								job->template.pre_wrapper_arguments, job);

		if ( pre_wrapper_arguments == NULL )
		{
			gw_job_print(job,"DM",'E',"Parse error (%s) while generating rsl.\n",job->template.pre_wrapper_arguments);
			free(job_environment);
			free(pre_wrapper);
			return NULL;
		}
	
		xml_pre_wrapper_arguments = gw_split_arguments(pre_wrapper_arguments);

		size = strlen(rsl_buffer) + strlen(xml_pre_wrapper_arguments) + 6;
		if ( size > GW_RSL_LENGTH )
		{
			free(xml_pre_wrapper_arguments);
			free(pre_wrapper_arguments);
			free(job_environment);        
			free(pre_wrapper);		
    		return NULL;
		}
		
    	strcat(rsl_buffer,xml_pre_wrapper_arguments);
    	
    	free(pre_wrapper_arguments);
    	free(xml_pre_wrapper_arguments);
    }
    
    strcat(rsl_buffer,"</job>");
                
	free(job_environment);        
	free(pre_wrapper);
	    
    rsl = strdup(rsl_buffer);
    return rsl;
}
