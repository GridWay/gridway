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

#include "gw_action.h"
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

/******************************************************************************
 ******************************************************************************
         MODULE FUNCTION PROTOTYPES & IMPLEMENTATION (INTERNAL INTERFACE)
 ******************************************************************************
 ******************************************************************************
 */

int gw_action_list_destroy ( gw_action_list_t **action_list );
int gw_action_list_del     ( gw_action_list_t **action_list );
int gw_action_list_add     (gw_action_list_t **action_list, int action_idx, 
            void *action_arg);
            
int gw_am_is_registered(gw_am_t *action_mngr, const char *action_id);
int gw_am_post_action  (gw_am_t *action_mngr, int action_idx,void *action_args);
int gw_am_pop_action   (gw_am_t *action_mngr );

/******************************************************************************
 ******************************************************************************
 ******************************************************************************
 */

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */

int gw_action_list_destroy ( gw_action_list_t **action_list )
{
    if ( *action_list != NULL )
    {
        gw_action_list_destroy (&((*action_list)->next_action));
        free(*action_list);
        return 0;
    }
    else
        return -1;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */

int gw_action_list_add(gw_action_list_t **action_list, int action_idx, 
            void *action_arg)
{

    if ( *action_list == NULL )
    {
        *action_list = malloc ( sizeof(gw_action_list_t) );

        if ( *action_list == NULL )
            return -1;

        (*action_list)->action_idx  = action_idx;
        (*action_list)->action_arg  = action_arg;
        (*action_list)->next_action = NULL;
    }
    else
        gw_action_list_add (&((*action_list)->next_action), action_idx,
                action_arg);
    return 0;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */

int gw_action_list_del ( gw_action_list_t **action_list )
{
    gw_action_list_t *first_action;

    if ( *action_list != NULL )
    {
        first_action = *action_list;
        *action_list = first_action->next_action;

        free(first_action);
    }

    return 0;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */

int gw_am_is_registered (gw_am_t *action_mngr, const char *action_id)
{
    int         idx, found, actions;
    gw_action_t *action_rgs;

    idx   = 0;
    found = 0;

    action_rgs = action_mngr->action_rgs;
    actions    = action_mngr->registered_actions - 1;

    while ( !found )
    {
        if (strcmp(action_rgs[idx].action_id, action_id) == 0 )
            found = 1;
        else
            if ( idx == actions )
            {
                idx = -1;
                break;
            }
            else
                idx++;
    }

    return(idx);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */

int gw_am_post_action (gw_am_t *action_mngr, int action_idx, void *action_args)
{
    int error;

    error = gw_action_list_add(&(action_mngr->action_list), action_idx, 
                    action_args);

    action_mngr->pending_actions++;

    return(error);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */

int gw_am_pop_action ( gw_am_t  *action_mngr )
{
    int error;

    error = gw_action_list_del ( &(action_mngr->action_list) );

    action_mngr->pending_actions--;

    return(error);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */

/* ************************************************************************** *
 * ************************************************************************** *
 *             MODULE FUNCTION IMPLEMENTATION (PUBLIC INTERFACE)              *
 * ************************************************************************** *
 * ************************************************************************** */

int gw_am_init ( gw_am_t  *action_mngr )
{
    int i;

    pthread_mutex_init(&(action_mngr->mutex),(pthread_mutexattr_t *) NULL);

    pthread_cond_init (&(action_mngr->cond),(pthread_condattr_t *) NULL);

    pthread_mutex_lock(&(action_mngr->mutex));

    action_mngr->pending_actions = 0;
    action_mngr->action_list     = NULL;

    action_mngr->registered_actions = 0;

    for (i=0;i<GW_ACTION_NUMBER;i++)
    {
        action_mngr->action_rgs[i].threaded       = GW_ACTION_SEQUENTIAL;
        action_mngr->action_rgs[i].action_id[0]   = '\0';
        action_mngr->action_rgs[i].action_handler = NULL;
    }

    pthread_mutex_unlock(&(action_mngr->mutex));

    return 0;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */

int gw_am_destroy (gw_am_t  *action_mngr)
{
    pthread_mutex_unlock(&(action_mngr->mutex));

    pthread_mutex_destroy(&(action_mngr->mutex));

    pthread_cond_destroy (&(action_mngr->cond));

    gw_action_list_destroy(&(action_mngr->action_list));

    return 0;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */

int gw_am_register(const char *action_id, int threaded, 
            gw_action_handler_t action_handler, gw_am_t *action_mngr)
{
    int         actions;
    gw_action_t *action_rgs;

    pthread_mutex_lock(&(action_mngr->mutex));

    action_rgs = action_mngr->action_rgs;
    actions    = action_mngr->registered_actions;
    
    if (actions >= GW_ACTION_NUMBER)
    {
        pthread_mutex_unlock(&(action_mngr->mutex));
        return -1;
    }

    action_rgs[actions].threaded       = threaded;
    action_rgs[actions].action_handler = action_handler;

    strcpy(action_rgs[actions].action_id, action_id);

    action_mngr->registered_actions++;

    pthread_mutex_unlock(&(action_mngr->mutex));

    return 0;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */

int gw_am_trigger(gw_am_t *action_mngr, const char *action_id,void *action_arg)
{
    int action_idx, finalize_idx;

    pthread_mutex_lock(&(action_mngr->mutex));

    action_idx = gw_am_is_registered (action_mngr,action_id);

    if (action_idx == -1)
    {
        pthread_mutex_unlock(&(action_mngr->mutex));
        return -1;
    }

    finalize_idx = gw_am_is_registered (action_mngr,GW_ACTION_FINALIZE);

    if ( finalize_idx != -1 && action_mngr->action_list != NULL )
        if (action_mngr->action_list->action_idx == finalize_idx)
        {
            pthread_mutex_unlock(&(action_mngr->mutex));
            return 0;
        }
    
    gw_am_post_action (action_mngr,action_idx,action_arg);

    pthread_cond_signal(&(action_mngr->cond));

    pthread_mutex_unlock(&(action_mngr->mutex));

    return 0;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */

void gw_am_loop(gw_am_t *action_mngr, time_t  wait_timeout, void *timer_args)
{
    int finalize;
    int code;

    int action_idx;
    int threaded;

    gw_action_handler_t handler;
    pthread_t           thread_id;
    pthread_attr_t      attr;
    void                *args;

    char action_id[GW_ACTION_ID_LENGTH];

    struct timespec    timeout;

    finalize = 0;
    timeout.tv_sec  = time(NULL) + wait_timeout;
    timeout.tv_nsec = 0;

    pthread_attr_init (&attr);
    pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);

    while (finalize == 0)
    {
        pthread_mutex_lock(&(action_mngr->mutex));

        while ( action_mngr->pending_actions == 0 )
            if ( wait_timeout != 0 )
            {
                code = pthread_cond_timedwait(&(action_mngr->cond), 
                                &(action_mngr->mutex), &timeout);
                if ( code == ETIMEDOUT )
                {
                    action_idx = gw_am_is_registered(action_mngr,
                                        GW_ACTION_TIMER);
                    if (action_idx != -1)
                        gw_am_post_action (action_mngr,action_idx,timer_args);

                    code = 0;
                }
            }
            else
                code = pthread_cond_wait(&(action_mngr->cond), 
                            &(action_mngr->mutex));

        action_idx = action_mngr->action_list->action_idx;

        threaded = action_mngr->action_rgs[action_idx].threaded;
        handler  = action_mngr->action_rgs[action_idx].action_handler;
        args     = action_mngr->action_list->action_arg;

        strcpy(action_id,action_mngr->action_rgs[action_idx].action_id);

        gw_am_pop_action(action_mngr);

        pthread_mutex_unlock(&(action_mngr->mutex));

        if ( handler!=NULL )
        {
            switch (threaded)
            {
                case GW_ACTION_SEQUENTIAL:
                    handler(args);
                    break;

                case GW_ACTION_THREADED:
                    pthread_create(&thread_id,&attr,(void *)handler,args);
                    break;
            }
        }
        
        if(strcmp(action_id,GW_ACTION_FINALIZE) == 0 )
        {
            finalize = 1;
            break;
        }

        if(strcmp(action_id,GW_ACTION_TIMER) == 0 )
        {
            timeout.tv_sec  = time(NULL) + wait_timeout;
            timeout.tv_nsec = 0;
        }
    }

//    gw_am_destroy(action_mngr);

    return;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
