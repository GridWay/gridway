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
#include <stdlib.h>
#include <string.h>
#include "gw_conf.h"
#include "gw_sch_conf_syntax.h"
#include "gw_log.h"

int  gw_sched_lex  (YYSTYPE *lvalp);
void gw_sched_error(gw_sch_conf_t *conf, const char *str);
%}

%parse-param {gw_sch_conf_t *conf}

%union {
    int    val_int;
    float  val_float;
    char * val_str;
};

%defines
%pure_parser
%name-prefix="gw_sched_"
%error-verbose

%token <val_int>   VARIABLE
%token <val_int>   INTEGER
%token <val_float> FLOAT
%token <val_str>   STRING

%%

stmt: stmt asig             { }
      | /* NULL */          { }
        ;

asig:
        VARIABLE '=' INTEGER                { gw_sch_set_var (conf,$1,(float) $3);}
      | VARIABLE '=' FLOAT                  { gw_sch_set_var (conf,$1,$3);}        
      | VARIABLE '[' STRING ']' '=' INTEGER { gw_sch_set_svar(conf,$1,$3,$6);}
        ;

%%

void gw_sched_error(gw_sch_conf_t *conf, const char *str)
{
    gw_log_print("GW",'E',"sched.conf: %s.\n",str);
}

