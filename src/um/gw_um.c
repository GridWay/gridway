/* -------------------------------------------------------------------------- */
/* Copyright 2002-2012, GridWay Project Leads (GridWay.org)                   */
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
#include <pthread.h>

#include "gw_um.h"
#include "gw_log.h"

/*---------------------------------------------------------------------------*/
static gw_um_t gw_um;
/*---------------------------------------------------------------------------*/

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

gw_um_t* gw_um_init()
{
    /* ------------------------------------------------------------ */
    /* 1.- Information Manager Initialization                       */
    /* ------------------------------------------------------------ */
        
    pthread_mutex_init(&(gw_um.mutex),(pthread_mutexattr_t *) NULL);

    pthread_mutex_lock(&(gw_um.mutex));

    /* ---------------------------------------------------- */
    /* 2.- Init Action Manager                              */
    /* ---------------------------------------------------- */

    gw_am_init(&(gw_um.am));

    /* ---------------------------------------------------- */
    /* 2.1- Information Manager Actions                     */
    /*      a.- Discover                                    */
    /*      b.- Monitor                                     */
    /* ---------------------------------------------------- */

    gw_am_register(GW_ACTION_FINALIZE, 
                   GW_ACTION_SEQUENTIAL, 
                   gw_um_finalize,
                   &(gw_um.am));

    gw_am_register(GW_ACTION_TIMER, 
                   GW_ACTION_SEQUENTIAL, 
                   gw_um_timer,
                   &(gw_um.am));

    pthread_mutex_unlock(&(gw_um.mutex));

    /* ------------------------------------------------------------ */
    
    return &(gw_um);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_um_finalize()
{    
    pthread_mutex_lock(&(gw_um.mutex));

    gw_am_destroy(&(gw_um.am));

    pthread_mutex_unlock(&(gw_um.mutex));
        
    pthread_mutex_destroy(&(gw_um.mutex));
   
    gw_log_print ("UM",'I',"User Manager finalized.\n");

    pthread_exit(0);    
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_um_start ( void *_null )
{
   /* ------------------------------- */
    /* 2.- Start the Action Manager    */
    /* ------------------------------- */    
    
    gw_log_print ("UM",'I',"User Manager started.\n");

    gw_am_loop(&(gw_um.am), 180, NULL);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void gw_um_timer(void *_null)
{
    static int    mark = 0;

    mark = mark + 180;
    if ( mark >= 300 )
    {
        gw_log_print ("UM",'I',"-- MARK --\n");
        mark = 0;
    }	
		
	gw_user_pool_check_users(180);		
}

