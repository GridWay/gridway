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


#include "gw_log.h"
#include "gw_conf.h"
#include "gw_job_pool.h"
#include "gw_tm_mad.h"
#include "gw_tm.h"
#include "gw_job_template.h"


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_tm_checkpoint_cp(gw_job_t * job)
{	
	
    int    i, num_xfrs, rc;
	char   *src_url;
	char   *dst_url;
	char   url[512];
	int    pend_xfrs;

	if (job->history == NULL) 
    {
		gw_log_print("TM",'E',"History of job %i doesn't exist (TM_CHECKPOINT).\n",
			job->id);
		return;
    }			
	
	/* ----------------------------------------------------------- */  
    /* 1.- build xfr arrary for this host and state                */
    /* ----------------------------------------------------------- */ 
    
    num_xfrs = job->template.num_restart_files;    

	gw_xfr_destroy (&(job->chk_xfrs));
	gw_xfr_init    (&(job->chk_xfrs), num_xfrs, job->template.number_of_retries);
	    	    
	for (i = 0; i< num_xfrs ; i++)
	{			
		job->chk_xfrs.xfrs[i].src_url =
                strdup(job->template.restart_files[i]);
			
		snprintf(url, sizeof(char)*512, "%s/%s",
				job->template.checkpoint_url,
				job->template.restart_files[i]);
					
		job->chk_xfrs.xfrs[i].dst_url     = strdup(url);
		job->chk_xfrs.xfrs[i].alt_src_url = NULL;
	}	
	
	gw_job_print(job,"TM",'I',"Staging restart files:\n");
	gw_job_print(job,"TM",'I',"\tSource      : %s\n",job->history->rdir);
	gw_job_print(job,"TM",'I',"\tDestination : %s\n",job->template.checkpoint_url);	

	/* ----------------------------------------------------------- */  
    /* 1.-Start transfers of restart files                         */
    /* ----------------------------------------------------------- */  
    
   	for (i = 0; i< num_xfrs ; i++)
	{
		if ( job->chk_xfrs.xfrs[i].src_url == NULL )
			continue;
			
		rc = gw_tm_epilog_build_urls(job,
		             job->chk_xfrs.xfrs[i].src_url,
		             job->chk_xfrs.xfrs[i].dst_url,
		             &src_url,
		             &dst_url);
		             
		if ( rc != 0 )
		{
			job->chk_xfrs.xfrs[i].done    = GW_TRUE;
	    	job->chk_xfrs.xfrs[i].success = GW_FALSE;
			continue;
		}
							
		gw_job_print(job,"TM",'I',"\tCopying file %s.\n",
			    job->chk_xfrs.xfrs[i].src_url);
		
		gw_tm_mad_cp(job->history->tm_mad, job->id, i, '-', src_url, dst_url);
		
		free(src_url);
		free(dst_url);
	}
	
	pend_xfrs = gw_xfr_pending (&(job->chk_xfrs));
	
	if ( pend_xfrs == 0 )
	{
		gw_job_print(job,"TM",'W',"Some checkpoint files were not copied.\n");
			
		if ((job->job_state == GW_JOB_STATE_STOP_EPILOG ) ||
			(job->job_state == GW_JOB_STATE_EPILOG_RESTART))
		{
			job->tm_state = GW_TM_STATE_EPILOG;			
			gw_tm_epilog_stage_out(job);
		}
		else
		{
			job->tm_state = GW_TM_STATE_INIT;
			gw_tm_mad_end(job->history->tm_mad, job->id);			
		}
	}		
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_tm_checkpoint_cp_cb(gw_job_t * job, int cp_xfr_id, gw_boolean_t failure)
{
	char *       src_url;
	char *       dst_url;
	int          pend_xfrs;
		
	if (job->history == NULL) 
    {
		gw_log_print("TM",'E',"History of job %i doesn't exist (TM_CHECKPOINT_CB).\n",
				job->id);
		return;
    }
    
    if ( failure == GW_FALSE )
    {
    	gw_job_print(job,"TM",'I',"\tFile %s copied.\n",
				job->chk_xfrs.xfrs[cp_xfr_id].src_url);
				 
    	job->chk_xfrs.xfrs[cp_xfr_id].done    = GW_TRUE;
    	job->chk_xfrs.xfrs[cp_xfr_id].success = GW_TRUE;
    }
	else
	{
		job->chk_xfrs.xfrs[cp_xfr_id].tries--;
	
		if ( job->chk_xfrs.xfrs[cp_xfr_id].tries == 0 )
		{
			gw_job_print(job,"TM",'E',"\tCopy of file %s failed.\n",
				job->chk_xfrs.xfrs[cp_xfr_id].src_url);
			
    		job->chk_xfrs.xfrs[cp_xfr_id].done    = GW_TRUE;
	    	job->chk_xfrs.xfrs[cp_xfr_id].success = GW_FALSE;
		}
	    else
	    {
			gw_tm_epilog_build_urls(job, 
		         job->chk_xfrs.xfrs[cp_xfr_id].src_url, 
		         job->chk_xfrs.xfrs[cp_xfr_id].dst_url, 
		         &src_url,
		         &dst_url);  
    
			gw_job_print(job,"TM",'I',"\tRetrying copy of file %s.\n",
				job->chk_xfrs.xfrs[cp_xfr_id].src_url);
		
			gw_tm_mad_cp(job->history->tm_mad, 
		             job->id, 
		             cp_xfr_id,
		             '-', 
		             src_url, 
		             dst_url);    	
		
			free(src_url);
			free(dst_url);
			
			return;
		}
	}	
	
	pend_xfrs = gw_xfr_pending (&(job->chk_xfrs));;
		
	if ( pend_xfrs == 0 )
	{	
		gw_job_print(job,"TM",'I',"All Checkpoint files copied.\n");
			
		if ((job->job_state == GW_JOB_STATE_STOP_EPILOG ) ||
			(job->job_state == GW_JOB_STATE_EPILOG_RESTART))
		{
			job->tm_state = GW_TM_STATE_EPILOG;			
			gw_tm_epilog_stage_out(job);
		}
		else
		{
			job->tm_state = GW_TM_STATE_INIT;
			gw_tm_mad_end(job->history->tm_mad, job->id);			
		}
	}
}
