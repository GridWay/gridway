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
#include <math.h>
#include "drmaa.h"
#include <unistd.h>

void setup_job_template( drmaa_job_template_t **jt);

int main(int argc, char *argv[])
{
    char                   error[DRMAA_ERROR_STRING_BUFFER];
    int                    rc;
    int                    stat;
    
    drmaa_job_template_t * jt;
    drmaa_attr_values_t  * rusage;
    drmaa_job_ids_t      * jobids;
    
    char 		           value[DRMAA_ATTR_BUFFER];    
    const char *           job_ids[2] ={DRMAA_JOB_IDS_SESSION_ALL,NULL};
    char                   job_id_out[DRMAA_JOBNAME_BUFFER];
    
    int			           rcj;

    /* -------- INIT DRMAA Library ------------*/
    
	rc = drmaa_init (NULL, error, DRMAA_ERROR_STRING_BUFFER-1);

    if ( rc != DRMAA_ERRNO_SUCCESS)
    {
      fprintf(stderr,"drmaa_init() failed: %s\n", error);
      return -1;
    }

    /* -------- INIT DRMAA Library ------------*/
                       
    setup_job_template(&jt);
    
    
   rc = drmaa_set_attribute(jt, 
                            DRMAA_OUTPUT_PATH, 
                            "stdout."DRMAA_PLACEHOLDER_INCR, 
                            error,
                            DRMAA_ERROR_STRING_BUFFER-1);
     
    rc = drmaa_set_attribute(jt, 
                             DRMAA_ERROR_PATH, 
                             "stderr."DRMAA_PLACEHOLDER_INCR, 
                             error,
                             DRMAA_ERROR_STRING_BUFFER-1);


	/* -------- Submit 5 jobs ----- */
	
    rc = drmaa_run_bulk_jobs(&jobids, 
                             jt,
                             0,
                             4,
                             1, 
                             error, 
                             DRMAA_ERROR_STRING_BUFFER-1);
            
    if ( rc != DRMAA_ERRNO_SUCCESS) 
    {
        fprintf(stderr,"drmaa_run_bulk_job() failed: %s\n", error);
        return -1;    
    }
    
    fprintf(stderr,"Bulk job successfully submitted IDs are:\n");
    
    do
    {
        rc = drmaa_get_next_job_id(jobids, value, DRMAA_ATTR_BUFFER-1);
        
        if ( rc == DRMAA_ERRNO_SUCCESS )
            fprintf(stderr,"\t%s\n", value);
        
    }while (rc != DRMAA_ERRNO_NO_MORE_ELEMENTS );
    

    
    
    fprintf(stderr,"Waiting for bulk job to finish...\n");
    
    rc = drmaa_synchronize(job_ids, 
                           DRMAA_TIMEOUT_WAIT_FOREVER, 
                           0, 
                           error,
                           DRMAA_ERROR_STRING_BUFFER-1);
                            
    fprintf(stderr,"All Jobs finished\n");
    
    
    do
    {
        rcj = drmaa_get_next_job_id(jobids, value, DRMAA_ATTR_BUFFER-1);
        
        if ( rcj == DRMAA_ERRNO_SUCCESS )
        {
	        drmaa_wait(value, 
    	                    job_id_out, 
	                        DRMAA_JOBNAME_BUFFER-1, 
	                        &stat, 
	                        DRMAA_TIMEOUT_WAIT_FOREVER, 
	                        &rusage, 
	                        error, 
	                        DRMAA_ERROR_STRING_BUFFER-1);

			drmaa_wexitstatus(&stat,stat,error,DRMAA_ERROR_STRING_BUFFER-1);
		
			fprintf(stderr,"Rusage for task %s (exit code %i)\n", value, stat);
		
			do
			{
	        	rc = drmaa_get_next_attr_value(rusage, value, DRMAA_ATTR_BUFFER-1);
        	
				if ( rc == DRMAA_ERRNO_SUCCESS )
	        		fprintf(stderr,"\t%s\n", value);
			
	    	}while (rc != DRMAA_ERRNO_NO_MORE_ELEMENTS );		   
	    	
	    	drmaa_release_attr_values(rusage);
    	} 
    }while (rcj != DRMAA_ERRNO_NO_MORE_ELEMENTS );
    
    drmaa_release_job_ids(jobids);
    
	/* ---- Finalize ---- */
    
    rc = drmaa_delete_job_template(jt, 
                                   error, 
                                   DRMAA_ERROR_STRING_BUFFER-1);
                                   
    rc = drmaa_exit (error, DRMAA_ERROR_STRING_BUFFER-1);

    if ( rc != DRMAA_ERRNO_SUCCESS)
    {
      fprintf(stderr,"drmaa_exit() failed: %s\n", error);
      return -1;
    }
      
    return 0;      
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

void setup_job_template( drmaa_job_template_t **jt)
{
    char        error[DRMAA_ERROR_STRING_BUFFER];
    int         rc;    
    char        cwd[DRMAA_ATTR_BUFFER];

    const char  *args[3] = {"-l", "-a", NULL};    

    rc = drmaa_allocate_job_template(jt, error, DRMAA_ERROR_STRING_BUFFER);

    if ( rc != DRMAA_ERRNO_SUCCESS) 
    {
        fprintf(stderr,"drmaa_allocate_job_template() failed: %s\n", error);
        exit(-1);    
    }
            
    if ( getcwd(cwd, DRMAA_ATTR_BUFFER) == NULL )
    {
        perror("Error getting current working directory");
        exit(-1);    
    }
    
    /* SHOULD CHECK RC's, REMOVED FOR THE SHAKE OF CLARTIY */
    rc = drmaa_set_attribute(*jt,
                             DRMAA_WD,
                             cwd,
                             error,
                             DRMAA_ERROR_STRING_BUFFER);
    
    rc = drmaa_set_attribute(*jt,
                             DRMAA_JOB_NAME,
                             "ht4",
                             error,
                             DRMAA_ERROR_STRING_BUFFER);

    rc = drmaa_set_attribute(*jt, 
                             DRMAA_REMOTE_COMMAND,
                             "/bin/ls", 
                             error, 
                             DRMAA_ERROR_STRING_BUFFER);
    
    rc = drmaa_set_vector_attribute(*jt, 
                                    DRMAA_V_ARGV, 
                                    args, 
                                    error,
                                    DRMAA_ERROR_STRING_BUFFER);
}


