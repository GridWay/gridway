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

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
 
char* gw_generate_wrapper_rsl (gw_job_t *job)
{
    char *rsl;
    char *job_environment; 
	char rsl_buffer[GW_RSL_LENGTH];
	int print_queue = 0;
	int rc;
	
    /* ---------------------------------------------------------------------- */
    /* 1.- Create dynamic job data environment                                */
    /* ---------------------------------------------------------------------- */
  
    job_environment = gw_job_rsl_environment(job);
    
    if ( job_environment == NULL )
        return NULL;
    
    /* ---------------------------------------------------------------------- */
    /* 2.- Build RSL String & Return it                                       */
    /* ---------------------------------------------------------------------- */

	if ( job->history->queue != NULL )
    	if ( strcmp(job->history->queue,"-") != 0 )
	        print_queue = 1;

	if ( print_queue )
    {
    	rc = snprintf(rsl_buffer, sizeof(char) * GW_RSL_LENGTH,
	    		"&(executable=\".gw_%s_%i/.wrapper\")"
             	"(stdout=\".gw_%s_%i/stdout.wrapper\")"
             	"(stderr=\".gw_%s_%i/stderr.wrapper\")"
             	"(queue=\"%s\")"
             	"(environment=%s)",
                job->owner, job->id,
             	job->owner, job->id,
             	job->owner, job->id,
             	job->history->queue,
             	job_environment);
	}
    else
    {
    	rc = snprintf(rsl_buffer, sizeof(char) * GW_RSL_LENGTH,
	    		"&(executable=\".gw_%s_%i/.wrapper\")"
             	"(stdout=\".gw_%s_%i/stdout.wrapper\")"
         	    "(stderr=\".gw_%s_%i/stderr.wrapper\")"
             	"(environment=%s)",
             	job->owner, job->id,
             	job->owner, job->id,
             	job->owner, job->id, 
         	    job_environment);
    }
    
	free(job_environment);        

	if ((rc >= (GW_RSL_LENGTH * sizeof(char))) || ( rc < 0 ) )          
    	return NULL;
    
    rsl = strdup(rsl_buffer);
    return rsl;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

char* gw_generate_wrapper_rsl_nsh (gw_job_t *job)
{
    char *rsl;
    char *job_environment; 
	char rsl_buffer[GW_RSL_LENGTH];
	int print_queue = 0;
	int rc;
	char *stage_rc;
	char *se;
	
    /* ---------------------------------------------------------------------- */
    /* 1.- Create dynamic job data environment                                */
    /* ---------------------------------------------------------------------- */
  
    job_environment = gw_job_rsl_environment(job);
    
    if ( job_environment == NULL )
        return NULL;
    
    /* ---------------------------------------------------------------------- */
    /* 2.- Build RSL String & Return it                                       */
    /* ---------------------------------------------------------------------- */

	if ( job->history->queue != NULL )
    	if ( strcmp(job->history->queue,"-") != 0 )
	        print_queue = 1;
	        
	se = gw_host_get_genvar_str("SE_HOSTNAME", 0, job->history->host);
		
	if ((se != NULL) && (se[0] != '\0'))
	{
		stage_rc = se;		
	}
	else
	{
		stage_rc = job->history->host->hostname;
	}
	
	if ( print_queue )
    {
    	rc = snprintf(rsl_buffer, sizeof(char) * GW_RSL_LENGTH,
	    		"&(executable=\"gsiftp://%s/~/.gw_%s_%i/.wrapper\")"
             	"(stdout=\"gsiftp://%s/~/.gw_%s_%i/stdout.wrapper\")"
         	    "(stderr=\"gsiftp://%s/~/.gw_%s_%i/stderr.wrapper\")"
             	"(queue=\"%s\")"
             	"(environment=%s)",
                stage_rc, job->owner, job->id,
             	stage_rc, job->owner, job->id,
         	    stage_rc, job->owner, job->id,
             	job->history->queue,
             	job_environment);
	}
    else
    {
    	rc = snprintf(rsl_buffer, sizeof(char) * GW_RSL_LENGTH,
	    		"&(executable=\"gsiftp://%s/~/.gw_%s_%i/.wrapper\")"
             	"(stdout=\"gsiftp://%s/~/.gw_%s_%i/stdout.wrapper\")"
         	    "(stderr=\"gsiftp://%s/~/.gw_%s_%i/stderr.wrapper\")"
             	"(environment=%s)",
             	stage_rc, job->owner, job->id,
             	stage_rc, job->owner, job->id,
             	stage_rc, job->owner, job->id, 
         	    job_environment);
    }
    
	free(job_environment);        

	if ((rc >= (GW_RSL_LENGTH * sizeof(char))) || ( rc < 0 ) )          
    	return NULL;
    
    rsl = strdup(rsl_buffer);
    return rsl;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
 
char* gw_generate_pre_wrapper_rsl (gw_job_t *job)
{
    char *rsl;
    char *job_environment;
    char *pre_wrapper;
	char *pre_wrapper_arguments;    
	char rsl_buffer[GW_RSL_LENGTH];
	int  rc, size;
	
    /* ---------------------------------------------------------------------- */
    /* 1.- Create dynamic job data environment                                */
    /* ---------------------------------------------------------------------- */
  
    job_environment = gw_job_rsl_environment(job);
    
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

	if ( job->template.pre_wrapper[0] == '/' ) /*Absolute path*/				
   		rc = snprintf(rsl_buffer, sizeof(char) * GW_RSL_LENGTH,
    			"&(executable=\"%s\")"
             	"(stdout=\".gw_%s_%i/stdout.pre_wrapper\")"
             	"(stderr=\".gw_%s_%i/stderr.pre_wrapper\")"
             	"(environment=%s)",
             	pre_wrapper,
             	job->owner, job->id,
             	job->owner, job->id,
             	job_environment);
	else
   		rc = snprintf(rsl_buffer, sizeof(char) * GW_RSL_LENGTH,
    			"&(executable=\".gw_%s_%i/%s\")"
             	"(stdout=\".gw_%s_%i/stdout.pre_wrapper\")"
             	"(stderr=\".gw_%s_%i/stderr.pre_wrapper\")"
             	"(environment=%s)",
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
		
		pre_wrapper_arguments = gw_job_substitute (
								job->template.pre_wrapper_arguments, job);

		if ( pre_wrapper_arguments == NULL )
		{
			gw_job_print(job,"DM",'E',"Parse error (%s) while generating rsl.\n",job->template.pre_wrapper_arguments);	
			free(job_environment);
			free(pre_wrapper);
			return NULL;
		}
		
		size = strlen(rsl_buffer) + strlen(pre_wrapper_arguments) + 13;
		if ( size > GW_RSL_LENGTH )
		{
			free(pre_wrapper_arguments);
			free(job_environment);        
			free(pre_wrapper);		
    		return NULL;
		}
		
		strcat(rsl_buffer,"(arguments=");
		strcat(rsl_buffer,pre_wrapper_arguments);
		strcat(rsl_buffer,")");
		
		free(pre_wrapper_arguments);		
	}
	
	free(job_environment);        
	free(pre_wrapper);
	
    rsl = strdup(rsl_buffer);
    return rsl;
}
