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

#include "gw_im_mad.h"
#include "gw_im.h"
#include "gw_log.h"

static int gw_im_mad_start(gw_im_mad_t *im_mad);

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int gw_im_mad_init(gw_im_mad_t *im_mad, const char *exe, const char *name, 
        const char *args, char *em_mad_name, char *tm_mad_name)
{
    int rc;

	if ((exe == NULL) || 
	    (name == NULL)|| 
	    (em_mad_name == NULL) || 
	    (tm_mad_name == NULL ))
		return -1;
		
    im_mad->name        = strdup(name);
    im_mad->executable  = strdup(exe);
    im_mad->em_mad_name = strdup(em_mad_name);
    im_mad->tm_mad_name = strdup(tm_mad_name);
    im_mad->state       = GW_IM_MAD_STATE_IDLE;

    if (args != NULL)
        im_mad->argument    = strdup(args);
    
    rc = gw_im_mad_start(im_mad);

    if (rc == -1)
        gw_im_mad_finalize (im_mad);  

    return rc;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_im_mad_discover(gw_im_mad_t *im_mad)
{
    char buf[GW_IM_MAX_STRING];
	int write_result;
    sprintf(buf, "DISCOVER - - -\n");
    write_result = write(im_mad->im_mad_pipe, buf, strlen(buf));

    return;    
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_im_mad_monitor(gw_im_mad_t *im_mad, int host_id, char *host)
{
    char buf[GW_IM_MAX_STRING];
	int write_result;
    sprintf(buf, "MONITOR %d %s -\n", host_id, host);
    write_result = write(im_mad->im_mad_pipe, buf, strlen(buf));
    
    return;    
}

int gw_im_mad_reload (gw_im_mad_t *im_mad)
{
    char buf[50];
    int  status;
    pid_t pid;
    int rc;
	int write_result;

    gw_log_print("DM",'I',"Reloading IM driver: %s (pid %i).\n"
                 ,im_mad->name, im_mad->pid);
                     
    strcpy(buf, "FINALIZE - - -\n");
    
    write_result = write(im_mad->im_mad_pipe, buf, strlen(buf));

    close(im_mad->im_mad_pipe);
    close(im_mad->mad_im_pipe);

    pid = waitpid(im_mad->pid, &status, WNOHANG);
    
    if ( pid == 0 )
    {
#ifdef GWIMDEBUG        
        gw_log_print("IM",'I',"Waiting for MAD %s (pid %i) to finalize.\n",
                     im_mad->name, 
                     im_mad->pid);
#endif                    
        sleep(1);
        
        waitpid(im_mad->pid, &status, WNOHANG);
    }       
    
    rc = gw_im_mad_start(im_mad);
    
    return rc;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_im_mad_finalize (gw_im_mad_t *im_mad)
{
    char buf[50];
	int  status;
	pid_t pid;
	int write_result;
    strcpy(buf, "FINALIZE - - -\n");
    
    write_result = write(im_mad->im_mad_pipe, buf, strlen(buf));

    close(im_mad->im_mad_pipe);
    close(im_mad->mad_im_pipe);

    pid = waitpid(im_mad->pid, &status, WNOHANG);
    
    if ( pid == 0 )
    {
#ifdef GWIMDEBUG       	
    	gw_log_print("IM",'I',"Waiting for MAD %s (pid %i) to finalize.\n",
    	             im_mad->name, 
    	             im_mad->pid);
#endif    	              
    	sleep(1);
    	
    	waitpid(im_mad->pid, &status, WNOHANG);
    }	    
    
    free(im_mad->name);
    free(im_mad->executable);
    free(im_mad->em_mad_name);
    free(im_mad->tm_mad_name);
    
    if (im_mad->argument != NULL)
        free(im_mad->argument);    

    return;
}


static int gw_im_mad_start(gw_im_mad_t *im_mad)
{

    char buf[50];
    char str[GW_IM_MAX_STRING], c;
    char info[GW_IM_MAX_INFO];
    char s_host_id[GW_IM_MAX_HOST_ID];
    char result[GW_IM_MAX_RESULT];
    char action[GW_IM_MAX_ACTION];
    int write_result;
    int im_mad_pipe[2], mad_im_pipe[2];
    int i, rc;

    if ( pipe(im_mad_pipe) == -1 ||  pipe(mad_im_pipe) == -1 )
        return -1;

    im_mad->pid = fork();

    switch (im_mad->pid){
        case -1: /* Error */
            return -1;
            break;

        case 0: /* Child process (MAD) */
            close(im_mad_pipe[1]);
            close(mad_im_pipe[0]);
            
            /* stdin and stdout redirection */
            if ( dup2(im_mad_pipe[0], 0) != 0 || dup2(mad_im_pipe[1], 1) != 1)
                exit(-1);         
            
            close(im_mad_pipe[0]);
            close(mad_im_pipe[1]);
           
            execlp(im_mad->executable, im_mad->executable, im_mad->argument, NULL);
        
            /* exec should not return */
            gw_log_print("IM",'E',"Could not execute MAD %s, exiting...\n", im_mad->executable);
            
            exit(-1);
            
            break;

        default: /* Parent process (IM) */
            close(im_mad_pipe[0]);
            close(mad_im_pipe[1]);

            im_mad->im_mad_pipe = im_mad_pipe[1];
            im_mad->mad_im_pipe = mad_im_pipe[0];
            
            fcntl(im_mad->im_mad_pipe, F_SETFD, FD_CLOEXEC);
            fcntl(im_mad->mad_im_pipe, F_SETFD, FD_CLOEXEC);
            
            strcpy(buf, "INIT - - -\n");
            write_result = write(im_mad->im_mad_pipe, buf, strlen(buf));

            i = 0;
            
            do
            {
                rc = read(im_mad->mad_im_pipe, (void *) &c, sizeof(char));
                str[i++] = c;
            }
            while ( rc > 0 && c != '\n' &&  c != '\0');

            str[i] = '\0';
            
            if (rc <= 0)
                return -1;

            sscanf(str,"%s %s %s %[^\n]", action, s_host_id, result, info);

            if (strcmp(action, "INIT") == 0)
            {
                if (strcmp(result, "FAILURE") == 0)
                    return -1;
            }
            else
                return -1;
 
            break; 
    }
    
    return 0;
}

