/* -------------------------------------------------------------------------- */
/* Copyright 2002-2011, GridWay Project Leads (GridWay.org)                   */
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


/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
 
char* gw_generate_wrapper_rsl (gw_job_t *job)
{
    char *rsl;
    char *job_environment; 
    char  rsl_buffer[GW_RSL_LENGTH];
    char  tmp_buffer[GW_RSL_LENGTH];
    int   print_queue = 0;
    char *jobtype;
    
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
    
    jobtype = strdup(gw_template_jobtype_string(job->template.type));
    
    snprintf(rsl_buffer, sizeof(char) * GW_RSL_LENGTH,
            "&(jobtype=\"%s\")"
            "(executable=\".gw_%s_%i/.wrapper\")"
            "(stdout=\".gw_%s_%i/stdout.wrapper\")"
            "(stderr=\".gw_%s_%i/stderr.wrapper\")"
            "(environment=%s)"
            "(count=%d)",
            jobtype,
            job->owner, job->id,
            job->owner, job->id,
            job->owner, job->id,
            job_environment,
            job->template.np);

    if ( print_queue )
    {
        snprintf(tmp_buffer, sizeof(char) * GW_RSL_LENGTH,
                "(queue=\"%s\")", job->history->queue);
        strncat(rsl_buffer, tmp_buffer, GW_RSL_LENGTH-strlen(rsl_buffer));
    }

    free(job_environment);
    free(jobtype);
    
    if ( strlen(rsl_buffer) >= GW_RSL_LENGTH )
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
    char tmp_buffer[GW_RSL_LENGTH];
    int print_queue = 0;
    int rc;
    char *staging_url;
    char wrapper[PATH_MAX], stdout_wrapper[PATH_MAX], stderr_wrapper[PATH_MAX];

    if ( job->history->queue != NULL )
        if ( strcmp(job->history->queue,"-") != 0 )
            print_queue = 1;

    if (job->history->tm_mad->url != NULL)
    {
        /* Perform staging with the URL provided by the TM MAD */
        
        staging_url = job->history->tm_mad->url;
        
        snprintf(wrapper, PATH_MAX -1, "%s", job->template.wrapper);
        
        snprintf(stdout_wrapper, 
                 PATH_MAX -1, 
                 "%s/" GW_VAR_DIR "/%d/stdout.wrapper.%d",
                 gw_conf.gw_location, 
                 job->id, 
                 job->restarted);
                
        snprintf(stderr_wrapper,
                 PATH_MAX - 1, 
                 "%s/" GW_VAR_DIR "/%d/stderr.wrapper.%d",
                 gw_conf.gw_location, 
                 job->id, 
                 job->restarted);
    }
    else
    {
    	/* Use the cluster front-end if no URL was provided */
        /* TODO: Executable staging always through CE */
        /* TODO: Wrapper must gather dst URLs without renaming instead of src URLs */
    	    	
        staging_url = job->history->host->hostname;
        
        snprintf(wrapper, 
                 PATH_MAX - 1,
                 "~/.gw_%s_%i/.wrapper", 
                 job->owner, 
                 job->id);
        
        snprintf(stdout_wrapper,
                 PATH_MAX - 1, 
                 "~/.gw_%s_%i/stdout.wrapper",
                 job->owner, 
                 job->id);
                
        snprintf(stderr_wrapper,
                 PATH_MAX - 1, 
                 "~/.gw_%s_%i/stderr.wrapper",
                 job->owner, 
                 job->id);        
    }
    
    /* ---------------------------------------------------------------------- */
    /* 1.- Create dynamic job data environment                                */
    /* ---------------------------------------------------------------------- */
  
    job_environment = gw_job_rsl_environment(job);
    
    if ( job_environment == NULL )
        return NULL;
    
    /* ---------------------------------------------------------------------- */
    /* 2.- Build RSL String & Return it                                       */
    /* ---------------------------------------------------------------------- */

    rc = snprintf(rsl_buffer, sizeof(char) * GW_RSL_LENGTH,
            "&(executable=\"%s/%s\")"
            "(arguments=\"%s/%s/" GW_VAR_DIR "/%d/job.env\")"
            "(stdout=\"%s/%s\")"
            "(stderr=\"%s/%s\")"
            "(environment=%s)",
            staging_url, wrapper,
            staging_url, gw_conf.gw_location, job->id,
            staging_url, stdout_wrapper,
            staging_url, stderr_wrapper,
            job_environment);
    
    free(job_environment);        

    if ( print_queue )
    {
        snprintf(tmp_buffer, sizeof(char) * GW_RSL_LENGTH,
                "(queue=\"%s\")", job->history->queue);
        strncat(rsl_buffer, tmp_buffer, GW_RSL_LENGTH-strlen(rsl_buffer));
    }
    
    if ( strlen(rsl_buffer) >= GW_RSL_LENGTH )
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
        pre_wrapper = gw_job_substitute(job->template.pre_wrapper, job);
        
    if ( pre_wrapper == NULL )
    {
        
        gw_job_print(job,"EM",'E',"Parse error (%s) while generating rsl.\n",job->template.pre_wrapper);    
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
        pre_wrapper_arguments = gw_job_substitute(
                job->template.pre_wrapper_arguments, job);

        if ( pre_wrapper_arguments == NULL )
        {
            gw_job_print(job,"EM",'E',"Parse error (%s) while generating rsl.\n",job->template.pre_wrapper_arguments);    
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

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/* Nordugrid xrsl */
char* gw_generate_wrapper_xrsl (gw_job_t *job)
{
    char *rsl;
    char *job_environment; 
    char rsl_buffer[GW_RSL_LENGTH];
	char transfers[GW_RSL_LENGTH];
	char transfer_line[1024];
    int print_queue = 0;
    int rc;
	int c;
    
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
				"&(executable=\"%s\")"
				"(arguments=%s)"
				"(stdout=\"stdout.execution\")"
				"(stderr=\"stderr.execution\")"
				"(queue=\"%s\")"
				"(environment=%s)",
				job->template.executable,
				job->template.arguments,
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
    
	/* Build transfer rsl part */
	
	/* Input files */
	if(job->template.num_input_files)
	{
		strcpy(transfers, "(inputFiles=");
		/*
		sprintf(transfer_line, "(\"%s\" \"%s\")", job->template.stdin_file,
				job->template.stdin_file);
		strcat(transfers, transfer_line);
		*/
	
		for(c=0;c<job->template.num_input_files;c++)
		{
			sprintf(transfer_line, "(\"%s\" \"%s\")",
					job->template.input_files[c][GW_LOCAL_FILE],
					job->template.input_files[c][GW_REMOTE_FILE]);
			strcat(transfers, transfer_line);
		}
		strcat(transfers, ")");
	
		strcat(rsl_buffer, transfers);
	}
	
	/* Output files */
	if(job->template.num_output_files)
	{
		strcpy(transfers, "(outputFiles=");
		for(c=0;c<job->template.num_output_files;c++)
		{
			sprintf(transfer_line, "(\"%s\" \"%s\")",
					job->template.output_files[c][GW_LOCAL_FILE],
					job->template.output_files[c][GW_REMOTE_FILE]);
			strcat(transfers, transfer_line);
		}
		strcat(transfers, ")");	
		strcat(rsl_buffer, transfers);
	}

    free(job_environment);        

    if ((rc >= (GW_RSL_LENGTH * sizeof(char))) || ( rc < 0 ) )          
        return NULL;
    
    rsl = strdup(rsl_buffer);
    return rsl;
}

