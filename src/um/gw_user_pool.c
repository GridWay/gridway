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

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

#include "gw_user_pool.h"
#include "gw_log.h"
#include "gw_conf.h"
#include "gw_dm.h"

/* -------------------------------------------------------------------------- */
gw_user_pool_t gw_user_pool;
/* -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

gw_user_pool_t * gw_user_pool_init()
{
	int i;

	pthread_mutex_init(&(gw_user_pool.mutex),(pthread_mutexattr_t *) NULL);

	pthread_mutex_lock(&(gw_user_pool.mutex));

	gw_user_pool.pool = (gw_user_t**) malloc(gw_conf.number_of_users
		* sizeof(gw_user_t*));
	gw_user_pool.number_of_users = 0;

  	if (gw_user_pool.pool == NULL)
  	{
    	pthread_mutex_unlock(&(gw_user_pool.mutex));
      	pthread_mutex_destroy(&(gw_user_pool.mutex));
      	return NULL;
  	}
  	
  	for ( i=0; i < gw_conf.number_of_users; i++)
    	gw_user_pool.pool[i] = NULL;

   	pthread_mutex_unlock(&(gw_user_pool.mutex));  	
   	
	gw_log_print("UM",'I',"User pool initiated.\n");	
	
	return(&gw_user_pool);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void  gw_user_pool_finalize()
{
	int i;
	
	pthread_mutex_lock(&(gw_user_pool.mutex));

  	for ( i=0; i < gw_conf.number_of_users; i++)
    	if (gw_user_pool.pool[i] != NULL)
      	{	
	        gw_user_pool.number_of_users--;
	        
		gw_user_destroy(gw_user_pool.pool[i]);

	        free(gw_user_pool.pool[i]);
	                
	        gw_user_pool.pool[i] = NULL;
      	}
      	
  	free(gw_user_pool.pool);

  	pthread_mutex_unlock(&(gw_user_pool.mutex));

  	pthread_mutex_destroy(&(gw_user_pool.mutex));
  
  	gw_log_print("UM",'I',"User pool destroyed.\n");
}

/* -------------------------------------------------------------------------- */

