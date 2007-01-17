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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>

#include "gw_host.h"
#include "gw_common.h"
#include "gw_conf.h"
#include "gw_log.h"

/* Local functions */
static int gw_host_search_genvar_int(char *name, gw_host_t *host);
static int gw_host_search_genvar_str(char *name, gw_host_t *host);

/*----------------------------------------------------------------------------*/

const char *gw_host_get_varname(gw_host_var_t var)
{
    switch(var)
    {
    case HOSTNAME:
        return "HOSTNAME";
    case ARCH:
        return "ARCH";
    case OS_NAME:
        return "OS_NAME";
    case OS_VERSION:
        return "OS_VERSION";
    case CPU_MODEL:
        return "CPU_MODEL";
    case CPU_MHZ:
        return "CPU_MHZ";
    case CPU_FREE:
        return "CPU_FREE";
    case CPU_SMP:
        return "CPU_SMP";
    case NODECOUNT:
        return "NODECOUNT";
    case SIZE_MEM_MB:
        return "SIZE_MEM_MB";
    case FREE_MEM_MB:
        return "FREE_MEM_MB";
    case SIZE_DISK_MB:
        return "SIZE_DISK_MB";
    case FREE_DISK_MB:
        return "FREE_DISK_MB";
    case FORK_NAME:
        return "FORK_NAME";
    case LRMS_NAME:
        return "LRMS_NAME";
    case LRMS_TYPE:
        return "LRMS_TYPE";
    case QUEUE_NAME:
        return "QUEUE_NAME";
    case QUEUE_NODECOUNT:
        return "QUEUE_NODECOUNT";
    case QUEUE_FREENODECOUNT:
        return "QUEUE_FREENODECOUNT";
    case QUEUE_MAXTIME:
        return "QUEUE_MAXTIME";
    case QUEUE_MAXCPUTIME:
        return "QUEUE_MAXCPUTIME";
    case QUEUE_MAXCOUNT:
        return "QUEUE_MAXCOUNT";
    case QUEUE_MAXRUNNINGJOBS:
        return "QUEUE_MAXRUNNINGJOBS";
    case QUEUE_MAXJOBSINQUEUE:
        return "QUEUE_MAXJOBSINQUEUE";
    case QUEUE_STATUS:
        return "QUEUE_STATUS";
    case QUEUE_DISPATCHTYPE:
        return "QUEUE_DISPATCHTYPE";
    case QUEUE_PRIORITY:
        return "QUEUE_PRIORITY";
    case GENERIC:
        return "GENERIC";
    }
    
    return "UNKNOWN";
}

/*----------------------------------------------------------------------------*/

void gw_host_set_var_int(gw_host_var_t var, int index, int value, gw_host_t *host)
{
    if (index >= GW_HOST_MAX_QUEUES)
    {
        gw_log_print("IM", 'E', "Max number of queues exceeded in variable %s.\n",
                gw_host_get_varname(var));
        return;
    }

    switch(var)
    {
        case CPU_MHZ:
            host->cpu_mhz = value;
            break;

        case CPU_FREE:
            host->cpu_free = value;
            break;
            
        case CPU_SMP:
            host->cpu_smp = value;
            break;

        case NODECOUNT:
            host->nodecount = value;
            break;
            
        case SIZE_MEM_MB:
            host->size_mem_mb = value;
            break;

        case FREE_MEM_MB:
            host->free_mem_mb = value;
            break;

        case SIZE_DISK_MB:
            host->size_disk_mb = value;
            break;

        case FREE_DISK_MB:
            host->free_disk_mb = value;
            break;

        case QUEUE_NODECOUNT:
            host->queue_nodecount[index] = value;
            break;
            
        case QUEUE_FREENODECOUNT:
            host->queue_freenodecount[index] = value;
            break;

        case QUEUE_MAXTIME:
            host->queue_maxtime[index] = value;
            break;

        case QUEUE_MAXCPUTIME:
            host->queue_maxcputime[index] = value;
            break;

        case QUEUE_MAXCOUNT:
            host->queue_maxcount[index] = value;
            break;

        case QUEUE_MAXRUNNINGJOBS:
            host->queue_maxrunningjobs[index] = value;
            break;

        case QUEUE_MAXJOBSINQUEUE:
            host->queue_maxjobsinqueue[index] = value;
            break;

        default:
            gw_log_print("IM",'E',"Invalid integer variable %s.\n",
                    gw_host_get_varname(var));
            break;
    }
}

/*----------------------------------------------------------------------------*/

