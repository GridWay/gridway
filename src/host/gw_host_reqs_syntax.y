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
#include <stdlib.h>
#include <string.h>
#include <fnmatch.h>
#include "gw_host_reqs_syntax.h"
#include "gw_common.h"
#include "gw_host.h"
#include "gw_log.h"

#define host_reqs_lex host_attr_lex
int host_attr_lex (YYSTYPE *lvalp, YYLTYPE *llocp);
void host_reqs_error(YYLTYPE *llocp, gw_host_t *host, int queue, gw_boolean_t *result, int *pos, const char *str);
char *value_str;
%}

%parse-param {gw_host_t *host}
%parse-param {int queue}
%parse-param {gw_boolean_t *result}
%parse-param {int *pos}

%union {
    int val_int;
    char *val_str;
};

%defines
%locations
%pure_parser
%name-prefix="host_reqs_"

%left '!' '&' '|'
%token <val_int> VARIABLE
%token <val_str> GENERICVAR
%token <val_int> INTEGER
%token <val_str> STRING
%type  <val_int> stmt expr
%%

stmt:   expr                     { *result=$1;}
        | expr';'                { *result=$1;}
        |                        { *result=GW_TRUE;} /* TRUE BY DEFAULT, ON EMPTY STRINGS */
	    | ';'                    { *result=GW_TRUE;} /* TRUE BY DEFAULT, ON EMPTY STRINGS */
        ;
expr:   VARIABLE '=' INTEGER        { $$ = gw_host_get_var_int($1,queue,host) == $3; }
		| VARIABLE '!' '=' INTEGER  { $$ = gw_host_get_var_int($1,queue,host) != $4; }
        | VARIABLE '>' INTEGER      { $$ = gw_host_get_var_int($1,queue,host) > $3 ; }
        | VARIABLE '<' INTEGER      { $$ = gw_host_get_var_int($1,queue,host) < $3 ; }
        | VARIABLE '=' STRING       { value_str = gw_host_get_var_str($1,queue,host);
	                              if ( value_str != NULL
				            && fnmatch($3, value_str, 0) == 0)
                                        $$ = 1;
                                      else
                                        $$ = 0;
                                      free($3); }                                      
        | VARIABLE '!''=' STRING    { value_str = gw_host_get_var_str($1,queue,host);
	                             if ( value_str != NULL
				          && fnmatch($4, value_str, 0) != 0)
                                        $$ = 1;
                                      else
                                        $$ = 0;
                                      free($4); }
                                      
        | GENERICVAR '=' INTEGER    { $$ = gw_host_get_genvar_int($1,queue,host) == $3; free($1);}
        | GENERICVAR '!''=' INTEGER { $$ = gw_host_get_genvar_int($1,queue,host) != $4; free($1);}        
        | GENERICVAR '>' INTEGER    { $$ = gw_host_get_genvar_int($1,queue,host) > $3 ; free($1);}
        | GENERICVAR '<' INTEGER    { $$ = gw_host_get_genvar_int($1,queue,host) < $3 ; free($1);}
        | GENERICVAR '=' STRING     { value_str = gw_host_get_genvar_str($1,queue,host);
                                      if ( value_str != NULL
                                          && fnmatch($3, value_str, 0) == 0)
                                        $$ = 1;
                                      else if ( value_str == NULL
                                          && strlen($3) == 0)
                                        $$ = 1;
                                      else
                                        $$ = 0;
                                      free($3); free($1);}
        | GENERICVAR '!''=' STRING  { value_str = gw_host_get_genvar_str($1,queue,host);
                                      if ( value_str != NULL
                                          && fnmatch($4, value_str, 0) != 0)
                                        $$ = 1;
                                      else if ( value_str != NULL
                                          && strlen($4) == 0)
                                        $$ = 1;
                                      else
                                        $$ = 0;
                                      free($4); free($1);}
                                      
        | expr '&' expr             { $$ = $1 && $3; }
        | expr '|' expr             { $$ = $1 || $3; }
        | '!' expr                  { $$ = ! $2; }
        | '(' expr ')'              { $$ =   $2; }
        ;


%%
void host_reqs_error(YYLTYPE *llocp, gw_host_t *host, int queue, gw_boolean_t *result, int *pos, const char *str)
{
    gw_log_print("IM",'E',"Syntax error %s at columns %i-%i.\n", str, llocp->first_column, llocp->last_column);
    *pos = llocp->first_column;
}
