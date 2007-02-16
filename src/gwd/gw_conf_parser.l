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

%{
#include <stdio.h>
#include <string.h>
#include "gw_conf.h"
#include "gw_log.h"
#define STR_SIZE 512
#define YY_DECL static int yylex_gwd_conf()

void gcp_set_val_int(gw_conf_var_t var, int val);
void init_mad_parser(int mad_index);
void process_escaped_chars(char *str);

char ***gcp_mads;

gw_conf_var_t gcp_var;

int  gcp_i,gcp_j;
int  gcp_num  = 0;
int  gcp_max;

%}

%x var_int
%x var_mad
%x var_sch
%x val_int
%x val_mad
%x val_sch
%option prefix="gcp_"
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

GWD_PORT                { gcp_var = GWD_PORT;              BEGIN var_int;}
MAX_NUMBER_OF_CLIENTS   { gcp_var = MAX_NUMBER_OF_CLIENTS; BEGIN var_int;}
NUMBER_OF_ARRAYS        { gcp_var = NUMBER_OF_ARRAYS;      BEGIN var_int;}
NUMBER_OF_JOBS          { gcp_var = NUMBER_OF_JOBS;        BEGIN var_int;}
NUMBER_OF_HOSTS         { gcp_var = NUMBER_OF_HOSTS;       BEGIN var_int;}
NUMBER_OF_USERS         { gcp_var = NUMBER_OF_USERS;       BEGIN var_int;}
SCHEDULING_INTERVAL     { gcp_var = SCHEDULING_INTERVAL;   BEGIN var_int;}
DISCOVERY_INTERVAL      { gcp_var = DISCOVERY_INTERVAL;    BEGIN var_int;}
MONITORING_INTERVAL     { gcp_var = MONITORING_INTERVAL;   BEGIN var_int;}
POLL_INTERVAL           { gcp_var = POLL_INTERVAL;         BEGIN var_int;}
IM_MAD                  { gcp_var = IM_MAD; init_mad_parser(0); BEGIN var_mad;}
TM_MAD                  { gcp_var = TM_MAD; init_mad_parser(1); BEGIN var_mad;}
EM_MAD                  { gcp_var = EM_MAD; init_mad_parser(2); BEGIN var_mad;}
DM_SCHED                { gcp_var = DM_SCHED; gcp_j = 0;   BEGIN var_sch;}

    /* --------------------------------------------------------------------- */
    /*    Assignment Parsing                                                 */
    /* --------------------------------------------------------------------- */

<var_int>[ \t]*=[ \t]*  { BEGIN val_int;}
<var_mad>[ \t]*=[ \t]*  { BEGIN val_mad;}
<var_sch>[ \t]*=[ \t]*  { BEGIN val_sch;}

    /* --------------------------------------------------------------------- */
    /*    Value Parsing                                                      */
    /* --------------------------------------------------------------------- */

