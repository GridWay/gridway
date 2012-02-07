/* -------------------------------------------------------------------------- */
/* Copyright 2002-2012, GridWay Project Leads (GridWay.org)                   */
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
#include <limits.h>

#include "gw_template.h"
#include "gw_job_template.h"
#include "gw_conf.h"
#include "gw_common.h"

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_template_array_init( char array[GW_JT_FILES][2][GW_JT_STR] )
{
	int i;
			
	for ( i = 0; i < GW_JT_FILES; i++ )
	{
		array[i][0][0]='\0';
		array[i][1][0]='\0';
	}
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */    

void gw_template_sarray_init( char array[GW_JT_FILES][GW_JT_STR] )
{
	int i;
			
	for ( i = 0; i < GW_JT_FILES; i++ )
		array[i][0]='\0';
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */	

int gw_template_init(gw_template_t *jt, const char *jt_file)
{
    struct passwd * pw_ent;
	
	char            path[PATH_MAX];
    char *          tmp;
    time_t          the_time;
    char *          GW_LOCATION;
	int             rc;

    if ( jt_file[strlen(jt_file)-1] == '/' )
    {
        strncpy(path,     jt_file, strlen(jt_file)-1);
        strncpy(jt->file, "stdin", GW_JT_PATH);
        strncpy(jt->name, "stdin", GW_JT_STR);
    }
    else
    {

        if ( realpath (jt_file, path) == NULL )
            return -1;


        tmp = strrchr(path,'/') + 1;

        strncpy(jt->file, tmp, GW_JT_PATH);
        strncpy(jt->name, tmp, GW_JT_STR);

        *(--tmp)='\0';
    }
	 
    pw_ent   = getpwuid(getuid());
    the_time = time(NULL);
        
	GW_LOCATION = getenv("GW_LOCATION");   
    
    if ((GW_LOCATION == NULL) || (pw_ent == NULL))
        return -1;
 
	strncpy(jt->job_home,  path,           GW_JT_PATH);
	strncpy(jt->user_home, pw_ent->pw_dir, GW_JT_PATH);

	jt->executable[0] = '\0';
	jt->arguments[0]  = '\0';

	jt->pre_wrapper[0]           = '\0';
	jt->pre_wrapper_arguments[0] = '\0';
	
	jt->num_input_files = 0;
	gw_template_array_init(jt->input_files);

	jt->num_output_files = 0;
	gw_template_array_init(jt->output_files);

	jt->num_restart_files = 0;
	gw_template_sarray_init(jt->restart_files);

	jt->num_env = 0;
	gw_template_array_init(jt->environment);
	
	jt->stdin_file[0]  = '\0';
	jt->stdout_file[0] = '\0';
	jt->stderr_file[0] = '\0';

	jt->requirements[0] = '\0';
	jt->rank[0]         = '\0';
	
	jt->rescheduling_interval  = GW_RESCHEDULING_INTERVAL_DEFAULT;
	jt->rescheduling_threshold = GW_RESCHEDULING_THRESHOLD_DEFAULT;

    jt->checkpoint_interval = GW_CHECKPOINT_INTERVAL_DEFAULT;
    jt->checkpoint_url[0]   = '\0';

	jt->suspension_timeout = GW_SUSPENSION_TIMEOUT_DEFAULT;
	jt->cpuload_threshold  = GW_CPULOAD_THRESHOLD_DEFAULT;

	jt->reschedule_on_failure = GW_RESCHEDULE_ON_FAILURE_DEFAULT;
	jt->number_of_retries     = GW_NUMBER_OF_RETRIES_DEFAULT;  

	snprintf(jt->wrapper, GW_JT_PATH,"%s/%s",GW_LOCATION,GW_WRAPPER_DEFAULT);
	jt->monitor[0] = '\0';
	
	jt->job_deps[0] = -1;

	jt->type = GW_JOB_TYPE_SINGLE;
    jt->np   = 1;

    jt->deadline = 0;
	
	rc = gw_template_parser(jt);

    return rc;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_template_print(gw_template_t *jt)
{
	int i;
	
	fprintf(stderr," ----------- Job configuration file (%s) values -----------\n",jt->file);
	fprintf(stderr,"  %-23s: %s\n","NAME",GWNSTR(jt->name));
	fprintf(stderr,"  %-23s: %s\n","EXECUTABLE",GWNSTR(jt->executable));
	fprintf(stderr,"  %-23s: %s\n","ARGUMENTS",GWNSTR(jt->arguments));

	fprintf(stderr,"  %-14s(Total %i):\n","INPUT_FILES",jt->num_input_files);
	for (i=0;i<jt->num_input_files;i++)
		fprintf(stderr,"    (%i) Local: %-25s - Remote: %s\n",i,
			GWNSTR(jt->input_files[i][0]),
			GWNSTR(jt->input_files[i][1]));

	fprintf(stderr,"  %-14s(Total %i):\n","OUTPUT_FILES",jt->num_output_files);
	for (i=0;i<jt->num_output_files;i++)
		fprintf(stderr,"    (%i) Local: %-25s - Remote: %s\n",i,
			GWNSTR(jt->output_files[i][0]),
			GWNSTR(jt->output_files[i][1]));

	fprintf(stderr,"  %-14s(Total %i):\n","RESTART_FILES",jt->num_restart_files);
	for (i=0;i<jt->num_restart_files;i++)
		fprintf(stderr,"    (%i) File: %-25s\n",i,GWNSTR(jt->restart_files[i]));
			
	if ( jt->num_env > 0 )
	{
		fprintf(stderr,"  %-14s(Total %i):\n","ENVIRONMENT",jt->num_env);
		for (i=0;i<jt->num_env;i++)
			fprintf(stderr,"    (%i) %-16s = %s\n",i,
				GWNSTR(jt->environment[i][0]),
				GWNSTR(jt->environment[i][1]));
	}

	fprintf(stderr,"  %-23s: %s\n","STDIN_FILE",GWNSTR(jt->stdin_file));
    fprintf(stderr,"  %-23s: %s\n","STDOUT_FILE",GWNSTR(jt->stdout_file));
	fprintf(stderr,"  %-23s: %s\n","STDERR_FILE",GWNSTR(jt->stderr_file));
	fprintf(stderr,"  %-23s: %s\n","REQUIREMENTS",GWNSTR(jt->requirements));
	fprintf(stderr,"  %-23s: %s\n","RANK",GWNSTR(jt->rank));
	fprintf(stderr,"  %-23s: %i\n","RESCHEDULING_INTERVAL",(int)jt->rescheduling_interval);
	fprintf(stderr,"  %-23s: %i\n","RESCHEDULING_THRESHOLD",(int)jt->rescheduling_threshold);
	fprintf(stderr,"  %-23s: %i\n","SUSPENSION_TIMEOUT",(int)jt->suspension_timeout);
	fprintf(stderr,"  %-23s: %i\n","CPULOAD_THRESHOLD",jt->cpuload_threshold);
    fprintf(stderr,"  %-23s: %s\n","RESCHEDULE_ON_FAILURE",jt->reschedule_on_failure?"yes":"no");
	fprintf(stderr,"  %-23s: %i\n","NUMBER_OF_RETRIES",jt->number_of_retries);
	fprintf(stderr,"  %-23s: %i\n","CHECKPOINT_INTERVAL",(int)jt->checkpoint_interval);
	fprintf(stderr,"  %-23s: %s\n","CHECKPOINT_URL",jt->checkpoint_url);
	fprintf(stderr,"  %-23s: %s\n","WRAPPER",GWNSTR(jt->wrapper));
	fprintf(stderr,"  %-23s: %s\n","MONITOR",GWNSTR(jt->monitor));
	fprintf(stderr,"  %-23s: %s\n","PRE_WRAPPER",GWNSTR(jt->pre_wrapper));		
	fprintf(stderr,"  %-23s: %s\n","PRE_WRAPPER_ARGUMENTS",GWNSTR(jt->pre_wrapper_arguments));
	
	if ( jt->job_deps[0] != -1 )
	{
		fprintf(stderr,"  %-23s:\n","JOB_DEPENDENCIES");	
	
		i=0;
		while ( jt->job_deps[i] != -1 )
		{
			fprintf(stderr,"    (%i) JOB = %i\n",i,jt->job_deps[i]);
			i++;
		}
	}

    fprintf(stderr,"  %-23s: %s\n","TYPE",gw_template_jobtype_string(jt->type));
    fprintf(stderr,"  %-23s: %d\n","NP",jt->np);
		
    fprintf(stderr,"  %-23s: %s\n","DEADLINE",gw_template_deadline_string(jt->deadline));

    fprintf(stderr," ----------------------------------------------------------\n");
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

char *gw_template_jobtype_string(gw_jobtype_t type)
{
    switch (type)
    {
    case GW_JOB_TYPE_SINGLE:
        return "single";
    case GW_JOB_TYPE_MULTIPLE:
        return "multiple";
    case GW_JOB_TYPE_MPI:
        return "mpi";
    default:
        return "unknown";
    }
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

char *gw_template_deadline_string(time_t deadline)
{
    char buf[500];

    sprintf(buf, "%ld:%02ld:%02ld", deadline/60/60/24, deadline/60/60%24, deadline/60%60);

    return strdup(buf);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
