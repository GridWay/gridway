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

#include "gw_dm_mad.h"
#include "gw_dm.h"
#include "gw_log.h"

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
int gw_dm_mad_init(gw_dm_mad_t *dm_mad, const char *exe, const char *name, 
		const char *args)
{
    char buf[50];
    char str[GW_DM_MAX_STRING], c;
    char info[GW_DM_MAX_INFO];
    char s_id[GW_DM_MAX_ID];
    char result[GW_DM_MAX_RESULT];
    char action[GW_DM_MAX_ACTION];
    
    int dm_mad_pipe[2], mad_dm_pipe[2];
    int i, rc;

    dm_mad->name        = strdup(name);
    dm_mad->executable  = strdup(exe);
    
    if (args != NULL)
        dm_mad->argument    = strdup(args);
	
    if ( pipe(dm_mad_pipe) == -1 ||  pipe(mad_dm_pipe) == -1)
        return -1;

    dm_mad->pid = fork();

    switch (dm_mad->pid)
	{
        case -1: /* Error */
            return -1;
            break;

        case 0: /* Child process (MAD) */
            close(dm_mad_pipe[1]);
            close(mad_dm_pipe[0]);
            
            /* stdin and stdout redirection */
            if ( dup2(dm_mad_pipe[0], 0) != 0 || dup2(mad_dm_pipe[1], 1) != 1)
                exit(-1);
            
            close(dm_mad_pipe[0]);
            close(mad_dm_pipe[1]);            
            
            execlp(dm_mad->executable, dm_mad->executable, dm_mad->argument, NULL);
	    
		    /* exec should not return */
            gw_log_print("DM",'E',"Could not execute MAD %s.\n", dm_mad->executable);

	    	exit(-1);
            break;

        default: /* Parent process (DM) */
            close(dm_mad_pipe[0]);
            close(mad_dm_pipe[1]);

            dm_mad->dm_mad_pipe = dm_mad_pipe[1];
            dm_mad->mad_dm_pipe = mad_dm_pipe[0];

            fcntl(dm_mad->dm_mad_pipe, F_SETFD, FD_CLOEXEC); /* Close pipes in other MADs*/
            fcntl(dm_mad->mad_dm_pipe, F_SETFD, FD_CLOEXEC);

            strcpy(buf, "INIT - - -\n");
            write(dm_mad->dm_mad_pipe, buf, strlen(buf));

            i = 0;
            
            do
			{
                rc = read(dm_mad->mad_dm_pipe, (void *) &c, sizeof(char));
                str[i++] = c;              
            }
			while ( rc > 0 && c != '\n' &&  c != '\0');

            str[i] = '\0';
                            
            if (rc <= 0)
			{
				gw_dm_mad_finalize (dm_mad);                
				return -1;
            }

            sscanf(str,"%s %s %s %[^\n]", action, s_id, result, info);

            if (strcmp(action, "INIT") != 0 || strcmp(result, "SUCCESS") != 0)
            {
				gw_dm_mad_finalize (dm_mad);
                return -1;
            }
 
            break;
    }

    return 0;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_dm_mad_schedule(gw_dm_mad_t *dm_mad)
{
    char buf[GW_DM_MAX_STRING];

    sprintf(buf, "SCHEDULE - - -\n");
    write(dm_mad->dm_mad_pipe, buf, strlen(buf));
    
    return;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_dm_mad_finalize (gw_dm_mad_t *dm_mad)
{
    char buf[50];
	int  status;
	pid_t pid;
	
    strcpy(buf, "FINALIZE - - -\n");
    
    write(dm_mad->dm_mad_pipe, buf, strlen(buf));

    close(dm_mad->dm_mad_pipe);
    close(dm_mad->mad_dm_pipe);
    
    pid = waitpid(dm_mad->pid, &status, WNOHANG);
    
    if ( pid != dm_mad->pid )
    {
#ifdef GWDMDEBUG    	
    	gw_log_print("DM",'D',"Waiting for scheduler %s (pid %i) to finalize.\n"
    	             ,dm_mad->name, dm_mad->pid);
#endif
    	sleep(1);
    	waitpid(dm_mad->pid, &status, WNOHANG);
    }	    
    
    free(dm_mad->name);
    free(dm_mad->executable);
    free(dm_mad->argument);    

    return;
}
