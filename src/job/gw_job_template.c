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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>

#include "gw_job_template.h"
#include "gw_common.h"
#include "gw_conf.h"
#include "gw_job_pool.h"

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_job_template_set_str(char **dst, const char *src)
{
	if (src[0] == '\0')	
		*dst = NULL;
	else
		*dst = strdup(src);
}

/* -------------------------------------------------------------------------- */

void gw_job_template_array_destroy (char **** array, int num_files)
{
	int i;

	if ( (*array) == NULL )
		return;
	
	for ( i = 0; i < num_files; i++ )
	{
		if ( (*array)[i] != NULL )
		{
			if ( (*array)[i][0]  != NULL )
				free((*array)[i][0]);

			if ( (*array)[i][1]  != NULL )
				free((*array)[i][1]);
				
			free ((*array)[i]);
		}
	}

    free(*array);
}	

/* -------------------------------------------------------------------------- */

void gw_job_template_sarray_destroy (char *** array, int num_files)
{
	int i;

	if ( (*array) == NULL )
		return;
	
	for ( i = 0; i < num_files; i++ )
	{
		if ( (*array)[i] != NULL )
			free ((*array)[i]);
	}

    free(*array);
}	

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
	
void gw_job_template_init (gw_job_template_t *jt, const gw_template_t *ct)
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

    gw_job_pool_dep_cp (ct->job_deps, &(jt->job_deps));

    jt->type = ct->type;
    jt->np = ct->np;

    jt->deadline = ct->deadline;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_job_template_destroy ( gw_job_template_t *job_template )
{
    if (job_template->file != NULL )
        free(job_template->file);

    if ( job_template->name != NULL )
        free(job_template->name);
       
    if ( job_template->job_home != NULL )
        free(job_template->job_home);
    
    if ( job_template->user_home != NULL )
        free(job_template->user_home);
    
    if ( job_template->executable != NULL )
		free(job_template->executable);
		
    if ( job_template->arguments != NULL )		
		free(job_template->arguments);

    if ( job_template->pre_wrapper != NULL )
		free(job_template->pre_wrapper);
		
    if ( job_template->pre_wrapper_arguments != NULL )		
		free(job_template->pre_wrapper_arguments);
		
	gw_job_template_array_destroy(&(job_template->input_files),
	                              job_template->num_input_files);

	gw_job_template_array_destroy(&(job_template->output_files),
	                              job_template->num_output_files);

	gw_job_template_sarray_destroy(&(job_template->restart_files),
	                              job_template->num_restart_files);

	gw_job_template_array_destroy(&(job_template->environment),
	                              job_template->num_env);

    if ( job_template->stdin_file != NULL )	
		free(job_template->stdin_file);
		
    if ( job_template->stdout_file != NULL )		
		free(job_template->stdout_file);

    if ( job_template->stderr_file != NULL )		
		free(job_template->stderr_file);

	if ( job_template->requirements != NULL )
		free(job_template->requirements);
		
    if ( job_template->rank != NULL )		
		free(job_template->rank);
        
    if ( job_template->checkpoint_url != NULL )
        free(job_template->checkpoint_url);
        
    if ( job_template->wrapper != NULL )		
		free(job_template->wrapper);

    if ( job_template->monitor != NULL )		
		free(job_template->monitor);
		
    if ( job_template->job_deps != NULL )		
		free(job_template->job_deps);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_job_template_print (FILE *fd, gw_job_template_t *template)
{
	int i;
	
	gw_print(fd,"DM",'I',"----------- Job configuration file (%s) values -----------\n",
		template->file);
	gw_print(fd,"DM",'I',"\t%-23s: %s\n","EXECUTABLE",GWNSTR(template->executable));
	gw_print(fd,"DM",'I',"\t%-23s: %s\n","ARGUMENTS",GWNSTR(template->arguments));

	gw_print(fd,"DM",'I',"\t%-14s(Total %i):\n","INPUT_FILES",template->num_input_files);
	for (i=0;i<template->num_input_files;i++)
		gw_print(fd,"DM",'I',"\t\t(%i) Local: %-25s - Remote: %s\n",i,
			GWNSTR(template->input_files[i][GW_LOCAL_FILE]),
			GWNSTR(template->input_files[i][GW_REMOTE_FILE]));

	gw_print(fd,"DM",'I',"\t%-14s(Total %i):\n","OUTPUT_FILES",template->num_output_files);
	for (i=0;i<template->num_output_files;i++)
		gw_print(fd,"DM",'I',"\t\t(%i) Local: %-25s - Remote: %s\n",i,
			GWNSTR(template->output_files[i][GW_LOCAL_FILE]),
			GWNSTR(template->output_files[i][GW_REMOTE_FILE]));

	gw_print(fd,"DM",'I',"\t%-14s(Total %i):\n","RESTART_FILES",template->num_restart_files);
	for (i=0;i<template->num_restart_files;i++)
		gw_print(fd,"DM",'I',"\t\t(%i) File: %-25s\n",i,GWNSTR(template->restart_files[i]));
			
	if ( template->num_env > 0 )
	{
		gw_print(fd,"DM",'I',"\t%-14s(Total %i):\n","ENVIRONMENT",template->num_env);
		for (i=0;i<template->num_env;i++)
			gw_print(fd,"DM",'I',"\t\t(%i) %-16s = %s\n",i,
				GWNSTR(template->environment[i][GW_ENV_VAR]),
				GWNSTR(template->environment[i][GW_ENV_VAL]));
	}

	gw_print(fd,"DM",'I',"\t%-23s: %s\n","STDIN_FILE",GWNSTR(template->stdin_file));
    gw_print(fd,"DM",'I',"\t%-23s: %s\n","STDOUT_FILE",GWNSTR(template->stdout_file));
	gw_print(fd,"DM",'I',"\t%-23s: %s\n","STDERR_FILE",GWNSTR(template->stderr_file));
	gw_print(fd,"DM",'I',"\t%-23s: %s\n","REQUIREMENTS",GWNSTR(template->requirements));
	gw_print(fd,"DM",'I',"\t%-23s: %s\n","RANK",GWNSTR(template->rank));
	gw_print(fd,"DM",'I',"\t%-23s: %i\n","RESCHEDULING_INTERVAL",template->rescheduling_interval);
	gw_print(fd,"DM",'I',"\t%-23s: %i\n","RESCHEDULING_THRESHOLD",template->rescheduling_threshold);
	gw_print(fd,"DM",'I',"\t%-23s: %i\n","SUSPENSION_TIMEOUT",template->suspension_timeout);
	gw_print(fd,"DM",'I',"\t%-23s: %i\n","CPULOAD_THRESHOLD",template->cpuload_threshold);
    gw_print(fd,"DM",'I',"\t%-23s: %s\n","RESCHEDULE_ON_FAILURE",template->reschedule_on_failure?"yes":"no");
	gw_print(fd,"DM",'I',"\t%-23s: %i\n","NUMBER_OF_RETRIES",template->number_of_retries);
	gw_print(fd,"DM",'I',"\t%-23s: %i\n","CHECKPOINT_INTERVAL",template->checkpoint_interval);
	gw_print(fd,"DM",'I',"\t%-23s: %s\n","CHECKPOINT_URL",GWNSTR(template->checkpoint_url));
	gw_print(fd,"DM",'I',"\t%-23s: %s\n","WRAPPER",GWNSTR(template->wrapper));
	gw_print(fd,"DM",'I',"\t%-23s: %s\n","MONITOR",GWNSTR(template->monitor));
	gw_print(fd,"DM",'I',"\t%-23s: %s\n","PRE_WRAPPER",GWNSTR(template->pre_wrapper));		
	gw_print(fd,"DM",'I',"\t%-23s: %s\n","PRE_WRAPPER_ARGUMENTS",GWNSTR(template->pre_wrapper_arguments));
	
	if ( template->job_deps != NULL )
	{
		gw_print(fd,"DM",'I',"\t%-23s:\n","JOB_DEPENDENCIES");	
	
		i=0;
		while ( template->job_deps[i] != -1 )
		{
			gw_print(fd,"DM",'I',"\t\t(%i) JOB = %i\n",i,template->job_deps[i]);
			i++;
		}
	}

    gw_print(fd,"DM",'I',"\t%-23s: %s\n","TYPE",gw_template_jobtype_string(template->type));
    gw_print(fd,"DM",'I',"\t%-23s: %i\n","NP",template->np);

    gw_print(fd,"DM",'I',"\t%-23s: %s %d\n","DEADLINE",gw_template_deadline_string(template->deadline), template->deadline);
				
    gw_print(fd,"DM",'I',"----------------------------------------------------------\n");
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_job_template_to_file(FILE *fd, gw_job_template_t *template)
{
	int i;

	fprintf(fd, "NAME=%s\n", GWNSTR(template->name));

	fprintf(fd, "EXECUTABLE=%s\n", GWNSTR(template->executable));
  	fprintf(fd, "ARGUMENTS=%s\n", GWNSTR(template->arguments));

    if (template->num_input_files > 0)
	{
        fprintf(fd, "INPUT_FILES=%s %s",
	    		GWNSTR(template->input_files[0][GW_LOCAL_FILE]),
		    	GWNSTR(template->input_files[0][GW_REMOTE_FILE]));
    	for (i=1; i<template->num_input_files; i++)
	    	fprintf(fd, ", %s %s",
		    	GWNSTR(template->input_files[i][GW_LOCAL_FILE]),
			    GWNSTR(template->input_files[i][GW_REMOTE_FILE]));
        fprintf(fd, "\n");
	}

    if (template->num_output_files > 0)
	{
        fprintf(fd, "OUTPUT_FILES=%s %s",
	    		GWNSTR(template->output_files[0][GW_LOCAL_FILE]),
		    	GWNSTR(template->output_files[0][GW_REMOTE_FILE]));
    	for (i=1; i<template->num_output_files; i++)
	    	fprintf(fd, ", %s %s",
		    	GWNSTR(template->output_files[i][GW_LOCAL_FILE]),
			    GWNSTR(template->output_files[i][GW_REMOTE_FILE]));
        fprintf(fd, "\n");
	}

    if (template->num_restart_files > 0)
	{
    	fprintf(fd, "RESTART_FILES=%s",	GWNSTR(template->restart_files[0]));
    	
    	for (i=1; i<template->num_restart_files; i++)
    		fprintf(fd,", %s",GWNSTR(template->restart_files[i]));
        fprintf(fd, "\n");
	}
			
	if ( template->num_env > 0 )
	{
		fprintf(fd, "ENVIRONMENT=%s=\"%s\"",
				GWNSTR(template->environment[0][GW_ENV_VAR]),
				GWNSTR(template->environment[0][GW_ENV_VAL]));
		for (i=1; i<template->num_env; i++)
			fprintf(fd, ", %s=\"%s\"",
    				GWNSTR(template->environment[i][GW_ENV_VAR]),
	    			GWNSTR(template->environment[i][GW_ENV_VAL]));
        fprintf(fd, "\n");
	}

	fprintf(fd, "STDIN_FILE=%s\n", GWNSTR(template->stdin_file));
    fprintf(fd, "STDOUT_FILE=%s\n", GWNSTR(template->stdout_file));
	fprintf(fd, "STDERR_FILE=%s\n", GWNSTR(template->stderr_file));
	fprintf(fd, "REQUIREMENTS=%s\n", GWNSTR(template->requirements));
	fprintf(fd, "RANK=%s\n", GWNSTR(template->rank));
	fprintf(fd, "RESCHEDULING_INTERVAL=%ld\n", template->rescheduling_interval);
	fprintf(fd, "RESCHEDULING_THRESHOLD=%ld\n", template->rescheduling_threshold);
	fprintf(fd, "SUSPENSION_TIMEOUT=%ld\n", template->suspension_timeout);
	fprintf(fd, "CPULOAD_THRESHOLD=%d\n", template->cpuload_threshold);
    fprintf(fd, "RESCHEDULE_ON_FAILURE=%s\n",template->reschedule_on_failure?"yes":"no");
	fprintf(fd, "NUMBER_OF_RETRIES=%d\n", template->number_of_retries);
	fprintf(fd, "CHECKPOINT_INTERVAL=%ld\n", template->checkpoint_interval);
	fprintf(fd, "CHECKPOINT_URL=%s\n", GWNSTR(template->checkpoint_url));
	fprintf(fd, "WRAPPER=%s\n", GWNSTR(template->wrapper));
   	fprintf(fd, "MONITOR=%s\n", GWNSTR(template->monitor));
  	fprintf(fd, "PRE_WRAPPER=%s\n", GWNSTR(template->pre_wrapper));
   	fprintf(fd, "PRE_WRAPPER_ARGUMENTS=%s\n", GWNSTR(template->pre_wrapper_arguments));
   	
   	if ( template->job_deps != NULL )
	{
		fprintf(fd, "JOB_DEPENDENCIES=");
			
		i=0;
		while ( template->job_deps[i] != -1 )
		{
			fprintf(fd, "%d ",template->job_deps[i]);
			i++;
		}
		fprintf(fd, "\n");
	}   	

	fprintf(fd, "TYPE=%s\n", gw_template_jobtype_string(template->type));
	fprintf(fd, "NP=%d\n", template->np);

	fprintf(fd, "DEADLINE=%s\n", gw_template_deadline_string(template->deadline));
}
