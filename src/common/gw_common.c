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

#include "gw_common.h"
#include <time.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include "gw_job.h"

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

static gw_short_string_t gw_em_state_strings[] = {
  "----",
  "pend",
  "susp",
  "actv",
  "fail", 
  "done",
  "*"
};

static gw_short_string_t gw_job_state_strings[] = {
   "----",       
   "pend",
   "hold",
   "prol",
   "prew",
   "wrap",
   "epil",
   "epil",
   "epil",   
   "epil",   
   "canl",
   "epil",
   "stop",
   "canl",
   "epil",
   "migr",
   "migr",
   "migr",
   "done",
   "fail",
   "*"
};

static gw_short_string_t gw_ret_code_strings[] = {
  "success",
  "failed",
  "failed can not access file",
  "failed bad job id",
  "failed bad array id",
  "failed bad host id",  
  "failed wrong job state",
  "failed no memory",
  "failed can not creat thread",
  "failed client API not initialized",
  "failed connection to gwd",
  "failed the job was killed",
  "failed could not register user (check proxy)",
  "failed permission denied",
  "failed job template parse error",
  "failed job execution failed",
  "failed timeout expired",
  "*invalid return code*"
};

static gw_short_string_t gw_migration_reason_strings[] = {
  "----",
  "user",
  "susp",
  "disc",
  "self",
  "perf",
  "s/r ",
  "err ",
  "kill",
  "*"
};

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
 
char *gw_ret_code_string(gw_return_code_t code)
{
  if(code >= 0 && code < GW_RC_LIMIT)
    return gw_ret_code_strings[code];
  else 
    return gw_ret_code_strings[GW_RC_LIMIT]; 
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_print (FILE *fd, const char* module, const char mode, const char *str_format,...)
{
    va_list ap;
    time_t  the_time;
    
    char str[26];

    va_start (ap, str_format);

    if (fd != NULL)
    {
        the_time = time(NULL);

#ifdef GWSOLARIS
        ctime_r(&(the_time),str,sizeof(char)*26);
#else
        ctime_r(&(the_time),str);
#endif

        str[24]='\0';

        fprintf(fd,"%s [%s][%c]: ", str, module, mode);
        vfprintf(fd,str_format,ap);
    }
        
    return;
}

/*----------------------------------------------------------------------------*/

char *gw_job_state_string(gw_job_state_t state)
{
  if(state >= 0 && state < GW_JOB_STATE_LIMIT)
    return gw_job_state_strings[state];
  else 
    return gw_job_state_strings[GW_JOB_STATE_LIMIT];  
}

/*----------------------------------------------------------------------------*/

char *gw_em_state_string(gw_em_state_t state) 
{
  if(state >= 0 && state < GW_EM_STATE_LIMIT)
    return gw_em_state_strings[state];
  else 
    return gw_em_state_strings[GW_EM_STATE_LIMIT];  
}

/*----------------------------------------------------------------------------*/

char *gw_reason_string(gw_migration_reason_t reason)
{
    if (reason >= 0 && reason < GW_REASON_LIMIT)
        return gw_migration_reason_strings[reason];
    else
        return gw_migration_reason_strings[GW_REASON_LIMIT];
}
