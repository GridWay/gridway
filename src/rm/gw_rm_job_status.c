/* -------------------------------------------------------------------------- */
/* Copyright 2002-2011, GridWay Project Leads (GridWay.org)                   */
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
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>

#include "gw_rm.h"
#include "gw_rm_msg.h"
#include "gw_log.h"
#include "gw_job_pool.h"

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

void gw_rm_job_status(int client_socket, int job_id)
{
    gw_job_t *     job;
    gw_msg_job_t   msg;
    int            rc;
    int            length;
    gw_history_t * tmp;
      
    job          = gw_job_pool_get(job_id, GW_TRUE);
    length       = sizeof(gw_msg_job_t);
    msg.msg_type = GW_MSG_JOB_STATUS;
      
    if ( job == NULL )
    {
        msg.rc = GW_RC_FAILED_BAD_JOB_ID;
        msg.id = job_id;
        rc     = send(client_socket,(void *) &msg,length,0);
        return;
    }
    
    if ( job->owner != NULL )
        strncpy(msg.owner, job->owner,GW_MSG_STRING_SHORT);
    
    msg.rc = GW_RC_SUCCESS;
    
    msg.id          = job->id;
    msg.array_id    = job->array_id;
    msg.task_id     = job->task_id;
    msg.total_tasks = job->total_tasks;    
    msg.uid         = job->user_id;

    msg.fixed_priority = job->fixed_priority;
    msg.deadline       = job->template.deadline;

    msg.type = job->template.type;
    msg.np   = job->template.np;

    msg.em_state  = job->em_state;
    msg.job_state = job->job_state;
    msg.exit_code = job->exit_code;

    msg.restarted      = job->restarted;
    msg.client_waiting = job->client_waiting;
    msg.reschedule     = job->reschedule;
    
    msg.start_time = job->start_time;
    msg.exit_time  = job->exit_time;
    msg.cpu_time   = gw_job_history_get_wrapper_time(job->history);
    msg.xfr_time   = gw_job_history_get_prolog_time(job->history)
            + gw_job_history_get_epilog_time(job->history)
            + gw_job_history_get_migration_time(job->history);
    
    /* Agregate stats from other hosts in history */
    if ( job->history != NULL )
    {
        tmp = job->history->next;
        while (tmp != NULL )
        {
            msg.cpu_time += gw_job_history_get_wrapper_time(tmp); 
            msg.xfr_time += gw_job_history_get_prolog_time(tmp)
                    + gw_job_history_get_epilog_time(tmp)
                    + gw_job_history_get_migration_time(tmp);
            tmp = tmp->next;
        }
    }
    
    if ( job->template.name != NULL )
        strncpy(msg.name,job->template.name,GW_MSG_STRING_SHORT);

    msg.host[0]='\0';
    if ( job->history != NULL )
        if (job->history->em_rc != NULL)
        {
            strncpy(msg.host,job->history->em_rc,GW_MSG_STRING_USER_AT_HOST);
            if ( strlen(job->history->em_rc) >= GW_MSG_STRING_USER_AT_HOST )
                msg.host[GW_MSG_STRING_USER_AT_HOST-1] = '\0';
        }
            
    pthread_mutex_unlock(&(job->mutex));
    
    rc = send(client_socket,(void *) &msg,length,0);
    
    if ( rc == -1 )
        gw_log_print("RM",'E',"Error sending message %s\n",strerror(errno));
}
         
    
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

void gw_rm_job_pool_status(int client_socket)
{
    int          job_id;
    gw_job_t *   job;
    gw_msg_job_t msg;
    int          length;
    int          rc;
          
    length = sizeof(gw_msg_job_t);
    
    for ( job_id=0; job_id < gw_conf.number_of_jobs ; job_id++)
    {
        job = gw_job_pool_get(job_id, GW_FALSE);
        
        if (job != NULL)
            gw_rm_job_status (client_socket, job_id);            
    }
    
    msg.msg_type = GW_MSG_END;
    msg.rc = GW_RC_SUCCESS;
    
    rc = send(client_socket,(void *) &msg,length,0);
    
    if ( rc == -1 )
        gw_log_print("RM",'E',"Error sending message %s\n",strerror(errno));
}
