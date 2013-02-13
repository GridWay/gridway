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

%{
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <limits.h>

#include "gw_template.h"

#define YY_DECL static int yylex_jt(gw_template_t *jt)

void gw_template_set_value_str (gw_template_t *jt,
        gw_template_var_t var, 
        const char *val);

void gw_template_set_value_int (gw_template_t *jt,
        gw_template_var_t var,
        int val);
        
static void jtp_error(gw_template_t *jt, const char *str);

gw_template_var_t jtp_var;

char *jtp_val_str = NULL;
int jtp_val_int;
gw_boolean_t jtp_val_yn;
char *jtp_val_files = NULL;
char *jtp_val_env = NULL;

pthread_mutex_t jtp_mutex = PTHREAD_MUTEX_INITIALIZER;

int file;
int find;
char *GW_LOCATION;

int dl_comp;
int dl[3];

%}

%x var_int
%x var_yn
%x var_str
%x var_files
%x var_env
%x var_dl
%x val_int
%x val_yn
%x val_str
%x val_files
%x val_env
%x val_env_val
%x val_dl
%option prefix="jtp_"
%option yylineno
%option nounput
%option outfile="lex.yy.c"

%%

    /* --------------------------------------------------------------------- */
    /*    Parse comments                                                     */
    /* --------------------------------------------------------------------- */
    
#.*\n       ;
             
    /* --------------------------------------------------------------------- */
    /*    Variable Parsing                                                   */
    /* --------------------------------------------------------------------- */

EXECUTABLE              { jtp_var = EXECUTABLE;             BEGIN var_str;}
ARGUMENTS               { jtp_var = ARGUMENTS;              BEGIN var_str;}
ENVIRONMENT             { jtp_var = ENVIRONMENT;   jt->num_env=0; BEGIN var_env;}
INPUT_FILES             { jtp_var = INPUT_FILES;   file= 0; find = 0; BEGIN var_files;}
OUTPUT_FILES            { jtp_var = OUTPUT_FILES;  file= 0; find = 1; BEGIN var_files;}
RESTART_FILES           { jtp_var = RESTART_FILES; file= 0; find = 2; BEGIN var_files;}
STDERR_FILE             { jtp_var = STDERR_FILE;            BEGIN var_str;}
STDIN_FILE              { jtp_var = STDIN_FILE;             BEGIN var_str;}
STDOUT_FILE             { jtp_var = STDOUT_FILE;            BEGIN var_str;}
PRE_WRAPPER             { jtp_var = PRE_WRAPPER;            BEGIN var_str;}
PRE_WRAPPER_ARGUMENTS   { jtp_var = PRE_WRAPPER_ARGUMENTS;  BEGIN var_str;}
RANK                    { jtp_var = RANK;                   BEGIN var_str;}
REQUIREMENTS            { jtp_var = REQUIREMENTS;           BEGIN var_str;}
WRAPPER                 { jtp_var = WRAPPER;                BEGIN var_str;}
MONITOR                 { jtp_var = MONITOR;                BEGIN var_str;}
RESCHEDULE_ON_FAILURE   { jtp_var = RESCHEDULE_ON_FAILURE;  BEGIN var_yn;}
CPULOAD_THRESHOLD       { jtp_var = CPULOAD_THRESHOLD;      BEGIN var_int;}
NUMBER_OF_RETRIES       { jtp_var = NUMBER_OF_RETRIES;      BEGIN var_int;}
RESCHEDULING_INTERVAL   { jtp_var = RESCHEDULING_INTERVAL;  BEGIN var_int;}
RESCHEDULING_THRESHOLD  { jtp_var = RESCHEDULING_THRESHOLD; BEGIN var_int;}
SUSPENSION_TIMEOUT      { jtp_var = SUSPENSION_TIMEOUT;     BEGIN var_int;}
CHECKPOINT_INTERVAL     { jtp_var = CHECKPOINT_INTERVAL;    BEGIN var_int;}
CHECKPOINT_URL          { jtp_var = CHECKPOINT_URL;         BEGIN var_str;}
JOB_DEPENDENCIES        { jtp_var = JOB_DEPENDENCIES;       BEGIN var_str;}
TYPE                    { jtp_var = TYPE;                   BEGIN var_str;}
NP                      { jtp_var = NP;                     BEGIN var_int;}
DEADLINE                { jtp_var = DEADLINE;               BEGIN var_dl;}
NAME                    { jtp_var = NAME;                   BEGIN var_str;}

    /* --------------------------------------------------------------------- */
    /*    Assignment Parsing                                                 */
    /* --------------------------------------------------------------------- */

