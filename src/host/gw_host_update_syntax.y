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

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gw_host_update_syntax.h"
#include "gw_host.h"
#include "gw_log.h"

#define host_update_lex host_attr_lex
int host_attr_lex (YYSTYPE *lvalp, YYLTYPE *llocp);
void host_update_error(YYLTYPE *llocp, gw_host_t *host, int *pos, const char *str);
%}

%parse-param {gw_host_t *host}
%parse-param {int *pos}

%union {
    int val_int;
    char *val_str;
};

%defines
%locations
%pure_parser
%name-prefix="host_update_"

%left '!' '&' '|'
%token <val_int> VARIABLE
%token <val_str> GENERICVAR
%token <val_int> INTEGER
%token <val_str> STRING
%type  <void> stmt asig

%%

stmt:   stmt asig                               { }
      | /* NULL */                              { }
        ;
asig:
        VARIABLE '=' INTEGER                    { gw_host_set_var_int($1,-1,$3,host);}
      | VARIABLE '=' STRING                     { gw_host_set_var_str($1,-1,$3,host);}
      | VARIABLE '[' INTEGER ']' '=' INTEGER    { gw_host_set_var_int($1,$3,$6,host);}
      | VARIABLE '[' INTEGER ']' '=' STRING     { gw_host_set_var_str($1,$3,$6,host);}
      | GENERICVAR '=' INTEGER                  { gw_host_set_genvar_int($1,-1,$3,host);free($1);}
      | GENERICVAR '=' STRING                   { gw_host_set_genvar_str($1,-1,$3,host);free($1);}
      | GENERICVAR '[' INTEGER ']' '=' INTEGER  { gw_host_set_genvar_int($1,$3,$6,host);free($1);}
      | GENERICVAR '[' INTEGER ']' '=' STRING   { gw_host_set_genvar_str($1,$3,$6,host);free($1);}
      | VARIABLE '='                            { gw_host_set_var_null($1,-1,host);}
      | VARIABLE '[' INTEGER ']' '='            { gw_host_set_var_null($1,$3,host);}      
      | GENERICVAR '='                          { gw_host_set_genvar_null($1,-1,host);free($1);}
      | GENERICVAR '[' INTEGER ']' '='          { gw_host_set_genvar_null($1,$3,host);free($1);}
        ;
%%

void host_update_error(YYLTYPE *llocp, gw_host_t *host, int *pos, const char *str)
{
    gw_log_print("IM",'E',"Syntax error %s at columns %i-%i.\n", str, llocp->first_column, llocp->last_column);
    *pos = llocp->first_column;
}