void gw_host_set_var_str(gw_host_var_t var, int index, char *value, gw_host_t *host)
{
    if (index >= GW_HOST_MAX_QUEUES)
    {
        gw_log_print("IM",'E',"Max number of queues exceeded in variable %s.\n",
                gw_host_get_varname(var));
        return;
    }
    
    switch(var)
    {
        case HOSTNAME:
            if (strcmp(host->hostname, value) != 0)
            {
                gw_log_print("IM",'W',"Updating host \"%s\" with values of host \"%s\".\n.",
                        host->hostname, value);
            }
            
            free(value);
            break;

        case ARCH:
            free(host->arch);
            host->arch = value;
            break;

        case OS_NAME:
            free(host->os_name);
            host->os_name = value;
            break;

        case OS_VERSION:
            free(host->os_version);
            host->os_version = value;
            break;

        case CPU_MODEL:
            free(host->cpu_model);
            host->cpu_model = value;
            break;
            
        case FORK_NAME:
            free(host->fork_name);
            host->fork_name = value;
            break;
            
        case LRMS_NAME:
            free(host->lrms_name);
            host->lrms_name = value;
            break;
            
        case LRMS_TYPE:
            free(host->lrms_type);
            host->lrms_type = value;
            break;

        case QUEUE_NAME:
            free(host->queue_name[index]);
            host->queue_name[index] = value;
            break;

        case QUEUE_DISPATCHTYPE:
            free(host->queue_dispatchtype[index]);
            host->queue_dispatchtype[index] = value;
            break;

        case QUEUE_PRIORITY:
            free(host->queue_priority[index]);
            host->queue_priority[index] = value;
            break;

        case QUEUE_STATUS:
            free(host->queue_status[index]);
            host->queue_status[index] = value;
            break;

        default:
            gw_log_print("IM",'E',"Invalid string variable %s.\n",
                    gw_host_get_varname(var));
            break;
    }
}

/*----------------------------------------------------------------------------*/

void gw_host_set_genvar_int(char *var, int index, int value, gw_host_t *host)
{
    int p;
    char name[500];
    
    if (index == -1)
    {
        /* Scalar variable */
        strcpy(name, var);
    } 
    else if (index >= GW_HOST_MAX_QUEUES)
    {
        gw_log_print("IM",'E',"Max number of queues exceeded in variable %s\n",
                var);
        return;
    }
    else
    {
        /* Array variable */
        sprintf(name, "%s[%i]", var, index);
    }
    
    gw_log_print("IM",'W',"Setting up generic integer variable for host %i (%s = %i).\n",
                 host->host_id, 
                 name, 
                 value);

    p = gw_host_search_genvar_int(name, host);

    if (p < GW_HOST_MAX_GENVARS && host->genvar_int[p].name != NULL)
    {
        /* Found */
        host->genvar_int[p].value = value;
    }
    else
    {
        /* Not found */
        host->genvar_int[p].name = strdup(name);
        host->genvar_int[p].value = value;
    }
}

/*----------------------------------------------------------------------------*/

void gw_host_set_genvar_str(char *var, int index, char *value, gw_host_t *host)
{
    int p;
    char name[500];
    
    if (index == -1)
    {
        /* Scalar variable */
        strcpy(name, var);
    }
    else if (index >= GW_HOST_MAX_QUEUES)
    {
        gw_log_print("IM",'E',"Max number of queues exceeded in variable %s\n",
                var);
        return;
    }
    else
    {
        /* Array variable */
        sprintf(name, "%s[%i]", var, index);
    }

    gw_log_print("IM",'W',"Setting up generic string variable for host %i (%s = \"%s\").\n",
                 host->host_id, 
                 name, 
                 value);
    
    p = gw_host_search_genvar_str(name, host);

    if (p < GW_HOST_MAX_GENVARS && host->genvar_str[p].name != NULL)
    {
        /* Found */
        free(host->genvar_str[p].value);
        host->genvar_str[p].value = strdup(value);
    }
    else
    {
        /* Not found */
        host->genvar_str[p].name = strdup(name);
        host->genvar_str[p].value = strdup(value);
    }
}

/*----------------------------------------------------------------------------*/

int gw_host_get_var_int(gw_host_var_t var, int index, gw_host_t *host)
{

    if (index >= GW_HOST_MAX_QUEUES)
    {
        gw_log_print("IM",'E',"Max number of queues exceeded in variable %s\n",
                gw_host_get_varname(var));
                
        return 0;
    }

    switch(var)
    {
        case CPU_MHZ:
            return host->cpu_mhz;

        case CPU_FREE:
            return host->cpu_free;
            
        case CPU_SMP:
            return host->cpu_smp;

        case NODECOUNT:
            return host->nodecount;
            
        case SIZE_MEM_MB:
            return host->size_mem_mb;

        case FREE_MEM_MB:
            return host->free_mem_mb;

        case SIZE_DISK_MB:
            return host->size_disk_mb;

        case FREE_DISK_MB:
            return host->free_disk_mb;
            
        case QUEUE_NODECOUNT:
            return host->queue_nodecount[index];
            
        case QUEUE_FREENODECOUNT:
            return host->queue_freenodecount[index];

        case QUEUE_MAXTIME:
            return host->queue_maxtime[index];

        case QUEUE_MAXCPUTIME:
            return host->queue_maxcputime[index];

        case QUEUE_MAXCOUNT:
            return host->queue_maxcount[index];

        case QUEUE_MAXRUNNINGJOBS:
            return host->queue_maxrunningjobs[index];

        case QUEUE_MAXJOBSINQUEUE:
            return host->queue_maxjobsinqueue[index];

        default:
            gw_log_print("IM",'E',"Invalid integer variable %s.\n",
                    gw_host_get_varname(var));
            return 0;
    }
}

