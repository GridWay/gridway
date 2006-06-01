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

#include "gw_em_mad.h"
#include "gw_em.h"
#include "gw_log.h"

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int gw_em_mad_init(gw_em_mad_t * em_mad, 
                   const char *   exe, 
                   const char *   name,
                   const char *   owner)
{
    char buf[50];
    char str[GW_EM_MAX_STRING], c;
    char info[GW_EM_MAX_INFO];
    char s_job_id[GW_EM_MAX_JOB_ID];
    char result[GW_EM_MAX_RESULT];
    char action[GW_EM_MAX_ACTION];
    int em_mad_pipe[2], mad_em_pipe[2];
    int i, rc;
	int length;
	
	if ((name == NULL) || (exe == NULL) || (owner == NULL))
		return -1;
	
	
	length = strlen(gw_conf.gw_location) + strlen(exe) + 6;

	em_mad->executable      = (char *) malloc(sizeof(char)*length);
    em_mad->name            = strdup(name);
    em_mad->wrapper_rsl     = NULL;
    em_mad->pre_wrapper_rsl = NULL;    
    em_mad->mode            = NULL;

	sprintf(em_mad->executable, "%s/bin/%s", gw_conf.gw_location, exe);    
    
    if (pipe(em_mad_pipe) == -1 || pipe(mad_em_pipe) == -1)
    {
        gw_log_print("EM",'E',"Could not create communication pipes: %s.\n",
                strerror(errno));
        return -1;
    }

    em_mad->pid = fork();
    
    switch (em_mad->pid)
    {
        case -1: /* Error */
            gw_log_print("EM",'E',"Could not fork to start execution MAD %s.\n", name);
            return -1;

        case 0: /* Child process (MAD) */
            close(em_mad_pipe[1]);
            close(mad_em_pipe[0]);
            
            /* stdin and stdout redirection */
            if (dup2(em_mad_pipe[0], 0) != 0 || dup2(mad_em_pipe[1], 1) != 1)
            {
                gw_log_print("EM",'E',"Could not duplicate communication pipes: %s\n",
                             strerror(errno));
                exit(-1);
            }
            
            close(em_mad_pipe[0]);
            close(mad_em_pipe[1]);
		    
			if (gw_conf.multiuser == GW_TRUE)
	            execlp("sudo", "sudo", "-u", owner, em_mad->executable, NULL);
	        else
	        	execlp(em_mad->executable, em_mad->executable, NULL);

            gw_log_print("EM",'E',"Could not execute MAD %s (exec/sudo).\n",
                        	em_mad->executable);
            exit(-1);

            break;

        default: /* Parent process (GWD) */

            close(em_mad_pipe[0]);
            close(mad_em_pipe[1]);

            em_mad->em_mad_pipe = em_mad_pipe[1];
            em_mad->mad_em_pipe = mad_em_pipe[0];

            fcntl(em_mad->em_mad_pipe, F_SETFD, FD_CLOEXEC); /* Close pipes in other MADs*/
            fcntl(em_mad->mad_em_pipe, F_SETFD, FD_CLOEXEC);
            
            sprintf(buf, "INIT %i - -\n",gw_conf.number_of_jobs);
            write(em_mad->em_mad_pipe, buf, strlen(buf));

            i = 0;

            do
            {
                rc = read(em_mad->mad_em_pipe, (void *) &c, sizeof(char));
                str[i++] = c;
            }
            while ( rc > 0 && c != '\n' &&  c != '\0');

            str[i] = '\0';

            if (rc <= 0)
            {
                gw_log_print("EM",'E',"\tInitialization failure, reading from MAD %s.\n", em_mad->name);
                gw_em_mad_finalize(em_mad);
                
               	return -1;
            }

            sscanf(str,"%s %s %s %[^\n]", action, s_job_id, result, info);

#ifdef GWEMDEBUG
             gw_log_print("EM",'D',"MAD message received:\"%s %s %s %s\".\n",
                          action, s_job_id, result, info);
#endif                        

            if (strcmp(action, "INIT") == 0)
            {
                if (strcmp(result, "FAILURE") == 0)
                {
                    gw_log_print("EM",'E',"\tInitialization failure of MAD %s.\n", em_mad->name);
    	            gw_em_mad_finalize(em_mad);
                    
                	return -1;
                }
            }
            else
            {
                gw_log_print("EM",'E',"\tInitialization failure, bad response from MAD %s.\n", em_mad->name);
                gw_em_mad_finalize(em_mad);

               	return -1;
            }
      
            break;
    }

    return 0;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_em_mad_submit(gw_em_mad_t *em_mad, int jid, char *rm_contact, char *rsl)
{
    char buf[GW_EM_MAX_STRING];

    sprintf(buf, "SUBMIT %d %s %s\n", jid, rm_contact, rsl);
    write(em_mad->em_mad_pipe, buf, strlen(buf));

}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_em_mad_recover(gw_em_mad_t *em_mad, int jid, char *job_contact)
{
    char buf[GW_EM_MAX_STRING];

    sprintf(buf, "RECOVER %d %s -\n", jid, job_contact);
    write(em_mad->em_mad_pipe, buf, strlen(buf));

}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_em_mad_cancel(gw_em_mad_t *em_mad, int jid)
{
    char buf[500];

    sprintf(buf, "CANCEL %d - -\n", jid);
    
    write(em_mad->em_mad_pipe, buf, strlen(buf));
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_em_mad_poll(gw_em_mad_t *em_mad, int jid)
{
    char buf[500];

    sprintf(buf, "POLL %d - -\n", jid);
    
    write(em_mad->em_mad_pipe, buf, strlen(buf));
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_em_mad_finalize (gw_em_mad_t *em_mad)
{
    char buf[50];
	int  status;
	pid_t pid;
	
    strcpy(buf, "FINALIZE - - -\n");
    write(em_mad->em_mad_pipe, buf, strlen(buf));

    close(em_mad->em_mad_pipe);
    close(em_mad->mad_em_pipe);
    
    pid = waitpid(em_mad->pid, &status, WNOHANG);
    
    if ( pid != em_mad->pid )
    {
#ifdef GWEMDEBUG    	
    	gw_log_print("UM",'I',"Waiting for execution MAD %s (pid %i) to finalize.\n"
    	             , em_mad->name, em_mad->pid);
#endif    	             
    	sleep(1);
    	waitpid(em_mad->pid, &status, WNOHANG);
    }	
    
    if ( em_mad->name != NULL )
	    free(em_mad->name);
    
    if ( em_mad->executable != NULL )
	    free(em_mad->executable);
	    
    if ( em_mad->mode != NULL )    
	    free(em_mad->mode);    

    return;
}
