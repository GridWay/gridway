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

%{
#include <stdio.h>
#include <string.h>
#include "gw_conf.h"
#include "gw_job.h"
#include "gw_common.h"
#include "gw_log.h"

#define YY_DECL int yylex_job_template_var(gw_job_t *job)

static void jtp_var_cat_int(int number);
static void jtp_var_cat_str(const char *str);
static int  jtp_var_env_search(gw_job_t *job, char *tmp);

char * jtp_var_parsed_value;

int    jtp_var_i=0;
char * tmp;
char * var_ptr;
int    env_i;
int    i, length_tmp;

pthread_mutex_t jtp_var_mutex = PTHREAD_MUTEX_INITIALIZER;
%}

%option prefix="jtp_var_"
%option nounput
%option outfile="lex.yy.c"

%%

\$\{ARRAY_ID\}    { jtp_var_cat_int(job->array_id);}
\$\{JOB_ID\}      { jtp_var_cat_int(job->id);}
\$\{TASK_ID\}     { jtp_var_cat_int(job->task_id);}
\$\{TOTAL_TASKS\} { jtp_var_cat_int(job->total_tasks);}
\$\{ARCH\}        { jtp_var_cat_str(job->history->host->arch);}
\$\{PARAM\}       { if (job->array_id != -1)
                      jtp_var_cat_int((job->pstart + (job->pinc*job->task_id)));
                    else
                      jtp_var_cat_int(0);
                  }
\$\{MAX_PARAM\}   { if (job->array_id != -1)
                      jtp_var_cat_int((job->pstart + (job->pinc*(job->total_tasks-1))));
                    else
                      jtp_var_cat_int(0);
                  }
\$\{[^\}]*\}      { 
                    length_tmp = yyleng;
					tmp = yytext+2;
					while (tmp[yyleng] != '}') yyleng--;
					tmp[yyleng] = '\0';
					
                    env_i = jtp_var_env_search(job,tmp);
                    if (env_i != -1)
                    	jtp_var_cat_str(job->template.environment[env_i][GW_ENV_VAL]);
                    else
                    {
                    	var_ptr = (char *) malloc (sizeof(char) * (length_tmp + 1));
                    	sprintf(var_ptr,"${%s}",tmp);
                    	jtp_var_cat_str(var_ptr);
                    	free(var_ptr);
                    }
                  }	
.                 { jtp_var_parsed_value[jtp_var_i++]=*yytext;}

%%

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

int jtp_var_env_search(gw_job_t *job, char *tmp)
{
	int i;
	
	for (i=0;i<job->template.num_env;i++)
		if ( strcmp(job->template.environment[i][GW_ENV_VAR],tmp) == 0 )
			return i;	
			
	return -1;
}

void jtp_var_cat_int(int number)
{
	char str[12];
	int  length;
	
	sprintf(str,"%i",number);
	strcat(jtp_var_parsed_value,str);
	
	length    = strlen(str);
	jtp_var_i = jtp_var_i + length;
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

void jtp_var_cat_str(const char *str)
{
	int  length;
	
	if (str==NULL)
		return;
		
	strcat(jtp_var_parsed_value,str);

	length    = strlen(str);
	jtp_var_i = jtp_var_i + length;
}

int jtp_var_wrap(void) {
return 1;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

char *gw_job_substitute (const char *var, gw_job_t *job)
{
	int length,i,rc;
	char *jtp_result;
	YY_BUFFER_STATE	str_buffer;
	
	if ( var == NULL )
		return NULL;
		
	pthread_mutex_lock(&jtp_var_mutex);	
	
	jtp_var_i=0;
	
	length = 2 * strlen(var);
	jtp_var_parsed_value = (char *) malloc (sizeof(char) * length);

    if (jtp_var_parsed_value == NULL)
    {
        gw_log_print("DM",'E',"Error (out of memory) parsing var \"%s\".\n",var);
        
        pthread_mutex_unlock(&jtp_var_mutex);
        return NULL;
    }
    	
	for (i=0;i<length;i++)
		jtp_var_parsed_value[i]='\0';
		
	str_buffer = yy_scan_string(var);

    if (str_buffer == NULL)
    {
        gw_log_print("DM",'E',"Lexical error parsing var \"%s\".\n",var);
        
        if (jtp_var_parsed_value != NULL)
	        free(jtp_var_parsed_value);
	        
	    pthread_mutex_unlock(&jtp_var_mutex);    
        return NULL;
    }
    
    rc = yylex_job_template_var(job);

	yy_delete_buffer(str_buffer);
	
	if ( rc == -1 )
	{
		if (jtp_var_parsed_value != NULL)
        	free(jtp_var_parsed_value);
        	
        pthread_mutex_unlock(&jtp_var_mutex);
        return NULL;
    }
	else
	{
		if (jtp_var_parsed_value != NULL)
		{	
			jtp_result = strdup(jtp_var_parsed_value);
			free(jtp_var_parsed_value);
		}
		else
			jtp_result = NULL;

        pthread_mutex_unlock(&jtp_var_mutex);			
    	return jtp_result;
    }
}
