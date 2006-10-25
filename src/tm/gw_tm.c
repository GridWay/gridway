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
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <fcntl.h>

#include "gw_tm.h"
#include "gw_log.h"

/*---------------------------------------------------------------------------*/
gw_tm_t gw_tm;
/*---------------------------------------------------------------------------*/

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

gw_tm_t* gw_tm_init()
{
	int rc;
	int um_tm_pipe[2];
	
    /* ------------------------------------------------------------ */
    /* 1.- Execution Manager Initialization                         */
    /* ------------------------------------------------------------ */
        
    pthread_mutex_init(&(gw_tm.mutex),(pthread_mutexattr_t *) NULL);

    pthread_mutex_lock(&(gw_tm.mutex));

    rc = pipe(um_tm_pipe);
    if ( rc == -1)
    	return NULL;
    	
    gw_tm.um_tm_pipe_r = um_tm_pipe[0];
    gw_tm.um_tm_pipe_w = um_tm_pipe[1];   
    
    fcntl(gw_tm.um_tm_pipe_r, F_SETFD, FD_CLOEXEC);
    fcntl(gw_tm.um_tm_pipe_w, F_SETFD, FD_CLOEXEC);
        
    gw_tm.dm_am = NULL;
    
    /* ---------------------------------------------------- */
    /* 2.- Init Action Manager                              */
    /* ---------------------------------------------------- */

    gw_am_init(&(gw_tm.am));

    gw_am_register(GW_ACTION_FINALIZE, GW_ACTION_SEQUENTIAL, gw_tm_finalize,
            &(gw_tm.am));                

    gw_am_register(GW_ACTION_TIMER, GW_ACTION_SEQUENTIAL, gw_tm_timer,
            &(gw_tm.am));            
                  
    gw_am_register("GW_TM_PROLOG", GW_ACTION_SEQUENTIAL, gw_tm_prolog,
            &(gw_tm.am));
            
    gw_am_register("GW_TM_EPILOG", GW_ACTION_SEQUENTIAL, gw_tm_epilog,
            &(gw_tm.am));       

    pthread_mutex_unlock(&(gw_tm.mutex));

    /* ------------------------------------------------------------ */
    
    return &(gw_tm);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_tm_set_dm_am(gw_am_t *dm_am)
{
    pthread_mutex_lock(&(gw_tm.mutex));
    
    gw_tm.dm_am   = dm_am;
    
    pthread_mutex_unlock(&(gw_tm.mutex));
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_tm_finalize()
{    
    /* ---------------------------------- */
    /* 1.- Free Memory                    */
    /* ---------------------------------- */    
    
    pthread_mutex_lock(&(gw_tm.mutex));
    
    close(gw_tm.um_tm_pipe_r);
    close(gw_tm.um_tm_pipe_w);    

    /* ---------------------------------- */
    /* 2.- Free Action Manager            */
    /* ---------------------------------- */    

    gw_am_destroy(&(gw_tm.am));

    /* ---------------------------------- */
    /* 4.- Cancel listener Thread         */
    /* ---------------------------------- */    

    pthread_cancel(gw_tm.listener_thread);

    pthread_mutex_unlock(&(gw_tm.mutex));

    /* ----------------------------------- */
    
    pthread_mutex_destroy(&(gw_tm.mutex));

    gw_log_print ("TM",'I',"Transfer Manager finalized.\n");
        
    pthread_exit(0);      
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_tm_start ( void *_null )
{
    int rc;
       
    pthread_attr_t attr;
    sigset_t       sig_group;
            
    sigfillset(&sig_group);
         
    pthread_sigmask(SIG_BLOCK,&sig_group,NULL);

    /* ----------------------------------------------------- */
    /* 1.- Start the listener_thread to interact with MADs.  */
    /* ----------------------------------------------------- */
    
    pthread_mutex_lock(&(gw_tm.mutex));
        
    pthread_attr_init (&attr);
    
    pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);
        
    rc = pthread_create(&(gw_tm.listener_thread), &attr, (void *)gw_tm_listener,
                NULL);
                
    if ( rc != 0 )
    {
        gw_log_print ("TM",'E',"Could not start listener thread.\n");
        return;
    }

    pthread_mutex_unlock(&(gw_tm.mutex));

    /* ------------------------------- */
    /* 2.- Start the action Manager    */
    /* ------------------------------- */    
    
    gw_log_print ("TM",'I',"Transfer Manager started.\n");
        
    gw_am_loop(&(gw_tm.am),GW_TM_TIMER_PERIOD,NULL);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
