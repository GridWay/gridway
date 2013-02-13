/* -------------------------------------------------------------------------- */
/* Copyright 2002-2013, GridWay Project Leads (GridWay.org)                   */
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


#include "gw_log.h"
#include "gw_conf.h"
#include "gw_job_pool.h"
#include "gw_tm_mad.h"
#include "gw_tm.h"
#include "gw_job_template.h"


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int gw_tm_prolog_build_urls(gw_job_t *   job, 
                            char *       src, 
                            char *       dst, 
                            char **      src_url,
                            char **      dst_url)
{
	int   is_gsiftp;
	char  url_buffer[1024];
	char  *tmp;
	
	if ( src[0] == '/' )
	{
		gw_job_print(job,"TM",'W',"\tSkipping file %s, absolute path.\n",src);

		return 1;
	}
			
	is_gsiftp = strstr(src,"gsiftp://") != NULL;
	
	if ( is_gsiftp )
	{
		*src_url = gw_job_substitute (src,job);
		if (*src_url == NULL )
		{
			gw_job_print(job,"TM",'E',"\tSkipping file %s, parse error.\n",src);
			return -1;
		}				
		
		if ( dst == NULL )
		{
			tmp = strrchr(src,'/');
			if ( tmp == NULL )
			{
				gw_job_print(job,"TM",'E',"\tSkipping file %s, no file in url.\n",src);				
				free(*src_url);
				return -1;
			}
			else
				tmp++;
		}
		else
			tmp = dst;
		
		snprintf(url_buffer,sizeof(char)*1024,"%s%s",job->history->rdir,tmp);
		
		*dst_url = gw_job_substitute (url_buffer, job);
		if ( dst_url == NULL )
		{
			gw_job_print(job,"TM",'E',"\tSkipping file %s, parse error.\n",url_buffer);
						
			free(*src_url);
			return -1;
		}				
		
	}
	else
	{
		if ( strstr(src,"file://") != NULL )
		{
			strncpy(url_buffer, src, sizeof(char) * 1024);

			if ( dst == NULL )
			{
			    tmp = strrchr(src,'/');

			    if ( tmp == NULL )
			    {
			    	gw_job_print(job,"TM",'E',"\tSkipping file %s, no file in url.\n",src);
					return -1;
			    }
			    else
					tmp++;
			}
			else
			{
				tmp = dst;
			}
		}
		else
		{
			snprintf(url_buffer,sizeof(char)*1024,"file://%s/%s",job->template.job_home, src);

			if ( dst == NULL )
				tmp = src;
			else
				tmp = dst;
		}

		*src_url = gw_job_substitute (url_buffer, job);		
		if ( *src_url == NULL )
		{
			gw_job_print(job,"TM",'E',"\tSkipping file %s, parse error.\n",url_buffer);				
			return -1;
		}
			 	
		snprintf(url_buffer,sizeof(char)*1024,"%s%s",job->history->rdir,tmp);
		
		*dst_url = gw_job_substitute (url_buffer, job);
		if ( *dst_url == NULL )
		{
			gw_job_print(job,"TM",'E',"\tSkipping file %s, parse error.\n",url_buffer);
			
			free(*src_url);
			return -1;
		}		
	}

	return 0;	
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_tm_prolog_stage_in(gw_job_t * job)
{	
	int          pend_xfrs;
	int          rc;
	char *       src_url;
	char *       dst_url;
    int          i;
	gw_boolean_t failed;	
	    		
	if (job->history == NULL) 
    {
		gw_log_print("TM",'E',"History of job %i doesn't exist (TM_PROLOG_CP).\n",job->id);
		return;
    }			

    gw_job_print(job,"TM",'I',"Staging input files:\n");
    gw_job_print(job,"TM",'I',"\tSource: %s.\n",job->template.job_home);
    
	/* ----------------------------------------------------------- */  
    /* 2.-Start transfers of input files                           */
    /* ----------------------------------------------------------- */  
    
    pend_xfrs = 0;
	
	for (i = 0; i< job->xfrs.number_of_xfrs ; i++)
	{
		if ( job->xfrs.xfrs[i].src_url == NULL )
			continue;
			
		rc = gw_tm_prolog_build_urls(job, 
		             job->xfrs.xfrs[i].src_url, 
		             job->xfrs.xfrs[i].dst_url, 
		             &src_url,
		             &dst_url);
		             
		if ( rc != 0 )
		{			
			job->xfrs.xfrs[i].done = GW_TRUE;
			
			if ( rc == 1 )
		    	job->xfrs.xfrs[i].success = GW_TRUE;
		    else
			    job->xfrs.xfrs[i].success = GW_FALSE;
			    
			continue;
		}
		
		gw_job_print(job,"TM",'I',"\tCopying file %s.\n",
			job->xfrs.xfrs[i].src_url);
		
		gw_tm_mad_cp(job->history->tm_mad, 
		             job->id, 
		             i, 
		             job->xfrs.xfrs[i].mode,
		             src_url, 
		             dst_url);
		             
		free(src_url);
		free(dst_url);
		
		pend_xfrs++;
	}
	
	if ( pend_xfrs == 0 )
	{		
		failed = GW_FALSE;
				
		for ( i = 0 ; i < job->xfrs.failure_limit; i++)
			if ((job->xfrs.xfrs[i].src_url != NULL) &&
				(job->xfrs.xfrs[i].success == GW_FALSE))
			{
				failed = GW_TRUE;
				break;
			}
		
		if ( failed == GW_TRUE )
		{
			gw_job_print(job,"TM",'E',"Some input files were not copied.\n");			
           	gw_job_print(job,"TM",'W',"Removing remote directory:\n");
           	gw_job_print(job,"TM",'W',"\tTarget url: %s.\n",job->history->rdir);
           	
			gw_tm_mad_rmdir(job->history->tm_mad, job->id, job->history->rdir);
				
			job->tm_state = GW_TM_STATE_PROLOG_FAILED;
		}
		else
			gw_tm_mad_end(job->history->tm_mad, job->id);
	}
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_tm_prolog_cp_cb(gw_job_t * job, int cp_xfr_id, gw_boolean_t failure)
{
	gw_boolean_t failed;	
	int          pend_xfrs;
	int          i;
	char *       src_url;
	char *       dst_url;
	
	if (job->history == NULL) 
    {
		gw_log_print("TM",'E',"History of job %i doesn't exist (TM_PROLOG_CP_CB).\n",job->id);
		return;
    }
    
    if ( failure == GW_FALSE )
    {
    	gw_job_print(job,"TM",'I',"\tFile %s copied.\n",
				job->xfrs.xfrs[cp_xfr_id].src_url);
				
    	job->xfrs.xfrs[cp_xfr_id].done    = GW_TRUE;
    	job->xfrs.xfrs[cp_xfr_id].success = GW_TRUE;
    	job->xfrs.xfrs[cp_xfr_id].counter = -1;
    }
    else
    {
		job->xfrs.xfrs[cp_xfr_id].tries--;    	

		if ( job->xfrs.xfrs[cp_xfr_id].tries == 0 )
		{
			if 	(job->xfrs.xfrs[cp_xfr_id].alt_src_url != NULL)
			{
				gw_job_print(job,"TM",'E',"\tCopy of file %s failed, trying alternate file %s.\n",
					job->xfrs.xfrs[cp_xfr_id].src_url,
					job->xfrs.xfrs[cp_xfr_id].alt_src_url);
					
				free(job->xfrs.xfrs[cp_xfr_id].src_url);			
				
				job->xfrs.xfrs[cp_xfr_id].tries       = job->template.number_of_retries;
				job->xfrs.xfrs[cp_xfr_id].src_url     = job->xfrs.xfrs[cp_xfr_id].alt_src_url;
				job->xfrs.xfrs[cp_xfr_id].alt_src_url = NULL;
				job->xfrs.xfrs[cp_xfr_id].counter     =	-1;
				
                gw_tm_prolog_build_urls(job,
		    		job->xfrs.xfrs[cp_xfr_id].src_url, 
			        job->xfrs.xfrs[cp_xfr_id].dst_url, 
			        &src_url,
			        &dst_url);
						
                gw_tm_mad_cp(job->history->tm_mad, 
			        job->id, 
			        cp_xfr_id, 
			        job->xfrs.xfrs[cp_xfr_id].mode,
			        src_url,
			        dst_url);

			    free(src_url);
			    free(dst_url);
			}
			else
			{
		    	job->xfrs.xfrs[cp_xfr_id].done    = GW_TRUE;
		    	job->xfrs.xfrs[cp_xfr_id].success = GW_FALSE;
      	        job->xfrs.xfrs[cp_xfr_id].counter = -1;
      	        
				gw_job_print(job,"TM",'E',"\tCopy of file %s failed.\n",
					job->xfrs.xfrs[cp_xfr_id].src_url);
			}						
		}
		else if (job->xfrs.xfrs[cp_xfr_id].tries > 0)
		{
			job->xfrs.xfrs[cp_xfr_id].counter = job->template.number_of_retries
			    - job->xfrs.xfrs[cp_xfr_id].tries;

			gw_job_print(job,"TM",'I',"\tRetrying copy of file %s in ~%i seconds.\n",
				job->xfrs.xfrs[cp_xfr_id].src_url, 
				job->xfrs.xfrs[cp_xfr_id].counter * GW_TM_TIMER_PERIOD);
		}
    }

	pend_xfrs = gw_xfr_pending (&(job->xfrs));
	
	if ( pend_xfrs == 0 )
	{		
		failed = GW_FALSE;
				
		for ( i = 0 ; i<job->xfrs.failure_limit; i++)
			if ((job->xfrs.xfrs[i].src_url != NULL) &&
				(job->xfrs.xfrs[i].success == GW_FALSE))
			{
				failed = GW_TRUE;
				break;
			}
			
		if ( failed == GW_TRUE )
		{
			gw_job_print(job,"TM",'E',"Some input files were not copied.\n");
           	gw_job_print(job,"TM",'W',"Removing remote directory:\n");
           	gw_job_print(job,"TM",'W',"\tTarget url: %s.\n",job->history->rdir);
           	
			gw_tm_mad_rmdir(job->history->tm_mad, job->id, job->history->rdir);

			job->tm_state = GW_TM_STATE_PROLOG_FAILED;
		}
		else
		{
			gw_job_print(job,"TM",'I',"All input files copied.\n");
								
			gw_tm_mad_end(job->history->tm_mad, job->id);
		}
	}		
}