<var_int>[ \t]*=[ \t]*      { BEGIN val_int;}
<var_yn>[ \t]*=[ \t]*       { BEGIN val_yn;}
<var_str>[ \t]*=[ \t]*      { BEGIN val_str;}
<var_files>[ \t]*=[ \t]*    { BEGIN val_files;}
<var_env>[ \t]*=[ \t]*      { BEGIN val_env;}
<var_dl>[ \t]*=[ \t]*       { BEGIN val_dl;}

    /* --------------------------------------------------------------------- */
    /*    Value Parsing                                                      */
    /* --------------------------------------------------------------------- */

<val_int>[ \t]*(#.*)?\n { BEGIN INITIAL;}
<val_int>[0-9]+         { gw_template_set_value_int(jt, jtp_var, atoi(yytext));}

<val_yn>[ \t]*(#.*)?\n  { BEGIN INITIAL;}
<val_yn>yes|YES|Yes     { gw_template_set_value_int(jt, jtp_var, GW_TRUE);}
<val_yn>no|NO|No        { gw_template_set_value_int(jt, jtp_var, GW_FALSE);}
<val_yn>.               { fprintf(stderr,"Parse error at line %i: expected \"yes\" or \"no\"\n",
                          yylineno);
                          return -1;}

<val_str>[ \t]*(#.*)?\n { BEGIN INITIAL;} 
<val_str>\"[^\"]*\"     { yytext[strlen(yytext)-1]= '\0';
                          gw_template_set_value_str(jt, jtp_var, yytext+1);}
<val_str>.*             { gw_template_set_value_str(jt, jtp_var, yytext);}

<val_files>[ \t]*(#.*)?\n   { BEGIN INITIAL;}
<val_files>[ \t]*,[ \t]*    { file = 0;}
<val_files>[ \t]*           { file++;}
<val_files>[^, \n]*         { switch (file)
                              {
                                  case 0:
                                      switch(find)
                                      {
                                      case 0:
                                          jt->num_input_files++;
                                          
                                          if (jt->num_input_files==GW_JT_FILES)
                                          {
                                              fprintf(stderr,"Max number of input files reached at line %i\n",yylineno);
                                              return -1;
                                          }
                                          else
                                              strncpy(jt->input_files[jt->num_input_files-1][0],yytext,GW_JT_STR);
                                          break;
                                        case 1:
                                          jt->num_output_files++;
                                          if (jt->num_output_files==GW_JT_FILES)
                                          {
                                              fprintf(stderr,"Max number of output files reached at line %i\n",yylineno);
                                              return -1;
                                          }                                          
                                          else                                          
                                              strncpy(jt->output_files[jt->num_output_files-1][0],yytext,GW_JT_STR);
                                          break;
                                        case 2:
                                          jt->num_restart_files++;
                                          if (jt->num_restart_files==GW_JT_FILES)
                                          {
                                              fprintf(stderr,"Max number of restart files reached at line %i\n",yylineno);
                                              return -1;
                                          }                                              
                                          else                                          
                                              strncpy(jt->restart_files[jt->num_restart_files-1],yytext,GW_JT_STR);
                                          break;
                                      }                                      
                                    break;
                            
                                  case 1:
                                      switch(find)
                                      {
                                      case 0:
                                          strncpy(jt->input_files[jt->num_input_files-1][1],yytext,GW_JT_STR);
                                          break;
                                        case 1:
                                          strncpy(jt->output_files[jt->num_output_files-1][1],yytext,GW_JT_STR);
                                          break;
                                        case 2:
                                          fprintf(stderr,"Parse error at line %i\n", yylineno);
                                          return -1;
                                      }                                  
                                    break;
                                      
                                  default:
                                     fprintf(stderr,"Parse error at line %i\n", yylineno);
                                     return -1;
                               }
                             }

<val_env>[ \t]*(#.*)?\n { BEGIN INITIAL;}

<val_env>[ \t]*=[ \t]*  { BEGIN val_env_val;}
<val_env>[^, =\n]*      { jt->num_env++;
                          strncpy(jt->environment[jt->num_env-1][0],yytext,GW_JT_STR);
                        }
<val_env>.              {jtp_error(jt, "ENVIRONMENT attribute (var)"); return -1;}
                        
<val_env_val>\"[^\"]*\" { yytext[strlen(yytext)-1]= '\0';
                          strncpy(jt->environment[jt->num_env-1][1],yytext+1,GW_JT_STR);}
<val_env_val>[^, \n]*   { strncpy(jt->environment[jt->num_env-1][1],yytext,GW_JT_STR);}
<val_env_val>[ \t]*,[ \t]*    BEGIN val_env;
<val_env_val>[ \t]*(#.*)?\n { BEGIN INITIAL;}
<val_env_val>.              {jtp_error(jt, "ENVIRONMENT attribute (value)"); return -1;}
                      

<val_dl>[ \t]*(#.*)?\n  { switch (dl_comp)
                          {
                              case 1:
                                  jt->deadline = dl[0]*60;
                                  break;
                              case 2:
                                  jt->deadline = dl[0]*60*60 + dl[1]*60;
                                  break;
                              case 3:
                                  jt->deadline = dl[0]*24*60*60 + dl[1]*60*60 + dl[2]*60;
                                  break;
                              default:
                                 jtp_error(jt, "Bad deadline format"); return -1;
                          }
                          BEGIN INITIAL;}
<val_dl>:[0-9]+         { if (dl_comp >= 3 )
                        {
                             jtp_error(jt, "Bad deadline format");
                             return -1;
                        }
                        dl[dl_comp++] = atoi(yytext+1);}
<val_dl>[0-9]+          { dl[0] = atoi(yytext); dl_comp = 1; }

    /* --------------------------------------------------------------------- */
    /*    Default Parsing                                                    */
    /* --------------------------------------------------------------------- */

[ \t\n]+        ;
[^ \t\n=#]+     { fprintf(stderr,"Undefined variable (%s) at line %i\n",
                        yytext, yylineno);
                  return -1;}
%%

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int jtp_wrap(void)
{
    return 1;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void jtp_error(gw_template_t *jt, const char *str)
{
    fprintf(stderr,"Parse Error %s (line %i)\n",str,yylineno);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_template_set_value_str (gw_template_t *jt,
        gw_template_var_t var, 
        const char *val)
{
    char *tmp;
    char *num;
    int  i;
    
    if ( val==NULL )
    {
        fprintf(stderr,"Warning!, Variable value not defined at line %i\n", yylineno);
        return;
    }
    
    switch (var)
    {
        case EXECUTABLE:
            strncpy(jt->executable,val,GW_JT_STR);
            break;
        
        case ARGUMENTS:
            strncpy(jt->arguments,val,GW_JT_STR);
            break;
        
        case PRE_WRAPPER:
            strncpy(jt->pre_wrapper,val,GW_JT_STR);
            break;
        
        case PRE_WRAPPER_ARGUMENTS:
            strncpy(jt->pre_wrapper_arguments,val,GW_JT_STR);
            break;
        
        case RANK:
            strncpy(jt->rank,val,GW_JT_STR);
            break;
        
        case REQUIREMENTS:
            strncpy(jt->requirements,val,GW_JT_STR);
            break;
        
        case STDERR_FILE:
            strncpy(jt->stderr_file,val,GW_JT_STR);
            break;
        
        case STDIN_FILE:
            strncpy(jt->stdin_file,val,GW_JT_STR);
            break;
        
        case STDOUT_FILE:
            strncpy(jt->stdout_file,val,GW_JT_STR);
            break;
        
        case CHECKPOINT_URL:
            strncpy(jt->checkpoint_url,val,GW_JT_STR);
            break;
        
        case WRAPPER:
            if (val[0] != '/')
                snprintf(jt->wrapper,GW_JT_STR,"%s/%s", GW_LOCATION, val);
            else
                strncpy(jt->wrapper,val,GW_JT_STR);
            break;
            
        case MONITOR:
            if (val[0] != '/')
                snprintf(jt->monitor,GW_JT_STR,"%s/%s", GW_LOCATION, val);
            else
                strncpy(jt->monitor,val,GW_JT_STR);
            break;
            
        case JOB_DEPENDENCIES:
            tmp = strdup(val);
            i = 0;
            jt->job_deps[0] = -1;
            
            num = strtok (tmp," ");
            
            while ( (num != NULL) && (i<(GW_JT_DEPS -1)) )
            {
                jt->job_deps[i++] = atoi(num);
                num = strtok (NULL," ");
            }
            
            jt->job_deps[i] = -1;
            free(tmp);
            break;

        case TYPE:
            if (strcasecmp(val,"mpi") == 0)
                jt->type = GW_JOB_TYPE_MPI;
            else if (strcasecmp(val,"multiple") == 0)
                jt->type = GW_JOB_TYPE_MULTIPLE;
            else if (strcasecmp(val,"single") == 0)
                jt->type = GW_JOB_TYPE_SINGLE;
            else
                fprintf(stderr,"Bad job type \"%s\", line %i\n",val,jtp_lineno);
            break;

        case NAME:
            strncpy(jt->name,val,GW_JT_STR);
            break;

        default:
            fprintf(stderr,"Unexpected error setting str variable, line %i\n",jtp_lineno);
            break;
    }
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_template_set_value_int (gw_template_t *jt,
        gw_template_var_t var,
        int val)
{
    switch (var)
    {
        case CPULOAD_THRESHOLD:
            jt->cpuload_threshold = val;
            break;

        case NUMBER_OF_RETRIES:
            jt->number_of_retries = val;
            break;
        
        case RESCHEDULE_ON_FAILURE:
            jt->reschedule_on_failure = val;
            break;
        
        case RESCHEDULING_INTERVAL:
            jt->rescheduling_interval = val;
            break;
        
        case RESCHEDULING_THRESHOLD:
            jt->rescheduling_threshold = val;
            break;

        case CHECKPOINT_INTERVAL:
            jt->checkpoint_interval = val;
            break;

        case SUSPENSION_TIMEOUT:
            jt->suspension_timeout = val;
            break;
        
        case NP:
            jt->np = val;
            break;
        
        default:
            fprintf(stderr,"Unexpected error setting int variable, line %i\n",jtp_lineno);
            break;
    }
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
int gw_template_parser (gw_template_t *jt)
{
    int rc;
    char template_file[PATH_MAX];
    
    pthread_mutex_lock(&jtp_mutex);

    GW_LOCATION = getenv("GW_LOCATION");   
    
    if (GW_LOCATION == NULL )
    {
        fprintf(stderr,"GW_LOCATION is not defined\n");
        pthread_mutex_unlock(&jtp_mutex);        
        return -1;
    }

    /* Default job template parsing */         

    sprintf(template_file,"%s/" GW_ETC_DIR "/job_template.default",GW_LOCATION);

    yyin = fopen (template_file, "r");

    if (yyin == NULL)
    {
        fprintf(stderr,"Error opening default template file (%s)\n",template_file);
    }
    else
    {    
        jtp_lineno = 0;
        rc = yylex_jt(jt);
    
        fclose(yyin);

        if ( rc != 0 )
        {
            pthread_mutex_unlock(&jtp_mutex);        
            return rc;
        }
    }

    /* Job template parsing */
    
    if(strcmp(jt->file,"stdin")==0)
		yyin = stdin;
	else
	{
	    sprintf(template_file,"%s/%s",jt->job_home,jt->file);            
    	yyin = fopen (template_file, "r");
	}



    if (yyin == NULL)
    {
        fprintf(stderr,"Error opening job template file (%s)\n",template_file);
        pthread_mutex_unlock(&jtp_mutex);        
        return -1;
    }
    
    jtp_lineno = 0;
    rc = yylex_jt(jt);

    fclose(yyin);
    
    pthread_mutex_unlock(&jtp_mutex);    
    return rc;
}
