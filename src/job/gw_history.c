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
#include <string.h>
#include <stdio.h>
#include <time.h>

#include "gw_history.h"
#include "gw_host.h"
#include "gw_common.h"
#include "gw_tm.h"
#include "gw_em.h"
#include "gw_log.h"
#include "gw_user_pool.h"

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

void gw_job_history_init(gw_history_t **job_history)
{
    *job_history = NULL;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

int gw_job_history_destroy(gw_history_t **job_history)
{
    if (*job_history != NULL)
    {
        gw_job_history_destroy(&((*job_history)->next));

    	if ((*job_history)->rdir != NULL)
			free((*job_history)->rdir);

    	if ((*job_history)->em_rc != NULL)
			free((*job_history)->em_rc);

    	if ((*job_history)->em_fork_rc != NULL)
			free((*job_history)->em_fork_rc);

    	if ((*job_history)->queue != NULL)
			free((*job_history)->queue);
       
        free(*job_history);

        return 0;
    }
    else
        return -1;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

int gw_job_history_add(gw_history_t **job_history, 
                       gw_host_t    *host,
                       int          rank, 
                       char         *queue,
                       const char   *fork, 
                       const char   *lrms, 
                       const char   *owner,
                       const char   *job_home, 
                       int          jid,
                       int          uid,
                       gw_boolean_t recover)
{
    gw_history_t   *new_record;
    int            i;
    char           tmp[256];
    char           history_file[2048];
    FILE           *file;
    char           *se;
    
    new_record = (gw_history_t *)malloc(sizeof(gw_history_t));

    if (new_record == NULL)
        return -1;        
    
    new_record->tm_mad = gw_user_pool_get_tm_mad(uid, host->tm_mad);
    
    if ( new_record->tm_mad == NULL)
    {
        gw_log_print("DM", 'E',"Transfer MAD (%s) not loaded for user %s.\n",
                host->tm_mad, owner);
        free(new_record);
        return -1;
    }

	new_record->em_mad = gw_user_pool_get_em_mad(uid, host->em_mad);

    if (new_record->em_mad == NULL)
    {
        gw_log_print("DM", 'E',"Execution MAD (%s) not loaded for user %s.\n",
                host->em_mad, owner);
        free(new_record);
        return -1;
    }
    
    new_record->rank   = rank;
    new_record->host   = host;
    new_record->tries  = 0;
    new_record->polls  = 0;
	
	se = gw_host_get_genvar_str("SE_HOSTNAME", 0, host);

	if ((se == NULL) || (se[0] == '\0'))
	{
	    sprintf(tmp,"gsiftp://%s/~/.gw_%s_%i/", host->hostname, owner, jid);
	    new_record->rdir = strdup(tmp);
	}
	else
	{
	    sprintf(tmp,"gsiftp://%s/~/.gw_%s_%i/", se, owner, jid);
	    new_record->rdir = strdup(tmp);			
	}
    
	if (lrms != NULL)
	{
    	sprintf(tmp,"%s/%s", host->hostname, lrms);
	    new_record->em_rc = strdup(tmp);
	}
	else/* Use the default Jobmanager!!!*/
    	new_record->em_rc = strdup(host->hostname);	

    
    if ( fork != NULL )
    {
	    sprintf(tmp,"%s/%s", host->hostname, fork);
	   	new_record->em_fork_rc  = strdup(tmp);
    }
    else /* Use the default Jobmanager*/
    	new_record->em_fork_rc  = strdup(host->hostname);
    
    if ( queue != NULL )
    	new_record->queue = strdup(queue);
    else
    	new_record->queue = NULL;

    for (i=0; i<GW_HISTORY_MAX_STATS; i++)
        new_record->stats[i] = 0;

    new_record->reason = GW_REASON_NONE;

    new_record->next = *job_history;
    *job_history = new_record;

    /* Save history to persistent storage (not for a recovey action) */
    if (!recover)
    {
        sprintf(history_file, "%s/var/%d/job.history", gw_conf.gw_location, jid);

        file = fopen(history_file, "a");

        if (file == NULL)
        {
            gw_log_print("DM",'E',"Could not open history file of job %d.\n", jid); 
            return -1;
        }

        fprintf(file, "%s %d %s %s %s %d %s %s %s\n", host->hostname,
                rank, queue, fork, lrms, host->nice,
                host->em_mad, host->tm_mad, host->im_mad);

        fclose(file);
    }

    return 0;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

int  gw_job_history_number_of_records(gw_history_t *job_history)
{
    int records = 0;

    while (job_history != NULL)
    {
        records++;
        job_history = job_history->next;
    }

    return records;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

int  gw_job_history_is_in_records(gw_history_t *job_history, char *hostname)
{
    int found = 0;

    while (job_history != NULL)
    {
        if (job_history->host != NULL
                && strcmp(hostname, job_history->host->hostname) == 0)
        {
            found = 1;
            break;
        }
        else
            job_history = job_history->next;
    }

    return found;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

void  gw_job_history_print_record(FILE *fd, gw_history_t *job_history)
{

    gw_print (fd,"DM",'I',"----------- Job history record -----------\n");
    
    if (job_history->host != NULL)
        gw_host_print(fd, job_history->host);
        
    gw_print (fd,"DM",'I',"\tHost GRAM contact = %s\n",job_history->em_rc);
    gw_print (fd,"DM",'I',"\tRemote job dir    = %s\n",job_history->rdir);
    gw_print (fd,"DM",'I',"\tHost Rank         = %i\n",job_history->rank);
    gw_print (fd,"DM",'I',"\tSubmission tries  = %i\n",job_history->tries);
    gw_print (fd,"DM",'I',"\tStart time        = %i\n",
            (int) job_history->stats[START_TIME]);
    gw_print (fd,"DM",'I',"\tExit Time         = %i\n",
            (int) job_history->stats[EXIT_TIME]);
    gw_print (fd,"DM",'I',"\tProlog Time       = %i\n",
            (int) job_history->stats[PROLOG_EXIT_TIME]
            - (int) job_history->stats[PROLOG_START_TIME]);
    gw_print (fd,"DM",'I',"\tWrapper Time      = %i\n",
            (int) job_history->stats[WRAPPER_EXIT_TIME]
            - (int) job_history->stats[WRAPPER_START_TIME]);
    gw_print (fd,"DM",'I',"\tEpilog Time       = %i\n",
            (int) job_history->stats[EPILOG_EXIT_TIME]
            - (int) job_history->stats[EPILOG_START_TIME]);
    gw_print (fd,"DM",'I',"\tMigration Time    = %i\n",
            (int) job_history->stats[MIGRATION_EXIT_TIME]
            - (int) job_history->stats[MIGRATION_START_TIME]);
    gw_print (fd,"DM",'I',"------------------------------------------\n");
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

void  gw_job_history_print(FILE *fd, gw_history_t *job_history)
{
    gw_history_t * p;
    
    p = job_history;
    
    while (p != NULL) {
        gw_job_history_print_record(fd, p);
        p = p->next;
    }
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

time_t gw_job_history_get_prolog_time (gw_history_t *history)
{
    time_t prolog_time;
    time_t prolog_start;
    time_t prolog_exit;

    if (history == NULL)
        return -1;

    prolog_start = history->stats[PROLOG_START_TIME];
    prolog_exit  = history->stats[PROLOG_EXIT_TIME];
    prolog_time  = 0;

    if (prolog_start !=0 && prolog_exit == 0)
        prolog_time = time(NULL) - prolog_start;
    else
        prolog_time = prolog_exit - prolog_start;

    return(prolog_time);
}

/*----------------------------------------------------------------------------*/

time_t gw_job_history_get_wrapper_time(gw_history_t *history)
{
    time_t wrapper_time;
    time_t wrapper_start;
    time_t wrapper_exit;

    if (history == NULL)
        return -1;

    wrapper_start   = history->stats[WRAPPER_START_TIME];
    wrapper_exit    = history->stats[WRAPPER_EXIT_TIME];
    wrapper_time    = 0;

    if (wrapper_start !=0 && wrapper_exit == 0)
        wrapper_time = time(NULL) - wrapper_start;
    else
        wrapper_time = wrapper_exit -  wrapper_start;

    return(wrapper_time);
}

/*----------------------------------------------------------------------------*/

time_t gw_job_history_get_pre_wrapper_time(gw_history_t *history)
{
    time_t pre_wrapper_time;
    time_t pre_wrapper_start;
    time_t pre_wrapper_exit;

    if (history == NULL)
        return -1;

    pre_wrapper_start   = history->stats[PRE_WRAPPER_START_TIME];
    pre_wrapper_exit    = history->stats[PRE_WRAPPER_EXIT_TIME];
    pre_wrapper_time    = 0;

    if (pre_wrapper_start !=0 && pre_wrapper_exit == 0)
        pre_wrapper_time = time(NULL) - pre_wrapper_start;
    else
        pre_wrapper_time = pre_wrapper_exit - pre_wrapper_start;

    return(pre_wrapper_time);
}

/*----------------------------------------------------------------------------*/

time_t gw_job_history_get_epilog_time(gw_history_t *history)
{
    time_t epilog_time;
    time_t epilog_start;
    time_t epilog_exit;

    if (history == NULL)
        return -1;

    epilog_start = history->stats[EPILOG_START_TIME];
    epilog_exit  = history->stats[EPILOG_EXIT_TIME];
    epilog_time  = 0;

    if (epilog_start !=0 && epilog_exit == 0)
        epilog_time = time(NULL) - epilog_start;
    else
        epilog_time = epilog_exit -  epilog_start;

    return(epilog_time);
}

/*----------------------------------------------------------------------------*/

time_t gw_job_history_get_migration_time (gw_history_t *history)
{
    time_t migration_time;
    time_t migration_start;
    time_t migration_exit;

    if (history == NULL)
        return -1;

    migration_start = history->stats[MIGRATION_START_TIME];
    migration_exit  = history->stats[MIGRATION_EXIT_TIME];
    migration_time  = 0;

    if (migration_start !=0 && migration_exit == 0)
        migration_time = time(NULL) - migration_start;
    else
        migration_time = migration_exit -  migration_start;

    return(migration_time);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
