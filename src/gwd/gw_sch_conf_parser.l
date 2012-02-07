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
#include <pthread.h>

#include "gw_sch_conf_syntax.h"
#include "gw_sch_conf.h"
#include "gw_log.h"

#define YY_DECL int  gw_sched_lex  (YYSTYPE *lvalp)
int gw_sched_parse (gw_sch_conf_t *conf);
%}

%option nounput
%option prefix="gw_sched_"
%option outfile="lex.yy.c"

%%

#.*\n           

FP_WEIGHT             { lvalp->val_int = FP_WEIGHT;            return VARIABLE;}
FP_USER               { lvalp->val_int = FP_USER;              return VARIABLE;}
FP_GROUP              { lvalp->val_int = FP_GROUP;             return VARIABLE;}
SH_WEIGHT             { lvalp->val_int = SH_WEIGHT;            return VARIABLE;}
SH_USER               { lvalp->val_int = SH_USER;              return VARIABLE;}
SH_WINDOW_DEPTH       { lvalp->val_int = SH_WINDOW_DEPTH;      return VARIABLE;}
SH_WINDOW_SIZE        { lvalp->val_int = SH_WINDOW_SIZE;       return VARIABLE;}
WT_WEIGHT             { lvalp->val_int = WT_WEIGHT;            return VARIABLE;}
DL_WEIGHT             { lvalp->val_int = DL_WEIGHT;            return VARIABLE;}
DL_HALF               { lvalp->val_int = DL_HALF;              return VARIABLE;}
RP_WEIGHT             { lvalp->val_int = RP_WEIGHT;            return VARIABLE;}
RP_HOST               { lvalp->val_int = RP_HOST;              return VARIABLE;}
RP_IM                 { lvalp->val_int = RP_IM;                return VARIABLE;}
RA_WEIGHT             { lvalp->val_int = RA_WEIGHT;            return VARIABLE;}
FR_MAX_BANNED         { lvalp->val_int = FR_MAX_BANNED;        return VARIABLE;}
FR_BANNED_C           { lvalp->val_int = FR_BANNED_C;          return VARIABLE;}
UG_WEIGHT             { lvalp->val_int = UG_WEIGHT;            return VARIABLE;}
UG_HISTORY_WINDOW     { lvalp->val_int = UG_HISTORY_WINDOW;    return VARIABLE;}
UG_HISTORY_RATIO      { lvalp->val_int = UG_HISTORY_RATIO;     return VARIABLE;}
DISABLE               { lvalp->val_int = DISABLE;              return VARIABLE;}
DISPATCH_CHUNK        { lvalp->val_int = DISPATCH_CHUNK;       return VARIABLE;}
MAX_DISPATCH_RESOURCE { lvalp->val_int = MAX_DISPATCH_RESOURCE; return VARIABLE;}
MAX_RUNNING_USER      { lvalp->val_int = MAX_RUNNING_USER;     return VARIABLE;}

-?[0-9]+          { lvalp->val_int   = atoi(yytext);   return INTEGER;}
Yes|yes|YES       { lvalp->val_int   = 1;              return INTEGER;}
No|no|NO          { lvalp->val_int   = 0;              return INTEGER;}
-?[0-9\.]+        { lvalp->val_float = atof(yytext);   return FLOAT;}
[0-9A-Za-z_\-\.]+ { lvalp->val_str   = strdup(yytext); return STRING;}

[=\[\]]         { return *yytext;}

[ \t\n]+
.               

%%

int gw_sched_wrap()
{
    return 1;
}

int gw_sch_loadconf(gw_sch_conf_t *conf, char *file)
{	
	int rc;

    yyin = fopen (file,"r");

    if (yyin == NULL)
    {
        gw_log_print("GW",'E',"sched.conf: Error opening file\n"); 
        fprintf(stderr,"Error opening sched.conf file (%s)\n",file);
        return -1;
    }
    
    rc = gw_sched_parse (conf);
            
    fclose(yyin);
    
    return rc;
}

