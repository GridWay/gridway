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

#include "gw_log.h"
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

static gw_log_t gw_log;

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_log_init(const char *log_file)
{
  int truncate_result;
    pthread_mutex_init(&(gw_log.mutex),(pthread_mutexattr_t *) NULL);

    pthread_mutex_lock(&(gw_log.mutex));

#ifndef GWSYSLOG
    if (log_file != NULL)
    {
        gw_log.log_file = strdup(log_file);
        truncate_result = truncate(log_file, 0);        
    }
    else
        gw_log.log_file = NULL;    
#else
    openlog("GridWay", LOG_PID, GWSYSLOG);
#endif
    
    pthread_mutex_unlock(&(gw_log.mutex));
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_log_destroy()
{
    pthread_mutex_lock(&(gw_log.mutex));

#ifndef GWSYSLOG
    if (gw_log.log_file != NULL)
        free(gw_log.log_file);
#else
    closelog();
#endif
 
    pthread_mutex_unlock(&(gw_log.mutex));
    
    pthread_mutex_destroy(&(gw_log.mutex));               
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_log_print(const char *module, const char type, const char *str_format,...)
{
#ifndef GWSYSLOG
    FILE    *log;
    //va_list ap;
    time_t  the_time;    
    char str[26];
#else
    char *str_syslog;
    str_syslog = (char*) malloc(sizeof(char)*(10+strlen(str_format)));
    sprintf(str_syslog, "[%s][%c] %s", module, type, str_format);
#endif
 
    va_list ap;
    va_start (ap, str_format);
    
    pthread_mutex_lock(&(gw_log.mutex));

#ifndef GWSYSLOG
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
#else
    switch(type)
    {
        case 'I':
            vsyslog(LOG_INFO, str_syslog, ap);
            break;
        case 'E':
            vsyslog(LOG_ERR, str_syslog, ap);
            break;
        case 'W':
            vsyslog(LOG_WARNING, str_syslog, ap);
            break;
        case 'D':
            vsyslog(LOG_DEBUG, str_syslog, ap);
            break;
    }
    free(str_syslog);
#endif

    pthread_mutex_unlock(&(gw_log.mutex));
}