/*----------------------------------------------------------------------------*/

char *gw_host_get_var_str(gw_host_var_t var, int index, gw_host_t *host)
{

    if (index >= GW_HOST_MAX_QUEUES)
    {
        gw_log_print("IM",'E',"Max number of queues exceeded in variable %s\n",
                gw_host_get_varname(var));
        return NULL;
    }

    switch(var)
    {
        case HOSTNAME:
            return host->hostname;

        case ARCH:
            return host->arch;

        case OS_NAME:
            return host->os_name;

        case OS_VERSION:
            return host->os_version;

        case CPU_MODEL:
            return host->cpu_model;
            
        case FORK_NAME:
            return host->fork_name;

        case LRMS_NAME:
            return host->lrms_name;

        case LRMS_TYPE:
            return host->lrms_type;

        case QUEUE_NAME:
            return host->queue_name[index];

        case QUEUE_DISPATCHTYPE:
            return host->queue_dispatchtype[index];

        case QUEUE_PRIORITY:
            return host->queue_priority[index];

        case QUEUE_STATUS:
            return host->queue_status[index];
            
        default:
            gw_log_print("IM",'E',"Invalid string variable %s.\n",
                    gw_host_get_varname(var));
            return 0;
    }
}
        
/*----------------------------------------------------------------------------*/

int gw_host_get_genvar_int(char *var, int index, gw_host_t *host)
{
    int  p;
    int  result;
    char name[500];
    
    if (index >= GW_HOST_MAX_QUEUES)
    {
        gw_log_print("IM",'E',"Max number of queues exceeded in variable %s.\n",
                var);
        return 0;
    }

    /* Scalar variable */
    strcpy(name, var);

#ifdef GWIMDEBUG
    gw_log_print("IM",'W',"Getting generic integer variable (%s) for host %i.\n",
                  name,
                  host->host_id);
#endif
    
    p = gw_host_search_genvar_int(name, host);

    if (p < GW_HOST_MAX_GENVARS && host->genvar_int[p].name != NULL) /* Found */
    {   
        result = host->genvar_int[p].value;
    }
    else /* Not found */
    {
        /* Array variable */
        sprintf(name, "%s[%i]", var, index);
        
        p = gw_host_search_genvar_int(name, host);

        if (p < GW_HOST_MAX_GENVARS && host->genvar_int[p].name != NULL) /* Found */
        {
            result = host->genvar_int[p].value;
        }
        else /* Not found */
        {
            result = 0;
        }
    }

#ifdef GWIMDEBUG
    gw_log_print("IM",'I',"Generic integer variable for host %i (%s == %i).\n",
            host->host_id, name, result);
#endif
            
    return result;
}

/*----------------------------------------------------------------------------*/

char *gw_host_get_genvar_str(char *var, int index, gw_host_t *host)
{
    int  p;
    char *result;
    char name[500];

    if (index >= GW_HOST_MAX_QUEUES)
    {
        gw_log_print("IM",'E',"Max number of queues exceeded in variable %s.\n",
                     var);
        return NULL;
    }

    /* Scalar variable */
    strcpy(name, var);

#ifdef GWIMDEBUG
    gw_log_print("IM",'W',"Getting generic string variable (%s) for host %i.\n",
                 name,
                 host->host_id);                  
#endif

    p = gw_host_search_genvar_str(name, host);
    
    if (p < GW_HOST_MAX_GENVARS && host->genvar_str[p].name != NULL) /* Found */
    {
        result = host->genvar_str[p].value;
    }
    else /* Not found */
    {
        /* Array variable */
        sprintf(name, "%s[%i]", var, index);
        
#ifdef GWIMDEBUG
        gw_log_print("IM",'W',"Getting generic string variable (%s) for host %i.\n",
                     name,
                     host->host_id);
#endif

        p = gw_host_search_genvar_str(name, host);

        if (p < GW_HOST_MAX_GENVARS && host->genvar_str[p].name != NULL) /* Found */
        {
            result = host->genvar_str[p].value;
        }
        else /* Not found */
        {
            result = NULL;
        }
    }
    
#ifdef GWIMDEBUG
    gw_log_print("IM",'I',"Generic string variable for host %i (%s == \"%s\").\n",    
                 host->host_id, 
                 name, 
                 result);
#endif

    return result;
}

/*----------------------------------------------------------------------------*/

int gw_host_search_genvar_int(char *name, gw_host_t *host)
{
    int i;
    
    for (i = 0; i < GW_HOST_MAX_GENVARS; i++)
    {
        if (host->genvar_int[i].name == NULL
                || strcmp(host->genvar_int[i].name, name) == 0)
        {
            break;
        }
    }
    
    return i;
}

/*----------------------------------------------------------------------------*/

int gw_host_search_genvar_str(char *name, gw_host_t *host)
{
    int i;
    
    for (i = 0; i < GW_HOST_MAX_GENVARS; i++)
    {
        if (host->genvar_str[i].name == NULL
                || strcmp(host->genvar_str[i].name, name) == 0)
        {
            break;
        }
    }

    return i;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
