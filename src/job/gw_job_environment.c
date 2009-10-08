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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <pwd.h>
#include <time.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>

#include "gw_job.h"
#include "gw_dm.h"
#include "gw_em.h"
#include "gw_em_mad.h"
#include "gw_common.h"
#include "gw_log.h"
#include "gw_user_pool.h"
#include "gw_host_pool.h"


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

static char *gw_job_template_files2str(gw_job_t *job, int ior);

static char *gw_job_template_files2str_extensions(gw_job_t *job, int ior);

static char *gw_job_template_sfiles2str(gw_job_t *job, int ior);
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

char * gw_job_rsl2_extensions(gw_job_t *job)
{
    char rsl_buffer[4096];
    char tmp_buffer[1024];
    char * file_str;
    char * var;
    char * extensions;
    
    snprintf(rsl_buffer, sizeof(char) * 4096,
                "<extensions>\n"
                "<gw>\n"
                "CPULOAD_THRESHOLD=%i\n"
                "SUSPENSION_TIMEOUT=%i\n",
                job->template.cpuload_threshold,
                (int) job->template.suspension_timeout);    

    if ( job->template.num_input_files != 0 )
    {
        file_str = gw_job_template_files2str_extensions(job, 0);
        
        if ( file_str != NULL )
        {
               var = gw_job_substitute (file_str, job);
               
               if (var != NULL)
            {
                snprintf(tmp_buffer,sizeof(char) * 1024,"INPUT_FILES=%s\n",var);
                strncat(rsl_buffer,tmp_buffer,sizeof(char) * (4096 - 1));            
                free(var);
            }
            
            free(file_str);
        }
    }

    if ( job->template.num_output_files != 0 )
    {
        file_str = gw_job_template_files2str_extensions(job,1);
        
        if ( file_str != NULL )
        {
            var = gw_job_substitute (file_str, job);
               
            if (var != NULL)
            {
                snprintf(tmp_buffer,sizeof(char) * 1024,"OUTPUT_FILES=%s\n",var);
                strncat(rsl_buffer,tmp_buffer,sizeof(char) * (4096 - 1));            
                
                free(var);
            }
            
            free(file_str);
        }
    }

    if ( job->template.num_restart_files != 0 )
    {
        file_str = gw_job_template_sfiles2str(job,2);        


        if ( file_str != NULL )
        {
               var = gw_job_substitute (file_str, job);
               
               if (var != NULL)
            {
                snprintf(tmp_buffer,sizeof(char) * 1024,"RESTART_FILES=%s\n",var);
                strncat(rsl_buffer,tmp_buffer,sizeof(char) * (4096 - 1));            
                
                free(var);
            }
            
            free(file_str);
        }        
    }

    if ( job->template.requirements != NULL )
    {
        snprintf(tmp_buffer,sizeof(char) * 1024,"REQUIREMENTS=%s\n",job->template.requirements);
        strncat(rsl_buffer,tmp_buffer,sizeof(char) * (4096 - 1));        
    }

    if ( job->template.rank != NULL )
    {
        snprintf(tmp_buffer,sizeof(char) * 1024,"RANK=%s\n",job->template.rank);
        strncat(rsl_buffer,tmp_buffer,sizeof(char) * (4096 - 1));        
    }

    snprintf(tmp_buffer,sizeof(char) * 1024,"</gw>\n</extensions>\n");
    strncat(rsl_buffer,tmp_buffer,sizeof(char) * (4096 - 1));        
    
    extensions = strdup(rsl_buffer);
                      
  return extensions;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


int gw_job_environment(gw_job_t *job)
{
    FILE * fd;
    char * file_str;
    char * var;
    int    i;
    
    fd = fopen(job->env_file,"w");
  
    if (fd == NULL)
    {
          gw_log_print("DM",'E',"Could not create job.env file of job %i: %s",
                  job->id,
                strerror(errno));
                
          gw_job_print(job,"DM",'E',"Could not create job.env file: %s",
                strerror(errno));
          return -1;
    }

    fprintf(fd,"export GW_CPULOAD_THRESHOLD=\"%i\"\n",
            job->template.cpuload_threshold);
    fprintf(fd,"export GW_OS_NAME=\"%s\"\n", job->history->host->os_name);  
    fprintf(fd,"export GW_OS_VERSION=\"%s\"\n", job->history->host->os_version);
    fprintf(fd,"export GW_CPU_MODEL=\"%s\"\n", job->history->host->cpu_model);
    fprintf(fd,"export GW_CPU_MHZ=\"%i\"\n", job->history->host->cpu_mhz);
    fprintf(fd,"export GW_MEM_MB=\"%i\"\n", job->history->host->free_mem_mb);
    fprintf(fd,"export GW_DISK_MB=\"%i\"\n", job->history->host->free_disk_mb);
    
    if (job->array_id != -1)
    {
        fprintf(fd,"export GW_PARAM=\"%i\"\n",job->pstart + (job->pinc*job->task_id));
        fprintf(fd,"export GW_MAX_PARAM=\"%i\"\n",job->pstart + (job->pinc*(job->total_tasks-1)));        
    }
    else
    {
        fprintf(fd,"export GW_PARAM=\"%i\"\n",0);
        fprintf(fd,"export GW_MAX_PARAM=\"%i\"\n",0);
    }
                
    if ( job->history->host->arch != NULL )
    {        
        fprintf(fd,"export GW_ARCH=\"%s\"\n",job->history->host->arch);
    }


    if ( job->template.executable != NULL )
    {
        var = gw_job_substitute (job->template.executable, job);        
        
        if ( var != NULL)
        {        
            fprintf(fd,"export GW_EXECUTABLE=\"%s\"\n",var);
            free(var);
        }
    }
    
    if ( job->template.monitor != NULL )
    {
        var = gw_job_substitute (job->template.monitor, job);        
        
        if ( var != NULL)
        {        
            fprintf(fd,"export GW_MONITOR=\"%s\"\n",var);
            free(var);
        }
    }
        

    if ( job->template.arguments != NULL )
    {
        var = gw_job_substitute (job->template.arguments, job);        
        
        if ( var != NULL)
        {
            fprintf(fd,"export GW_ARGUMENTS=\"%s\"\n",var);
            free(var);
        }
    }

    if ( job->template.stdin_file != NULL )
    {
        var = gw_job_substitute (job->template.stdin_file, job);

        if ( var != NULL)
        {
            fprintf(fd,"export GW_STDIN=\"%s\"\n",var);
            free(var);
        }
    }

    if ( job->template.stdout_file != NULL )
    {
        var = gw_job_substitute (job->template.stdout_file, job);

        if ( var != NULL)
        {
            fprintf(fd,"export GW_STDOUT=\"%s\"\n",var);
            free(var);
        }
    }

    if ( job->template.stderr_file != NULL )
    {
        var = gw_job_substitute (job->template.stderr_file, job);

        if ( var != NULL)
        {
            fprintf(fd,"export GW_STDERR=\"%s\"\n",var);
            free(var);
        }
    }

    if (job->history->tm_mad->url != NULL)
    {
        fprintf(fd,"export GW_STAGING_URL=\"%s\"\n",job->history->tm_mad->url);
    }
    else
    {
        fprintf(fd,"export GW_STAGING_URL=\"%s\"\n",job->history->host->hostname);
    }
    
    fprintf(fd,"export GW_JOB_HOME=\"%s\"\n",job->template.job_home);

    if ( job->template.num_input_files != 0 )
    {
        file_str = gw_job_template_files2str(job, 0);
        
        if ( file_str != NULL )
        {
               var = gw_job_substitute (file_str, job);
               
               if (var != NULL)
            {
                fprintf(fd,"export GW_INPUT_FILES=\"%s\"\n",GWNSTR(var));
                free(var);
            }
            
            free(file_str);
        }
    }

    if ( job->template.num_output_files != 0 )
    {
        file_str = gw_job_template_files2str(job,1);
        
        if ( file_str != NULL )
        {
               var = gw_job_substitute (file_str, job);
               
               if (var != NULL)
            {
                fprintf(fd,"export GW_OUTPUT_FILES=\"%s\"\n",GWNSTR(var));
                free(var);
            }
            
            free(file_str);
        }
    }

    if ( job->template.num_restart_files != 0 )
    {
        file_str = gw_job_template_sfiles2str(job,2);        


        if ( file_str != NULL )
        {
               var = gw_job_substitute (file_str, job);
               
               if (var != NULL)
            {
                fprintf(fd,"export GW_RESTART_FILES=\"%s\"\n",GWNSTR(var));
                free(var);
            }
            
            free(file_str);
        }        
    }

    if ( job->template.num_env != 0 )
    {
        for (i=0;i<job->template.num_env;i++)
        {
               var = gw_job_substitute (job->template.environment[i][GW_ENV_VAL], job);        
            
            if (var != NULL)
            {
                   fprintf(fd,"export %s=\"%s\"\n",
                    job->template.environment[i][GW_ENV_VAR], var); 
                free(var);                
            }
        }
    }
              
  fclose(fd);
                      
  return 0;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

char *gw_job_rsl_environment(gw_job_t *job)
{
    char rsl_buffer[512];
    char *rsl_env; 
    int  rc;    
    
    if (job->history == NULL)
    {
        return NULL;
    }
    
    if (job->history->host == NULL)
    {
        return NULL;
    }
            
    rc = snprintf(rsl_buffer, 512,
                "(GW_HOSTNAME \"%s\")"    
                "(GW_USER \"%s\")"
                "(GW_JOB_ID %i)"
                "(GW_TASK_ID %i)"
                "(GW_ARRAY_ID %i)"
                "(GW_TOTAL_TASKS %i)"
                "(GW_RESTARTED %i)",
                job->history->host->hostname,        
                job->owner,
                job->id,
                job->task_id,
                job->array_id,
                job->total_tasks,
                job->restarted);
        
    if ((rc >= 512 ) || ( rc < 0 ) )
        return NULL;
                
    rsl_env = strdup(rsl_buffer);
    
    return rsl_env;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

char *gw_job_rsl2_environment(gw_job_t *job)
{
    char rsl_buffer[640];
    char *rsl_env; 
    int  rc;
    
    rc = snprintf(rsl_buffer, 640,
            "<environment>"
            " <name>GW_HOSTNAME</name> <value>%s</value>"
            "</environment>"
            "<environment>"
            " <name>GW_USER</name> <value>%s</value>"
            "</environment>"
            "<environment>"
            " <name>GW_JOB_ID</name> <value>%i</value>"
            "</environment>"
            "<environment>"
            " <name>GW_TASK_ID</name> <value>%i</value>"
            "</environment>"
            "<environment>"
            " <name>GW_ARRAY_ID</name> <value>%i</value>"
            "</environment>"
            "<environment>"
            " <name>GW_TOTAL_TASKS</name> <value>%i</value>"
            "</environment>"
            "<environment>"
            " <name>GW_RESTARTED</name> <value>%i</value>"
            "</environment>",
            job->history->host->hostname,
            job->owner,
            job->id,
            job->task_id,
            job->array_id,
            job->total_tasks,
            job->restarted);

    if ((rc >= 640 ) || ( rc < 0 ) )
        return NULL;
        
    rsl_env = strdup(rsl_buffer);
    
    return rsl_env;      
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

char *gw_job_template_files2str_extensions(gw_job_t *job, int ior)
{
    char *files_str;
    char ****files;
    int  *num;
    int  length;
    int  i;
    
    switch (ior)
    {
        case 0: /* Only consider GW_REMOTE_FILE for the GateWay */
            files = &(job->template.input_files);
            num = &(job->template.num_input_files);
            
            if ( (*num) == 0 )
                return NULL;
            
            length = 0;
    
            for ( i = 0; i<(*num); i++)
            {
                if ( (*files)[i][GW_REMOTE_FILE] != NULL )
                {
                    length = length + strlen((*files)[i][GW_REMOTE_FILE]);
                    length = length + 2;
                }
                else if ( (*files)[i][GW_LOCAL_FILE] != NULL )
                {
                    length = length + strlen((*files)[i][GW_LOCAL_FILE]);
                    length = length + 1;
                }
            }
                
            files_str = (char *) malloc(sizeof(char)*(length + 1));

            if (files_str == NULL )
                return NULL;

            *files_str='\0';
        
            for ( i = 0; i<((*num)-1); i++)
            {
                if ( (*files)[i][GW_REMOTE_FILE] != NULL )
                {
                    sprintf(files_str,"%s%s,",
                        files_str,
                        (*files)[i][GW_REMOTE_FILE]);
                }
                else if ( (*files)[i][GW_LOCAL_FILE] != NULL )
                {
                    sprintf(files_str,"%s%s,",
                        files_str,
                        (*files)[i][GW_LOCAL_FILE]);
                }                
            }

            if ( (*files)[i][GW_REMOTE_FILE] != NULL )
            {
                sprintf(files_str,"%s%s",
                    files_str,
                    (*files)[i][GW_REMOTE_FILE]);
            }
            else if ( (*files)[i][GW_LOCAL_FILE] != NULL )
            {
                sprintf(files_str,"%s%s",
                    files_str,
                    (*files)[i][GW_LOCAL_FILE]);            
            }
            
            break;
            
        case 1: /* Only consider GW_LOCAL_FILE for the GateWay */
            files = &(job->template.output_files);
            num   = &(job->template.num_output_files);
            
            if ( (*num) == 0 )
                return NULL;
            
            length = 0;
    
            for ( i = 0; i<(*num); i++)
            {
                if ( (*files)[i][GW_LOCAL_FILE] != NULL )
                {
                    length = length + strlen((*files)[i][GW_LOCAL_FILE]);
                    length = length + 1;
                }
            }
                
            files_str = (char *) malloc(sizeof(char)*(length + 1));

            if (files_str == NULL )
                return NULL;

            *files_str='\0';
        
            for ( i = 0; i<((*num)-1); i++)
            {
                if ( (*files)[i][GW_LOCAL_FILE] != NULL )
                {
                    sprintf(files_str,"%s%s,",
                        files_str,
                        (*files)[i][GW_LOCAL_FILE]);
                }                
            }

            if ( (*files)[i][GW_LOCAL_FILE] != NULL )
                sprintf(files_str,"%s%s",
                    files_str,
                    (*files)[i][GW_LOCAL_FILE]);            
            break;
            
        default:
            return NULL;
    }
    
    return files_str;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

char *gw_job_template_files2str(gw_job_t *job, int ior)
{
    char *files_str;
    char *buffer;
    
    char ****files;
    
    int  *num;
    int  length;
    int  i;
    
    switch (ior)
    {
        case 0:
            files = &(job->template.input_files);
            num = &(job->template.num_input_files);
            break;
            
        case 1:
            files = &(job->template.output_files);
            num = &(job->template.num_output_files);
            break;

        default:
            return NULL;            
    }
    
    if ( (*num) == 0 )
        return NULL;
        
    length = 0;
    
    for ( i = 0; i<(*num); i++)
    {
        if ( (*files)[i][GW_LOCAL_FILE] != NULL )
        {
            length = length + strlen((*files)[i][GW_LOCAL_FILE]);
            length = length + 1;
        }
            
        if ( (*files)[i][GW_REMOTE_FILE] != NULL )
        {
            length = length + strlen((*files)[i][GW_REMOTE_FILE]);
            length = length + 2;
        }
    }
        
    files_str = (char *) malloc(sizeof(char)*(length + 1));
    buffer    = (char *) malloc(sizeof(char)*(length + 1));
    
    if (files_str == NULL)
    {
        if (buffer != NULL)
            free(buffer);
            
        return NULL;
    }
    else if (buffer == NULL) 
    {
        free(files_str);
        
        return NULL;
    }
    
    
    *files_str='\0';
        
    for ( i = 0; i<((*num)-1); i++)
    {
        if ( (*files)[i][GW_REMOTE_FILE] != NULL )
        {
            sprintf(buffer,"%s %s,",
                    (*files)[i][GW_LOCAL_FILE], 
                    (*files)[i][GW_REMOTE_FILE]);
            
            strcat(files_str,buffer);
        }
        else
        {
            sprintf(buffer,"%s,",(*files)[i][GW_LOCAL_FILE]);
            
            strcat(files_str,buffer);            
        }
    }
                
    if ( (*files)[i][GW_REMOTE_FILE] != NULL )
    {
           sprintf(buffer,"%s %s",
                   (*files)[i][GW_LOCAL_FILE], 
                   (*files)[i][GW_REMOTE_FILE]);
            
           strcat(files_str,buffer);        
    }
    else
    {
           strcat(files_str,(*files)[i][GW_LOCAL_FILE]);
    }
    
    free(buffer);
    
    return files_str;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

char *gw_job_template_sfiles2str(gw_job_t *job, int ior)
{
    char *files_str;
    char *buffer;
    
    char ***files;
    int  *num;
    
    int  length;
    int  i;
    
    switch (ior)
    {
        case 2:
            files = &(job->template.restart_files);
            num = &(job->template.num_restart_files);
            break;            
        
        default:
            return NULL;
    }
    
    if ( (*num) == 0 )
        return NULL;
        
    length = 0;
    
    for ( i = 0; i<(*num); i++)
    {
        if ( (*files)[i] != NULL )
        {
            length = length + strlen((*files)[i]);
            length = length + 1;
        }            
    }
        
    files_str = (char *) malloc(sizeof(char)*(length + 1));
    buffer    = (char *) malloc(sizeof(char)*(length + 1));
    
    if (files_str == NULL)
    {
        if (buffer != NULL)
            free(buffer);
            
        return NULL;
    }
    else if (buffer == NULL) 
    {
        free(files_str);
        
        return NULL;
    }
    
    *files_str='\0';
        
    for ( i = 0; i<((*num)-1); i++)
    {
        sprintf(buffer,"%s,",(*files)[i]);
        strcat(files_str,buffer);
    }
                
    strcat(files_str,(*files)[i]);
    
    free(buffer);

    return files_str;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
