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

#include "gw_em.h"
#include "gw_log.h"
#include "gw_em_rsl.h"

/*---------------------------------------------------------------------------*/
gw_em_t gw_em;
/*---------------------------------------------------------------------------*/

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

gw_em_t* gw_em_init()
{
	int rc;
	int um_em_pipe[2];
	
    /* ------------------------------------------------------------ */
    /* 1.- Execution Manager Initialization                         */
    /* ------------------------------------------------------------ */
        
    pthread_mutex_init(&(gw_em.mutex),(pthread_mutexattr_t *) NULL);

    pthread_mutex_lock(&(gw_em.mutex));
    
    rc = pipe(um_em_pipe);
    if ( rc == -1)
    	return NULL;
    	
    gw_em.um_em_pipe_r = um_em_pipe[0];
    gw_em.um_em_pipe_w = um_em_pipe[1];
    
    fcntl(gw_em.um_em_pipe_r, F_SETFD, FD_CLOEXEC);
    fcntl(gw_em.um_em_pipe_w, F_SETFD, FD_CLOEXEC);    

    gw_em.dm_am = NULL;

    /* ---------------------------------------------------- */
    /* 2.- Init Action Manager                              */
    /* ---------------------------------------------------- */

    gw_am_init(&(gw_em.am));

    /* ---------------------------------------------------- */
    /* 2.1- Execution Manager Actions                       */
    /*      a.- Finalize                                    */
    /*      b.- Poll (Timer action)                         */
    /*      c.- Submit                                      */
    /*      d.- Cancel                                      */
    /* ---------------------------------------------------- */

    gw_am_register(GW_ACTION_FINALIZE, 
                   GW_ACTION_SEQUENTIAL, 
                   gw_em_finalize,
                   &(gw_em.am));
                   
    gw_am_register(GW_ACTION_TIMER, 
                   GW_ACTION_SEQUENTIAL, 
                   gw_em_timer,
                   &(gw_em.am));
                   
    gw_am_register("GW_EM_SUBMIT", 
                   GW_ACTION_SEQUENTIAL, 
                   gw_em_submit,
                   &(gw_em.am));
                                      
    gw_am_register("GW_EM_CANCEL", 
                   GW_ACTION_SEQUENTIAL, 
                   gw_em_cancel,
                   &(gw_em.am));

    /* ---------------------------------------------------- */
    /* 2.1- Execution Manager State-transtion Actions       */
    /*      a.- pending                                     */
    /*      b.- active                                      */
    /*      c.- suspended                                   */
    /*      d.- failed                                      */
    /* ---------------------------------------------------- */

    gw_am_register("GW_EM_STATE_PENDING", 
                   GW_ACTION_SEQUENTIAL, 
                   gw_em_pending,
                   &(gw_em.am));

    gw_am_register("GW_EM_STATE_ACTIVE", 
                   GW_ACTION_SEQUENTIAL, 
                   gw_em_active,
                   &(gw_em.am));

    gw_am_register("GW_EM_STATE_SUSPENDED", 
                   GW_ACTION_SEQUENTIAL, 
                   gw_em_suspended, 
                   &(gw_em.am));


    gw_am_register("GW_EM_STATE_DONE", 
                   GW_ACTION_SEQUENTIAL, 
                   gw_em_done,
                   &(gw_em.am));
                   
    gw_am_register("GW_EM_STATE_FAILED", 
                   GW_ACTION_SEQUENTIAL, 
                   gw_em_failed,
                   &(gw_em.am));

    pthread_mutex_unlock(&(gw_em.mutex));

    return &(gw_em);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_em_set_dm_am(gw_am_t *dm_am)
{
    pthread_mutex_lock(&(gw_em.mutex));
    
    gw_em.dm_am   = dm_am;
    
    pthread_mutex_unlock(&(gw_em.mutex));
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_em_finalize()
{    
    
    /* ---------------------------------- */
    /* 1.- Free Memory                    */
    /* ---------------------------------- */    
    
    pthread_mutex_lock(&(gw_em.mutex));
    
    close(gw_em.um_em_pipe_r);
    close(gw_em.um_em_pipe_w);

    /* ---------------------------------- */
    /* 2.- Free Action Manager            */
    /* ---------------------------------- */    

    gw_am_destroy(&(gw_em.am));

    /* ---------------------------------- */
    /* 4.- Cancel listener Thread         */
    /* ---------------------------------- */    

    pthread_cancel(gw_em.listener_thread);
    pthread_join(gw_em.listener_thread,NULL);
    
    pthread_mutex_unlock(&(gw_em.mutex));

    /* ----------------------------------- */
    
    pthread_mutex_destroy(&(gw_em.mutex));
        
    gw_log_print ("EM",'I',"Execution Manager finalized.\n");
    
    pthread_exit(0);  
}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_em_start ( void *_null )
{
    int rc;
       
    pthread_attr_t attr;
    sigset_t       sig_group;
            
    sigfillset(&sig_group);
         
    pthread_sigmask(SIG_BLOCK,&sig_group,NULL);

    /* ----------------------------------------------------- */
    /* 1.- Start the listener_thread to interact with MADs.  */
    /* ----------------------------------------------------- */
    
    pthread_mutex_lock(&(gw_em.mutex));
        
    pthread_attr_init (&attr);
    
    pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);
        
#ifdef GWEMDEBUG
    gw_log_print ("EM",'D',"Starting the listener thread.\n");
#endif

    rc = pthread_create(&(gw_em.listener_thread), &attr, (void *)gw_em_listener,
                NULL);
                
    if ( rc != 0 )
    {
        gw_log_print ("EM",'E',"Could not start listener thread.\n");
        return;
    }

    pthread_mutex_unlock(&(gw_em.mutex));

    /* ------------------------------- */
    /* 2.- Start the action Manager    */
    /* ------------------------------- */    
    
    gw_log_print ("EM",'I',"Execution Manager started.\n");
        
    gw_am_loop(&(gw_em.am), GW_EM_TIMER_PERIOD, NULL);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
