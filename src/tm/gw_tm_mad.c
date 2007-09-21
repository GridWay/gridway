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
#include <limits.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>

#include "gw_tm_mad.h"
#include "gw_tm.h"
#include "gw_log.h"

static int gw_tm_mad_start_mad(gw_tm_mad_t * tm_mad);

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int gw_tm_mad_init(gw_tm_mad_t * tm_mad, 
                   const char *  exe, 
                   const char *  name,
		           const char *  args,
		           const char *  owner)
{
    int rc,length;
	
	if ((name == NULL) || (exe == NULL) || (owner == NULL))
		return -1;
		
	length = strlen(gw_conf.gw_location) + strlen(exe) + 6;
	
    tm_mad->name         = strdup(name);
    tm_mad->owner         = strdup(owner);
    tm_mad->executable   = (char *) malloc(sizeof(char)*length);
    
    sprintf(tm_mad->executable,"%s/bin/%s",gw_conf.gw_location,exe);
    
    if (args != NULL)
		tm_mad->argument = strdup(args);
	else
		tm_mad->argument = NULL;
        
    tm_mad->url          = NULL;
    
    rc = gw_tm_mad_start_mad(tm_mad);
    
    if ( rc == -1 )
        gw_tm_mad_finalize (tm_mad);
        
    return rc;
    
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_tm_mad_mkdir(gw_tm_mad_t *tm_mad, int xfr_id, const char *dir)
{
    char buf[GW_TM_MAX_STRING];

    sprintf(buf, "MKDIR %d - - %s - \n", xfr_id, dir);
    write(tm_mad->tm_mad_pipe, buf, strlen(buf));
    
    return;    

}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_tm_mad_rmdir(gw_tm_mad_t *tm_mad, int xfr_id, const char *dir)
{
    char buf[GW_TM_MAX_STRING];

    sprintf(buf, "RMDIR %d - - %s - \n",xfr_id,dir);
    
    write(tm_mad->tm_mad_pipe, buf, strlen(buf));
    
    return;    
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_tm_mad_cp(gw_tm_mad_t * tm_mad, 
                  int           xfr_id, 
                  int           cp_xfr_id,
                  char          modex, 
                  const char *  src, 
                  const char *  dst)
{
    char buf[GW_TM_MAX_STRING];

    sprintf(buf, "CP %d %d %c %s %s\n",xfr_id,cp_xfr_id,modex,src,dst);

    write(tm_mad->tm_mad_pipe, buf, strlen(buf));
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_tm_mad_start(gw_tm_mad_t *tm_mad, int xfr_id)
{
    char buf[80];

    sprintf(buf, "START %d - - - - \n", xfr_id);
    
    write(tm_mad->tm_mad_pipe, buf, strlen(buf));
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_tm_mad_end(gw_tm_mad_t *tm_mad, int xfr_id)
{
    char buf[80];

    sprintf(buf, "END %d - - - - \n", xfr_id);
    
    write(tm_mad->tm_mad_pipe, buf, strlen(buf));
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_tm_mad_exists(gw_tm_mad_t *tm_mad, int xfr_id, const char *dir)
{
    char buf[GW_TM_MAX_STRING];

    sprintf(buf, "EXISTS %d - - %s - \n",xfr_id,dir);
    
    write(tm_mad->tm_mad_pipe, buf, strlen(buf));
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_tm_mad_finalize (gw_tm_mad_t *tm_mad)
{
    char buf[50];
	int  status;
	pid_t pid;
	
    strcpy(buf, "FINALIZE - - - - -\n");
    write(tm_mad->tm_mad_pipe, buf, strlen(buf));

    close(tm_mad->tm_mad_pipe);
    close(tm_mad->mad_tm_pipe);

    pid = waitpid(tm_mad->pid, &status, WNOHANG);
    
    if ( pid == 0 )
    {
#ifdef GWTMDEBUG
    	gw_log_print("USER",'I',"Waiting for transfer MAD %s (pid %i) to finalize.\n"
    	             ,tm_mad->name, tm_mad->pid);
#endif    	             
    	sleep(1);
    	waitpid(tm_mad->pid, &status, WNOHANG);
    }	    
    
    if ( tm_mad->name != NULL )
	    free(tm_mad->name);
	    
    if ( tm_mad->executable != NULL )
	    free(tm_mad->executable);
	    
    if ( tm_mad->argument != NULL )    
	    free(tm_mad->argument);    
	    
    if ( tm_mad->url != NULL )    
        free(tm_mad->url);
        
    if ( tm_mad->owner != NULL )    
        free(tm_mad->owner);
                    
    return;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int gw_tm_mad_reload (gw_tm_mad_t *tm_mad)
{
    char buf[50];
    int  status;
    pid_t pid, rc;
    
    strcpy(buf, "FINALIZE - - - - -\n");
    write(tm_mad->tm_mad_pipe, buf, strlen(buf));

    close(tm_mad->tm_mad_pipe);
    close(tm_mad->mad_tm_pipe);

    pid = waitpid(tm_mad->pid, &status, WNOHANG);
    
    if ( pid == 0 )
    {
#ifdef GWTMDEBUG
        gw_log_print("USER",'I',"Waiting for transfer MAD %s (pid %i) to finalize.\n"
                     ,tm_mad->name, tm_mad->pid);
#endif                   
        sleep(1);
        waitpid(tm_mad->pid, &status, WNOHANG);
    }       
    
    rc = gw_tm_mad_start_mad(tm_mad);
    
    return rc;
}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

static int gw_tm_mad_start_mad(gw_tm_mad_t * tm_mad)
{
    char buf[50];
    char str[GW_TM_MAX_STRING], c;
    char info[GW_TM_MAX_INFO];
    char s_job_id[GW_TM_MAX_JOB_ID];
    char s_cp_xfr_id[5];
    char result[GW_TM_MAX_RESULT];
    char action[GW_TM_MAX_ACTION];
    
    int tm_mad_pipe[2], mad_tm_pipe[2];
    int i, rc;
    
    if ( pipe(tm_mad_pipe) == -1 ||  pipe(mad_tm_pipe) == -1)
    {
        gw_log_print("TM",'E',"Could not create communication pipes: %s.\n",
                strerror(errno));
        return -1;
    }

    tm_mad->pid = fork();

    switch (tm_mad->pid){
        case -1: /* Error */
            gw_log_print("TM",'E',"Could not fork to start transfer MAD %s.\n",tm_mad->name);
            return -1;

        case 0: /* Child process (MAD) */
            close(tm_mad_pipe[1]);
            close(mad_tm_pipe[0]);
            
            /* stdin and stdout redirection */
            if ( dup2(tm_mad_pipe[0], 0) != 0 || dup2(mad_tm_pipe[1], 1) != 1)
            {
                gw_log_print("TM",'E',"Could not duplicate communication pipes: %s\n",
                             strerror(errno));                
                exit(-1);
            }
            
            close(tm_mad_pipe[0]);
            close(mad_tm_pipe[1]);
                  
                        
            if (gw_conf.multiuser == GW_TRUE)            
                execlp("sudo","sudo","-H", "-u",tm_mad->owner,tm_mad->executable,tm_mad->argument,NULL);
            else
                execlp(tm_mad->executable,tm_mad->executable,tm_mad->argument,NULL);
                
            /* exec should not return */
            gw_log_print("TM",'E',"Could not execute MAD %s (exec/sudo), exiting...\n",tm_mad->executable);
            
            exit(-1);

            break;

        default: /* Parent process (GWD) */
            close(tm_mad_pipe[0]);
            close(mad_tm_pipe[1]);

            tm_mad->tm_mad_pipe = tm_mad_pipe[1];
            tm_mad->mad_tm_pipe = mad_tm_pipe[0];

            fcntl(tm_mad->tm_mad_pipe, F_SETFD, FD_CLOEXEC); /* Close pipes in other MADs*/
            fcntl(tm_mad->mad_tm_pipe, F_SETFD, FD_CLOEXEC);
            
            sprintf(buf, "INIT %i - - - -\n",gw_conf.number_of_jobs);
            write(tm_mad->tm_mad_pipe, buf, strlen(buf));

            i = 0;
            
            do
            {
                rc = read(tm_mad->mad_tm_pipe, (void *) &c, sizeof(char));
                str[i++] = c;
            }
            while ( rc > 0 && c != '\n' &&  c != '\0');

            str[i] = '\0';
            
            if (rc <= 0)
            {
                gw_log_print("TM",'E',"\tInitialization failure, reading from MAD %s.\n",tm_mad->name);
                return -1;
            }

            sscanf(str,"%s %s %s %s %[^\n]", action, s_job_id, s_cp_xfr_id, result, info);
            
            if (strcmp(action, "INIT") == 0)
            {
                if (strcmp(result, "SUCCESS") == 0)
                {
                    if (strcmp(info, "-") == 0)
                        tm_mad->url = NULL;
                    else
                        tm_mad->url = strdup(info);
                }
                else
                {
                    gw_log_print("TM",'E',"\tInitialization failure of MAD %s.\n", tm_mad->name);
                    return -1;            
                }
            }
            else
            {
                gw_log_print("TM",'E',"\tInitialization failure, bad response from MAD %s.\n", tm_mad->name);
                return -1;            
            }
                        
            break;               
    }

    return 0;    
}