void gw_user_pool_set_mad_pipes(int um_em_pipe_w, int um_tm_pipe_w)
{
	pthread_mutex_lock(&(gw_user_pool.mutex));
	
	gw_user_pool.um_em_pipe_w = um_em_pipe_w;
	gw_user_pool.um_tm_pipe_w = um_tm_pipe_w;
	
  	pthread_mutex_unlock(&(gw_user_pool.mutex));	
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int gw_user_pool_user_allocate (const char *user, const char *proxy_path, int *user_id)
{
    int  i=0;
    int  rc;
    char buf = 'A';
	int write_result;
    pthread_mutex_lock(&(gw_user_pool.mutex));
	
    if (gw_user_pool.number_of_users == gw_conf.number_of_users)
    {
        pthread_mutex_unlock(&(gw_user_pool.mutex));
        return -1;
    }	
	
    while (i < gw_conf.number_of_users)
        if ( gw_user_pool.pool[i] == NULL )
            break;
        else
            i = i + 1;
	
    gw_user_pool.pool[i] = (gw_user_t *) malloc (sizeof(gw_user_t));

    rc = gw_user_init(gw_user_pool.pool[i], user, proxy_path);
	
    if ( rc == 0 )
    {
        *user_id = i;
        gw_user_pool.pool[i]->active_jobs = 1;

        write_result = write(gw_user_pool.um_em_pipe_w, &buf, sizeof(char));
        write_result = write(gw_user_pool.um_tm_pipe_w, &buf, sizeof(char));
		
        gw_dm_mad_user_add(&gw_dm.dm_mad[0],
                i,
                gw_user_pool.pool[i]->active_jobs,
                gw_user_pool.pool[i]->running_jobs,
                gw_user_pool.pool[i]->name);
    }
    else
    {
        free(gw_user_pool.pool[i]);
        gw_user_pool.pool[i] = NULL;
    }
	
    pthread_mutex_unlock(&(gw_user_pool.mutex));

    return rc;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_user_pool_user_free (int user_id)
{
	char buf = 'R';	
	int write_result;
	pthread_mutex_lock(&(gw_user_pool.mutex));
		
	if ( ( user_id >= 0 ) && ( user_id < gw_conf.number_of_users ) )
		if ( gw_user_pool.pool[user_id] != NULL )
		{
			gw_user_destroy(gw_user_pool.pool[user_id]);
			free(gw_user_pool.pool[user_id]);
			
			gw_user_pool.pool[user_id] = NULL;
			
			gw_user_pool.number_of_users--;	
			
			write_result = write(gw_user_pool.um_em_pipe_w, &buf, sizeof(char));
			write_result = write(gw_user_pool.um_tm_pipe_w, &buf, sizeof(char));			
		}
		
	pthread_mutex_unlock(&(gw_user_pool.mutex));
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

gw_em_mad_t * gw_user_pool_get_em_mad (int user_id, const char *name)
{
    gw_em_mad_t * em_mad;
	
    em_mad = NULL;

    pthread_mutex_lock(&(gw_user_pool.mutex));

    if ( ( user_id >= 0 ) && ( user_id < gw_conf.number_of_users ) )
        em_mad = gw_em_get_mad(gw_user_pool.pool[user_id], name);

    pthread_mutex_unlock(&(gw_user_pool.mutex));

    return (em_mad);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

gw_tm_mad_t * gw_user_pool_get_tm_mad (int user_id, const char *name)
{
	gw_tm_mad_t * tm_mad;
	
	tm_mad = NULL;

    pthread_mutex_lock(&(gw_user_pool.mutex));

    if ( ( user_id >= 0 ) && ( user_id < gw_conf.number_of_users ) )
		tm_mad = gw_tm_get_mad(gw_user_pool.pool[user_id], name);

    pthread_mutex_unlock(&(gw_user_pool.mutex));

    return (tm_mad);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int gw_user_pool_set_em_pipes (fd_set *       in_pipes,
                               int *          fds, 
                               int *          num_fds, 
                               gw_em_mad_t ** em_mads, 
                               int            um_em_pipe_r)
{
    int greater;
    int i,j;
    int fd;

    pthread_mutex_lock(&(gw_user_pool.mutex));
  	   	
    FD_ZERO(in_pipes);

    FD_SET(um_em_pipe_r, in_pipes);
    greater    = um_em_pipe_r;
    fds[0]     = um_em_pipe_r;
    em_mads[0] = NULL;

    *num_fds = 1;
            
    for (i=0; i<gw_conf.number_of_users; i++)
    {
    	if ( gw_user_pool.pool[i] != NULL )
    	{
            for (j=0; j< gw_user_pool.pool[i]->em_mads; j++)
            {
            	fd = gw_user_pool.pool[i]->em_mad[j].mad_em_pipe;
                
                if ( fd != -1 )
                {
	                em_mads[*num_fds] = &(gw_user_pool.pool[i]->em_mad[j]);
	                fds[*num_fds]     = fd;
	
                    FD_SET(fd, in_pipes);
            
                    if ( fd > greater )
                        greater = fd;
                        
                    *num_fds = *num_fds + 1;                         
                }                
    		}
        }
    }
    
    pthread_mutex_unlock(&(gw_user_pool.mutex));
    
    return greater;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int gw_user_pool_set_tm_pipes (fd_set *       in_pipes, 
                               int *          fds, 
                               int *          num_fds, 
                               gw_tm_mad_t ** tm_mads, 
                               int            um_tm_pipe_r)
{
	int greater;
	int i,j;
	int fd;

    pthread_mutex_lock(&(gw_user_pool.mutex));
  	   	
    FD_ZERO(in_pipes);

    FD_SET(um_tm_pipe_r, in_pipes);
    greater    = um_tm_pipe_r;
    fds[0]     = um_tm_pipe_r;
    tm_mads[0] = NULL;

    *num_fds = 1;
 	
    for (i=0; i<gw_conf.number_of_users; i++)
    {
    	if ( gw_user_pool.pool[i] != NULL )
    	{
		    for (j=0; j< gw_user_pool.pool[i]->tm_mads; j++)
    		{
            	fd = gw_user_pool.pool[i]->tm_mad[j].mad_tm_pipe;
                
                if (fd != -1)
                {            	
                    tm_mads[*num_fds] = &(gw_user_pool.pool[i]->tm_mad[j]);
                    fds[*num_fds]     = fd;
            	 
                    FD_SET(fd, in_pipes);
            
                	if ( fd > greater )
                	   greater = fd;
                	
                    *num_fds = *num_fds + 1;
                }
    		}
        }
    }
    
    pthread_mutex_unlock(&(gw_user_pool.mutex));
    
    return greater;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

gw_boolean_t gw_user_pool_exists (const char *name, const char *proxy_path,
        int *user_id)
{
    gw_boolean_t exists;
    int          i;
	
    pthread_mutex_lock(&(gw_user_pool.mutex));

    exists   = GW_FALSE;
    *user_id = -1;
    
    for (i=0; i<gw_conf.number_of_users; i++)
    {
        if ( gw_user_pool.pool[i] != NULL
    	        && strcmp(gw_user_pool.pool[i]->name, name) == 0
                && strcmp(gw_user_pool.pool[i]->proxy_path, proxy_path) == 0)
        {
            exists   = GW_TRUE;
            *user_id = i;
            break;
        }
    }

    pthread_mutex_unlock(&(gw_user_pool.mutex));
    
    return exists;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_user_pool_inc_jobs(int uid, int jobs)
{
    pthread_mutex_lock(&(gw_user_pool.mutex));

    if ( ( uid >= 0 ) && ( uid < gw_conf.number_of_users ) )
    	if (gw_user_pool.pool[uid] != NULL)
    	{
    		gw_user_pool.pool[uid]->idle = 0;
    		gw_user_pool.pool[uid]->active_jobs += jobs;
    		
    		gw_dm_mad_user_add(&gw_dm.dm_mad[0],
		                       uid,
		                       gw_user_pool.pool[uid]->active_jobs,
		                       gw_user_pool.pool[uid]->running_jobs,
		                       gw_user_pool.pool[uid]->name);    		
    	}
    	
    pthread_mutex_unlock(&(gw_user_pool.mutex));	
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_user_pool_dec_jobs(int uid)
{
    pthread_mutex_lock(&(gw_user_pool.mutex));

    if ( ( uid >= 0 ) && ( uid < gw_conf.number_of_users ) )
    	if (gw_user_pool.pool[uid] != NULL)
    	{
    		gw_user_pool.pool[uid]->idle = 0;
    		gw_user_pool.pool[uid]->active_jobs -= 1;
    		
    		gw_dm_mad_user_add(&gw_dm.dm_mad[0],
		                       uid,
		                       gw_user_pool.pool[uid]->active_jobs,
		                       gw_user_pool.pool[uid]->running_jobs,
		                       gw_user_pool.pool[uid]->name);    		
    	}
    	
    pthread_mutex_unlock(&(gw_user_pool.mutex));	
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_user_pool_check_users(time_t period)
{
    int    i;
    char * name;
	
    pthread_mutex_lock(&(gw_user_pool.mutex));
	
    for (i=0; i<gw_conf.number_of_users; i++)
    	if ( gw_user_pool.pool[i] != NULL )    	
    	{    		   		
    		if ( gw_user_pool.pool[i]->active_jobs == 0 )
    		{
    			gw_user_pool.pool[i]->idle += period;
	    			
				if (gw_user_pool.pool[i]->idle >= GW_MAX_IDLE_TIME)
				{			
#ifdef GWUSERDEBUG					
		   			gw_log_print("UM",'I',"User %s has been idle %is, freeing user resources\n",
		    			gw_user_pool.pool[i]->name,
		    			gw_user_pool.pool[i]->idle);
#endif		    	
					name = strdup(gw_user_pool.pool[i]->name);
					
					gw_user_destroy(gw_user_pool.pool[i]);
					
					free(gw_user_pool.pool[i]);
			
					gw_user_pool.pool[i] = NULL;
			
					gw_user_pool.number_of_users--;	
										
					gw_log_print("UM",'I',"User %s freed.\n",name);
					
					free(name);
					
					gw_dm_mad_user_del(&gw_dm.dm_mad[0],i);
				}
    		}
        }	
	
	pthread_mutex_unlock(&(gw_user_pool.mutex));
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_user_pool_inc_running_jobs(int uid, int jobs)
{
    pthread_mutex_lock(&(gw_user_pool.mutex));

    if ( ( uid >= 0 ) && ( uid < gw_conf.number_of_users ) )
    	if (gw_user_pool.pool[uid] != NULL)
    	{
    		gw_user_pool.pool[uid]->running_jobs += jobs;
    		
    		gw_dm_mad_user_add(&gw_dm.dm_mad[0],
		                       uid,
		                       gw_user_pool.pool[uid]->active_jobs,
		                       gw_user_pool.pool[uid]->running_jobs,
		                       gw_user_pool.pool[uid]->name);     		
    	}
    	
    pthread_mutex_unlock(&(gw_user_pool.mutex));	
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_user_pool_dec_running_jobs(int uid)
{
    pthread_mutex_lock(&(gw_user_pool.mutex));

    if ( ( uid >= 0 ) && ( uid < gw_conf.number_of_users ) )
    	if (gw_user_pool.pool[uid] != NULL)
    	{
    		gw_user_pool.pool[uid]->running_jobs -= 1;
    		
    		gw_dm_mad_user_add(&gw_dm.dm_mad[0],
		                       uid,
		                       gw_user_pool.pool[uid]->active_jobs,
		                       gw_user_pool.pool[uid]->running_jobs,
		                       gw_user_pool.pool[uid]->name);     		
    	}
    	
    pthread_mutex_unlock(&(gw_user_pool.mutex));	
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

gw_boolean_t gw_user_pool_get_info(int uid, gw_msg_user_t *msg)
{
	int j;
	
	pthread_mutex_lock(&(gw_user_pool.mutex));
	
	if (gw_user_pool.pool[uid] == NULL)
	{
		pthread_mutex_unlock(&(gw_user_pool.mutex));
		
		return GW_FALSE;
	}
	    		
    msg->user_id  = uid;
    gw_rm_copy_str_short(gw_user_pool.pool[uid]->name,msg->name);
    gw_rm_copy_str_long(gw_user_pool.pool[uid]->dn,msg->dn);
            
    msg->active_jobs  = gw_user_pool.pool[uid]->active_jobs;
    msg->running_jobs = gw_user_pool.pool[uid]->running_jobs;
    msg->idle         = gw_user_pool.pool[uid]->idle;
    
    msg->num_ems      = gw_user_pool.pool[uid]->em_mads;
    msg->num_tms      = gw_user_pool.pool[uid]->tm_mads;
    		
    for (j=0;j<msg->num_ems;j++)
    {
    	msg->em_pid[j] = gw_user_pool.pool[uid]->em_mad[j].pid;
   	
    	gw_rm_copy_str_short(gw_user_pool.pool[uid]->em_mad[j].name,
    				         msg->em_name[j]);
    }

    for (j=0;j<msg->num_tms;j++)
    {
    	msg->tm_pid[j] = gw_user_pool.pool[uid]->tm_mad[j].pid;
    	
    	gw_rm_copy_str_short(gw_user_pool.pool[uid]->tm_mad[j].name,
    				         msg->tm_name[j]);
    }
    	
	pthread_mutex_unlock(&(gw_user_pool.mutex));	
	
	return GW_TRUE;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_user_pool_dm_recover (gw_dm_mad_t * dm_mad)
{
    gw_user_t * user;
    int         i;
    
    pthread_mutex_lock(&(gw_user_pool.mutex));

    for (i=0; i<gw_conf.number_of_users; i++)
    {
        user = gw_user_pool.pool[i];
        
        if ( user != NULL )
        {

#ifdef GWDMDEBUG
            gw_log_print("DM",'D',"Recovering (sched) user %i.\n",i);
#endif                       
            gw_dm_mad_user_add(dm_mad,
                               i,
                               user->active_jobs,
                               user->running_jobs,
                               user->name);     
        }
    }
    
    pthread_mutex_unlock(&(gw_user_pool.mutex));
    
    return;
}
