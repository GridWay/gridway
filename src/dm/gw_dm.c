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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>

#include "gw_dm.h"
#include "gw_log.h"
#include "gw_conf.h"
#include "gw_host_pool.h"
#include "gw_user_pool.h"

/* -------------------------------------------------------------------------- */
gw_dm_t gw_dm;
/* -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

gw_dm_t * gw_dm_init()
{
    /* ------------------------------------------------------------ */
    /* 1.- Dispatch Manager Initialization                          */
    /* ------------------------------------------------------------ */
        
    pthread_mutex_init(&(gw_dm.mutex),(pthread_mutexattr_t *) NULL);

    pthread_mutex_lock(&(gw_dm.mutex));
    
    gw_dm.em_am = NULL;
    gw_dm.tm_am = NULL;    
    gw_dm.rm_am = NULL;
    
    gw_dm.scheduling = GW_FALSE;

    /* ----------------------------------------------------- */
    /* 2.- Init Action Manager                               */
    /* ----------------------------------------------------- */
        
    gw_am_init(&(gw_dm.am));

    /* ---------------------------------------------------- */
    /* 2.1- Dispatch Manager Actions                        */
    /*      a.- hold                                        */
    /*      b.- release                                     */
    /*      c.- stop                                        */
    /*      d.- resume                                      */
    /*      e.- kill                                        */
    /*      f.- wait                                        */
    /*      g.- re-schedule                                 */
    /* ---------------------------------------------------- */
    
    gw_am_register(GW_ACTION_FINALIZE, 
                   GW_ACTION_SEQUENTIAL, 
                   gw_dm_finalize,
                   &(gw_dm.am));

    gw_am_register("GW_DM_ALLOCATE_JOB", 
                   GW_ACTION_SEQUENTIAL, 
                   gw_dm_jalloc,
                   &(gw_dm.am));
            
    gw_am_register("GW_DM_ALLOCATE_ARRAY", 
                   GW_ACTION_SEQUENTIAL, 
                   gw_dm_aalloc,
                   &(gw_dm.am));
            
    gw_am_register("GW_DM_HOLD", 
                   GW_ACTION_SEQUENTIAL, 
                   gw_dm_hold,
                   &(gw_dm.am));

    gw_am_register("GW_DM_RELEASE", 
                   GW_ACTION_SEQUENTIAL, 
                   gw_dm_release,
                   &(gw_dm.am));

    gw_am_register("GW_DM_STOP", 
                   GW_ACTION_SEQUENTIAL, 
                   gw_dm_stop,
                   &(gw_dm.am));
            
    gw_am_register("GW_DM_RESUME", 
                   GW_ACTION_SEQUENTIAL, 
                   gw_dm_resume,
                   &(gw_dm.am));
    
    gw_am_register("GW_DM_KILL", 
                   GW_ACTION_SEQUENTIAL, 
                   gw_dm_kill,
                   &(gw_dm.am));
                   
    gw_am_register("GW_DM_KILL_HARD", 
                   GW_ACTION_SEQUENTIAL, 
                   gw_dm_kill_hard,
                   &(gw_dm.am));                   

    gw_am_register("GW_DM_WAIT", 
                   GW_ACTION_SEQUENTIAL, 
                   gw_dm_wait,
                   &(gw_dm.am));

    gw_am_register("GW_DM_RESCHEDULE", 
                   GW_ACTION_SEQUENTIAL, 
                   gw_dm_reschedule,
                   &(gw_dm.am));
            
    gw_am_register("GW_DM_PROLOG_DONE", 
                   GW_ACTION_SEQUENTIAL,
                   gw_dm_prolog_done_cb, 
                   &(gw_dm.am));

    gw_am_register("GW_DM_PROLOG_FAILED", 
                   GW_ACTION_SEQUENTIAL,
                   gw_dm_prolog_failed_cb, 
                   &(gw_dm.am));

    gw_am_register("GW_DM_EPILOG_DONE", 
                   GW_ACTION_SEQUENTIAL,
                   gw_dm_epilog_done_cb, 
                   &(gw_dm.am));

    gw_am_register("GW_DM_EPILOG_FAILED",
                   GW_ACTION_SEQUENTIAL,
                   gw_dm_epilog_failed_cb, 
                   &(gw_dm.am));

    gw_am_register("GW_DM_WRAPPER_DONE", 
                   GW_ACTION_SEQUENTIAL,
                   gw_dm_wrapper_done_cb, 
                   &(gw_dm.am));

    gw_am_register("GW_DM_WRAPPER_FAILED",
                   GW_ACTION_SEQUENTIAL,
                   gw_dm_wrapper_failed_cb, 
                   &(gw_dm.am));
                           
    /* ---------------------------------------------------- */
    /* 2.2- Dispatch Manager State Transitions              */
    /*      a.- prolog                                      */
    /*      b.- migr_prolog                                 */
    /*      c.- pre-wrapper                                 */
    /*      d.- epilog                                      */
    /*      e.- migr_epilog                                 */
    /*      f.- stop_epilog                                 */
    /*      g.- kill_epilog                                 */
    /*      h.- zombie                                      */    
    /* ---------------------------------------------------- */
            
    gw_am_register("GW_DM_STATE_PROLOG", 
                   GW_ACTION_SEQUENTIAL, 
                   gw_dm_prolog,
                   &(gw_dm.am));
                   
    gw_am_register("GW_DM_STATE_MIGR_PROLOG", 
                   GW_ACTION_SEQUENTIAL,
                   gw_dm_migr_prolog, 
                   &(gw_dm.am));

    gw_am_register("GW_DM_STATE_PRE_WRAPPER", 
                   GW_ACTION_SEQUENTIAL,
                   gw_dm_pre_wrapper, 
                   &(gw_dm.am));
                   
    gw_am_register("GW_DM_STATE_WRAPPER", 
                   GW_ACTION_SEQUENTIAL,
                   gw_dm_wrapper, 
                   &(gw_dm.am));

    gw_am_register("GW_DM_STATE_EPILOG", 
                   GW_ACTION_SEQUENTIAL, 
                   gw_dm_epilog,
                   &(gw_dm.am));
                   
    gw_am_register("GW_DM_STATE_EPILOG_STD",
                   GW_ACTION_SEQUENTIAL,
                   gw_dm_epilog_std,
                   &(gw_dm.am));
                   
    gw_am_register("GW_DM_STATE_MIGR_EPILOG", 
                   GW_ACTION_SEQUENTIAL,
                   gw_dm_migr_epilog, 
                   &(gw_dm.am));
                   
    gw_am_register("GW_DM_STATE_STOP_EPILOG", 
                   GW_ACTION_SEQUENTIAL,
                   gw_dm_stop_epilog, 
                   &(gw_dm.am));
                   
    gw_am_register("GW_DM_STATE_KILL_EPILOG", 
                   GW_ACTION_SEQUENTIAL,
                   gw_dm_kill_epilog, 
                   &(gw_dm.am));
                   
    gw_am_register("GW_DM_STATE_EPILOG_FAIL", 
                   GW_ACTION_SEQUENTIAL, 
                   gw_dm_epilog_fail,
                   &(gw_dm.am));
                   
    gw_am_register("GW_DM_STATE_EPILOG_RESTART", 
                   GW_ACTION_SEQUENTIAL, 
                   gw_dm_epilog_restart,
                   &(gw_dm.am)); 

    gw_am_register("GW_DM_STATE_MIGR_CANCEL", 
                   GW_ACTION_SEQUENTIAL, 
                   gw_dm_migr_cancel,
                   &(gw_dm.am));

    gw_am_register("GW_DM_STATE_PENDING", 
                   GW_ACTION_SEQUENTIAL, 
                   gw_dm_pending,
                   &(gw_dm.am));
                   
    gw_am_register("GW_DM_STATE_ZOMBIE", 
                   GW_ACTION_SEQUENTIAL, 
                   gw_dm_zombie,
                   &(gw_dm.am));
                   
    gw_am_register("GW_DM_STATE_FAILED", 
                   GW_ACTION_SEQUENTIAL, 
                   gw_dm_failed,
                   &(gw_dm.am)); 
            
    gw_am_register("GW_DM_STATE_STOPPED", 
                   GW_ACTION_SEQUENTIAL, 
                   gw_dm_stopped,
                   &(gw_dm.am));
                                               
    gw_am_register(GW_ACTION_TIMER, 
                   GW_ACTION_THREADED, 
                   gw_dm_schedule,
                   &(gw_dm.am));

    pthread_mutex_unlock(&(gw_dm.mutex));
    
    return &(gw_dm);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_dm_finalize()
{
    int i;
    
    pthread_mutex_lock(&(gw_dm.mutex));

    /* ---------------------------------- */
    /* 1.- Free Memory                    */
    /* ---------------------------------- */    
    
    /* ---------------------------------- */
    /* 2.- Free Action Manager            */
    /* ---------------------------------- */    

    gw_am_destroy(&(gw_dm.am));

    /* ---------------------------------- */
    /* 4.- Cancel listener Thread         */
    /* ---------------------------------- */    

    pthread_cancel(gw_dm.listener_thread);
    
    pthread_join(gw_dm.listener_thread, NULL);
    
    /* ---------------------------------- */
    /* 3.- Stop Registered MADs           */
    /* ---------------------------------- */    
    
    for (i=0; i<gw_dm.registered_mads; i++)
        gw_dm_mad_finalize (&(gw_dm.dm_mad[i]));

    pthread_mutex_unlock(&(gw_dm.mutex));

    gw_log_print("DM",'I',"Dispatch Manager finalized.\n");
        
    pthread_mutex_destroy(&(gw_dm.mutex)); 
    
    pthread_exit(0);      
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int gw_dm_register_mad(const char *executable, const char *name, 
            const char *arg)
{
    int rc;
    gw_dm_mad_t *mad;
    
    pthread_mutex_lock(&(gw_dm.mutex));

    /* ----------------------------------------------------- */
    /* 1.- Check if there is space left                      */
    /* We do not check if this mad was already registered... */
    /* ----------------------------------------------------- */    
    
    if (gw_dm.registered_mads == GW_MAX_MADS)
    {
        gw_log_print("DM",'E',"\tCould not load sheduler %s, max number of MADs reached.\n",
                GWNSTR(executable));
        pthread_mutex_unlock(&(gw_dm.mutex));
        return -1;
    }

    /* ----------------------------------------------------- */
    /* 2.- Init MAD structure and start the driver           */
    /* ----------------------------------------------------- */    
    
    mad = &(gw_dm.dm_mad[gw_dm.registered_mads]);
    
    rc = gw_dm_mad_init(mad,executable,name,arg);
    
    if ( rc == 0 )
    {        
        gw_dm.registered_mads++;
        
    	gw_log_print ("DM",'I',"\tScheduler %s loaded (exec: %s, arg: %s).\n",
				  GWNSTR(name),
				  GWNSTR(executable), 
				  GWNSTR(arg));
	}
    else
    {
        gw_log_print("DM",'E',"\tCould not load scheduler %s, check name/path.\n",
                GWNSTR(executable));
    }

    pthread_mutex_unlock(&(gw_dm.mutex));

    return rc;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

gw_dm_mad_t* gw_dm_get_mad(const char *name)
{
    gw_dm_mad_t *mad;
    int i;

    pthread_mutex_lock(&(gw_dm.mutex));

    mad = NULL;

    for (i=0; i < gw_dm.registered_mads; i++)
        if (strcmp(name, gw_dm.dm_mad[i].name) == 0)
        {
            mad = &(gw_dm.dm_mad[i]);
            break;
        }

    pthread_mutex_unlock(&(gw_dm.mutex));

    return(mad);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_dm_set_em_am ( gw_am_t *em_am )
{
    pthread_mutex_lock(&(gw_dm.mutex));

    gw_dm.em_am = em_am;
    
    pthread_mutex_unlock(&(gw_dm.mutex));
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_dm_set_tm_am ( gw_am_t *tm_am )
{
    pthread_mutex_lock(&(gw_dm.mutex));

    gw_dm.tm_am = tm_am;
    
    pthread_mutex_unlock(&(gw_dm.mutex));
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_dm_set_rm_am ( gw_am_t *rm_am )
{
    pthread_mutex_lock(&(gw_dm.mutex));

    gw_dm.rm_am = rm_am;
    
    pthread_mutex_unlock(&(gw_dm.mutex));
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_dm_start ()
{
    int rc;
    pthread_attr_t attr;
    sigset_t sig_group;
            
    sigfillset(&sig_group);

    pthread_sigmask(SIG_BLOCK, &sig_group, NULL);

    /* ----------------------------------------------------- */
    /* 1.- Start the listener_thread to interact with MADs.  */
    /* MADs should be already registered!!!                  */
    /* ----------------------------------------------------- */
    
    pthread_mutex_lock(&(gw_dm.mutex));
        
    pthread_attr_init (&attr);
    
    pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);
        
    rc = pthread_create(&(gw_dm.listener_thread), &attr, (void *)gw_dm_listener,
                NULL);
                
    if ( rc != 0 )
    {
        gw_log_print ("DM",'E',"Could not initialize listener thread.\n");
        return;
    }

    pthread_mutex_unlock(&(gw_dm.mutex));
    
    /* ------------------------------- */
    /* 2.- Start the Action Manager    */
    /* ------------------------------- */    

    gw_log_print("DM",'I',"Dispatch Manager started.\n");
             
    gw_am_loop(&(gw_dm.am), gw_conf.scheduling_interval, NULL);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

gw_dm_mad_t * gw_dm_get_mad_by_fd(int fd)
{
    int i;
    gw_dm_mad_t * mad = NULL;

    pthread_mutex_lock(&(gw_dm.mutex));
            
    for (i=0; i<gw_dm.registered_mads; i++)
    {
        if ( fd == gw_dm.dm_mad[i].mad_dm_pipe)
        {
            mad = &(gw_dm.dm_mad[i]);
            break;
        }
    }

    pthread_mutex_unlock(&(gw_dm.mutex));
        
    return mad; 
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int gw_dm_set_pipes (fd_set *in_pipes, int *num_mads)
{
    int greater = 0;
    int i;
    int fd;
    
    pthread_mutex_lock(&(gw_dm.mutex));
    
    *num_mads = gw_dm.registered_mads;
    
    FD_ZERO(in_pipes);
    greater = 0;
        
    for (i= 0; i<gw_dm.registered_mads; i++)
    {
        fd = gw_dm.dm_mad[i].mad_dm_pipe;
            
        if (fd != -1)
        {
            FD_SET(fd, in_pipes);
            
            if ( fd > greater )
                greater = fd;
        }            
    }
    
    pthread_mutex_unlock(&(gw_dm.mutex));
    
    return greater;
}
