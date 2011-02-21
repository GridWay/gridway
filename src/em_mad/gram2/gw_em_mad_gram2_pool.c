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

#include "gw_em_mad_gram2.h"

/*----------------------------------------------------------------------------*/

struct job_pool_s  job_pool;

/*----------------------------------------------------------------------------*/


void init_job_pool(int max_jobs)
{
    int jid;

    job_pool.cancel_state = (int *) malloc (sizeof(int) * max_jobs);
    job_pool.job_contact  = (char **) malloc (sizeof(char *) * max_jobs);
    job_pool.max_jobs     = max_jobs;

    for (jid = 0; jid<job_pool.max_jobs ; jid++)
    {
        job_pool.job_contact[jid]  = NULL;
        job_pool.cancel_state[jid] = 0;
    }
}

/*----------------------------------------------------------------------------*/


void add_job( int jid, const char *job_contact )
{
    if ( job_pool.job_contact[jid] != NULL )
        return;
    
    job_pool.job_contact[jid]  = strdup(job_contact);
    job_pool.cancel_state[jid] = 0;
}

/*----------------------------------------------------------------------------*/

void del_job( int jid )
{
    if ( job_pool.job_contact[jid] != NULL )
    {
        free(job_pool.job_contact[jid]);
        job_pool.job_contact[jid]  = NULL;
        job_pool.cancel_state[jid] = 0;
    }
}

/*----------------------------------------------------------------------------*/

char *get_job_contact( int jid )
{
    return job_pool.job_contact[jid];
}

/*----------------------------------------------------------------------------*/

int get_max_jobs()
{
    return job_pool.max_jobs;
}

/*----------------------------------------------------------------------------*/


int get_jid( char *job_contact )
{
    int jid;

    for (jid = 0; jid<job_pool.max_jobs; jid++)
    {
        if ( job_pool.job_contact[jid] != NULL )
            if ( strcmp(job_contact, job_pool.job_contact[jid]) == 0 )
                break;
    }

    if (jid == job_pool.max_jobs)
        jid = -1;

    return jid;
}

/*----------------------------------------------------------------------------*/
