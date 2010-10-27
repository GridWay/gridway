/* -------------------------------------------------------------------------- */
/* Copyright 2002-2010, GridWay Project Leads (GridWay.org)                   */
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

#include "gw_log.h"
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

static gw_log_t gw_log;

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_log_init(const char *log_file)
{
  int truncate_result;
    pthread_mutex_init(&(gw_log.mutex),(pthread_mutexattr_t *) NULL);

    pthread_mutex_lock(&(gw_log.mutex));
    
    if (log_file != NULL)
    {
        gw_log.log_file = strdup(log_file);
        truncate_result = truncate(log_file, 0);        
    }
    else
        gw_log.log_file = NULL;    
        
    pthread_mutex_unlock(&(gw_log.mutex));
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_log_destroy()
{
    pthread_mutex_lock(&(gw_log.mutex));

    if (gw_log.log_file != NULL)
        free(gw_log.log_file);
        
    pthread_mutex_unlock(&(gw_log.mutex));
    
    pthread_mutex_destroy(&(gw_log.mutex));               
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_log_print(const char *module, const char type, const char *str_format,...)
{
    FILE    *log;
    va_list ap;
    time_t  the_time;
    
    char str[26];

    va_start (ap, str_format);
    
    pthread_mutex_lock(&(gw_log.mutex));
    
    log = fopen(gw_log.log_file,"a");

    if (log != NULL)
    {
        the_time = time(NULL);

#ifdef GWSOLARIS
        ctime_r(&(the_time),str,sizeof(char)*26);
#else
        ctime_r(&(the_time),str);
#endif

        str[24]='\0';

        fprintf(log,"%s [%s][%c]: ", str, module, type);
        vfprintf(log,str_format,ap);
        
        fclose(log);
    }
    
    pthread_mutex_unlock(&(gw_log.mutex));
}
