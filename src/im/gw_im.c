/* -------------------------------------------------------------------------- */
/* Copyright 2002-2010, GridWay Project Leads (GridWay.org)                   */
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

#include "gw_im.h"
#include "gw_log.h"

/* -------------------------------------------------------------------------- */

gw_im_t gw_im;

/* -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

gw_im_t* gw_im_init()
{        
    pthread_mutex_init(&(gw_im.mutex),(pthread_mutexattr_t *) NULL);

    pthread_mutex_lock(&(gw_im.mutex));

    /* ----------------------------------------------------- */
    /* Initialize IM structure                               */
    /* ----------------------------------------------------- */    

    gw_im.registered_mads = 0;

    gw_im.dm_am = NULL;

    gw_am_init(&(gw_im.am));

    /* ----------------------------------------------------- */
    /* Register IM events                                    */
    /*   - FINALIZE                                          */
    /*   - TIMER                                             */
    /* ----------------------------------------------------- */    

    gw_am_register(GW_ACTION_FINALIZE, 
                   GW_ACTION_SEQUENTIAL, 
                   gw_im_finalize,
                   &(gw_im.am));

    gw_am_register(GW_ACTION_TIMER, 
                   GW_ACTION_SEQUENTIAL, 
                   gw_im_timer,
                   &(gw_im.am));

    /* ---------------------------------------------------- */

    pthread_mutex_unlock(&(gw_im.mutex));
    
    return &(gw_im);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_im_set_dm_am(gw_am_t *dm_am)
{
    pthread_mutex_lock(&(gw_im.mutex));
    
    gw_im.dm_am = dm_am;
    
    pthread_mutex_unlock(&(gw_im.mutex));
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_im_finalize()
{    
    int i;
    
    pthread_mutex_lock(&(gw_im.mutex));

    /* ----------------------------------------------------- */
    /* Destroy Action Manager                                */
    /* ----------------------------------------------------- */    

    gw_am_destroy(&(gw_im.am));

    /* ----------------------------------------------------- */
    /* Stop Information MADs                                 */
    /* ----------------------------------------------------- */    
    
    for (i=0; i<gw_im.registered_mads; i++)
        gw_im_mad_finalize (&(gw_im.im_mad[i]));

    /* ----------------------------------------------------- */
    /* Stop Listener Thread                                  */
    /* ----------------------------------------------------- */    

    pthread_cancel(gw_im.listener_thread);
    pthread_join(gw_im.listener_thread, NULL);
    
    /* ----------------------------------------------------- */    
    
    pthread_mutex_unlock(&(gw_im.mutex));
        
    pthread_mutex_destroy(&(gw_im.mutex));
   
    gw_log_print ("IM",'I',"Information Manager finalized.\n");
    
    pthread_exit(0);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int gw_im_register_mad(const char * executable, 
                       const char * name, 
                       const char * arg)
{
    int           rc;
    int           i;
    gw_im_mad_t * mad;
    
    pthread_mutex_lock(&(gw_im.mutex));
    
    /* ----------------------------------------------------- */
    /* Check if there is space left                          */
    /* ----------------------------------------------------- */    
    
    if (gw_im.registered_mads == GW_MAX_MADS)
        return -1;
    else
    {
    	for (i=0 ; i<gw_im.registered_mads; i++)
    	{
    		if (strcmp(name,gw_im.im_mad[i].name)==0)
    		{
				gw_log_print ("IM",'W',"\tMAD %s already loaded.\n",
                              GWNSTR(name));
                              
				pthread_mutex_unlock(&(gw_im.mutex));
    			return 0;	
    		}
    	}
    }

    /* ----------------------------------------------------- */
    /* Init MAD structure and start the driver               */
    /* ----------------------------------------------------- */    
    
    mad = &(gw_im.im_mad[gw_im.registered_mads]);
    
    rc = gw_im_mad_init(mad, 
                        executable, 
                        name, 
                        arg,
                        gw_conf.im_mads[gw_im.registered_mads][GW_MAD_IM_EM_INDEX],
                        gw_conf.im_mads[gw_im.registered_mads][GW_MAD_IM_TM_INDEX]);
    
    if ( rc == 0 )
    {
        gw_im.registered_mads++;
        
	    gw_log_print ("IM",'I',"\tMAD %s loaded (exec: %s, arg: %s).\n",
                  GWNSTR(name),
                  GWNSTR(executable), 
                  GWNSTR(arg));        
    }
    else
        gw_log_print("IM",'E',"\tUnable to load MAD %s.\n",GWNSTR(executable));

    pthread_mutex_unlock(&(gw_im.mutex));

    return rc;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

gw_im_mad_t* gw_im_get_mad_by_name(const char *name)
{
    int i;
    gw_im_mad_t * mad = NULL;;

    pthread_mutex_lock(&(gw_im.mutex));

    for (i=0; i < gw_im.registered_mads; i++)
    {
        if (strcmp(name, gw_im.im_mad[i].name) == 0)
        {
            mad = &(gw_im.im_mad[i]);
            break;
        }
    }
    
    pthread_mutex_unlock(&(gw_im.mutex));

    return(mad);
}

/* -------------------------------------------------------------------------- */

gw_im_mad_t * gw_im_get_mad_by_fd(int fd)
{
	int i;
	gw_im_mad_t * mad = NULL;

    pthread_mutex_lock(&(gw_im.mutex));
    		
    for (i=0; i<gw_im.registered_mads; i++)
    {
    	if ( fd == gw_im.im_mad[i].mad_im_pipe)
    	{
    		mad = &(gw_im.im_mad[i]);
    		break;
    	}
    }

    pthread_mutex_unlock(&(gw_im.mutex));
    	
	return mad;	
}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_im_start ( void *_null )
{
    int rc;
    pthread_attr_t attr;
    sigset_t       sig_group;
            
    sigfillset(&sig_group);
         
    pthread_sigmask(SIG_BLOCK,&sig_group,NULL);

    /* ----------------------------------------------------- */
    /* Start the listener thread (MADS must be registered)   */
    /* ----------------------------------------------------- */    
   
    pthread_mutex_lock(&(gw_im.mutex));
        
    pthread_attr_init (&attr);
    
    pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);
        
    rc = pthread_create(&(gw_im.listener_thread), 
                        &attr, 
                        (void *)gw_im_listener,
                        NULL);                
    if ( rc != 0 )
    {
        gw_log_print ("IM",'E',"Unable to start the listener thread.\n");
        return;
    }

    pthread_mutex_unlock(&(gw_im.mutex));

    /* ----------------------------------------------------- */
    /* Start the Action Manager                              */
    /* ----------------------------------------------------- */    
    
    gw_log_print ("IM",'I',"Information Manager started.\n");

    gw_am_loop(&(gw_im.am), 5, NULL);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

int gw_im_set_pipes (fd_set *in_pipes, int *num_mads)
{
	int greater = 0;
	int i;
	int fd;

    pthread_mutex_lock(&(gw_im.mutex));
    
    *num_mads = gw_im.registered_mads;
    	
    FD_ZERO(in_pipes);
                
    for (i=0; i<gw_im.registered_mads; i++)
    {
    	fd = gw_im.im_mad[i].mad_im_pipe;
    	
        if (fd != -1)
        {
            FD_SET(fd, in_pipes);
            
            if ( fd > greater )
        	   greater = fd;
        }
    }	

    pthread_mutex_unlock(&(gw_im.mutex));
    	
	return greater;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