<val_int>[ \t]*(#.*)?\n { BEGIN INITIAL;}
<val_int>[0-9]+         { gcp_set_val_int(gcp_var, atoi(yytext));}

<val_mad>[ \t]*(#.*)?\n { BEGIN INITIAL;}
<val_mad>[^:\n]*:       { if ( yytext[yyleng-2] == '\\' )
                          {
                            yymore();
                          }
                          else
                          {
                            if ( gcp_mads != NULL )
                            {
                                yytext[yyleng-1] = '\0';
                                process_escaped_chars(yytext);
                                gcp_mads[gcp_num][gcp_j] = strdup(yytext);
                            }
                            gcp_j++;
                            if (gcp_j >= gcp_max)
                            {
                                gw_log_print("GW",'E',"gwd.conf: Parse error in MAD at line %i\n", yylineno);
                                return -1;
                            }
                          }
                        }
<val_mad>[^:\n]*        { if ( gcp_mads != NULL )
                          {
                            process_escaped_chars(yytext);
                            gcp_mads[gcp_num][gcp_j] = strdup(yytext);
                          }
                        }

<val_sch>[ \t]*(#.*)?\n { BEGIN INITIAL;}
<val_sch>:              { gcp_j++;
                          if (gcp_j >= 3)
                          {
                              gw_log_print("GW",'E',"gwd.conf: Parse error in Sched at line %i\n", yylineno);
                              return -1;
                          }
                        }
<val_sch>[^:\n]*        { gw_conf.dm_mad[gcp_j] = strdup(yytext);}

    /* --------------------------------------------------------------------- */
    /*    Default Parsing                                                    */
    /* --------------------------------------------------------------------- */

[ \t\n]+        ;
[^ \t\n=#]+     { gw_log_print("GW",'E',"Undefined variable (%s) at line %i\n",
                        yytext, yylineno);}

.               { gw_log_print("GW",'E',"gwd.conf: Parse error at line %i\n",yylineno);
                  return -1; }
%%

void gcp_set_val_int(gw_conf_var_t var, int val)
{
    switch(var)
    {
        case DISCOVERY_INTERVAL:
            gw_conf.discovery_interval = val;
            break;
            
        case GWD_PORT:
            gw_conf.gwd_port = val;
            break;

        case MONITORING_INTERVAL:
            gw_conf.monitoring_interval = val;
            break;        

        case MAX_NUMBER_OF_CLIENTS:
            gw_conf.max_number_of_clients = val;
            break;        
        
        case NUMBER_OF_ARRAYS:
            gw_conf.number_of_arrays = val;
            break;

        case NUMBER_OF_JOBS:
            gw_conf.number_of_jobs = val;
            break;

        case NUMBER_OF_HOSTS:
            gw_conf.number_of_hosts = val;
            break;

        case NUMBER_OF_USERS:
            gw_conf.number_of_users = val;
            break;
            		
		case POLL_INTERVAL:
			gw_conf.poll_interval = val;
			break;
			
		case SCHEDULING_INTERVAL:
			gw_conf.scheduling_interval = val;
			break;		

		default:
            gw_log_print("GW",'E',"Error! Unknown variable\n");
			break;
    }
}

void init_mad_parser(int mad_index)
{
	gcp_j = 0;
	
	switch (mad_index)
	{
		case 0:
			gcp_mads = gw_conf.im_mads;
            gcp_max  = 6;
			break;
			
		case 1:
			gcp_mads = gw_conf.tm_mads;
            gcp_max  = 3;
			break;
            
		case 2:
			gcp_mads = gw_conf.em_mads;
            gcp_max  = 3;
			break;
			
		default:
	        gw_log_print("GW",'E',"gwd.conf: Unexpected error parsing MAD, line %i\n",gcp_lineno);
	        break;	
	}
	
	gcp_num = 0;
	
	while ( gcp_mads[gcp_num][0] != NULL )
	{
	    gcp_num++;
	    if ( gcp_num == GW_MAX_MADS )
        {
            gw_log_print("GW",'W',"gwd.conf: Max. number of MAD reached, ignoring MAD, line %i\n",gcp_lineno);
            gcp_mads = NULL;
        }
    }
    
}

void process_escaped_chars(char *str)
{
    int i, j;
    
    for (i = 0, j = 0; str[i] != '\0'; i++)
    {
        if ( str[i] != '\\' )
            str[j++] = str[i];
    }
    
    str[j] = '\0';
}

int gcp_wrap(void)
{
    return 1;
}

int gw_conf_lex_parser ()
{
    int rc;
	    
    gcp_lineno = 0;
    yyin = fopen (gw_conf.conf_file,"r");

    if (yyin == NULL)
    {
        gw_log_print("GW",'E',"gwd.conf: Error opening file\n"); 
        fprintf(stderr,"Error opening gwd.conf file (%s)\n",gw_conf.conf_file);
        return -1;
    }
    
    rc = yylex_gwd_conf();
    
    fclose(yyin);
    
    return rc;
}
