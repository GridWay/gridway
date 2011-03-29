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
#include <string.h>
#include <math.h>
#include "drmaa.h"
#include <unistd.h>

void setup_job_template( drmaa_job_template_t **jt);

int main(int argc, char *argv[])
{
    char                   error[DRMAA_ERROR_STRING_BUFFER];
    int                    result;
    drmaa_job_template_t * jt;
    char                   job_id[DRMAA_JOBNAME_BUFFER];
    char                   job_id_out[DRMAA_JOBNAME_BUFFER];    
    drmaa_attr_values_t *  rusage;
    int                    stat;    
    char                   attr_value[DRMAA_ATTR_BUFFER];
                                              
    result = drmaa_init (NULL, error, DRMAA_ERROR_STRING_BUFFER-1);

    if ( result != DRMAA_ERRNO_SUCCESS)
    {
      fprintf(stderr,"drmaa_init() failed: %s\n", error);
      return -1;
    }
    else
      printf("drmaa_init() success \n");
      
          
    setup_job_template(&jt);

    result = drmaa_run_job(job_id, 
                           DRMAA_JOBNAME_BUFFER-1, 
                           jt, 
                           error, 
       			   DRMAA_ERROR_STRING_BUFFER-1);
            			               
    if ( result != DRMAA_ERRNO_SUCCESS) 
    {
        fprintf(stderr,"drmaa_run_job() failed: %s\n", error);
        return -1;    
    }
    
    fprintf(stderr,"Job successfully submitted ID: %s\n",job_id);
        
	result = drmaa_wait(job_id,
	                    job_id_out, 
	                    DRMAA_JOBNAME_BUFFER-1, 
                        &stat, 
                        DRMAA_TIMEOUT_WAIT_FOREVER, 
                        &rusage, 
                        error, 
                        DRMAA_ERROR_STRING_BUFFER-1);

    if ( result != DRMAA_ERRNO_SUCCESS) 
    {
        fprintf(stderr,"drmaa_wait() failed: %s\n", error);
        return -1;    
    }
    
    drmaa_wexitstatus(&stat,stat,error,DRMAA_ERROR_STRING_BUFFER);
    
    fprintf(stderr,"Job finished with exit code %i, usage: %s\n",stat,job_id);

	while ( drmaa_get_next_attr_value(rusage,attr_value,DRMAA_ATTR_BUFFER-1) !=
			DRMAA_ERRNO_NO_MORE_ELEMENTS)
		fprintf(stderr,"\t%s\n",attr_value);		
    
    drmaa_release_attr_values (rusage);    
	    
	/* ---- Finalize ---- */
    
    result = drmaa_delete_job_template(jt, 
                                   error, 
                                   DRMAA_ERROR_STRING_BUFFER-1);

    if ( result != DRMAA_ERRNO_SUCCESS)
    {
      fprintf(stderr,"drmaa_delete_job_template() failed: %s\n", error);
      return -1;
    }
                                       
    result = drmaa_exit (error, DRMAA_ERROR_STRING_BUFFER-1);

    if ( result != DRMAA_ERRNO_SUCCESS)
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
    
    rc = drmaa_set_attribute(*jt,
                             DRMAA_WD,
                             cwd,
                             error,
                             DRMAA_ERROR_STRING_BUFFER);
 
    if ( rc != DRMAA_ERRNO_SUCCESS )
    {
        fprintf(stderr,"Error setting job template attribute: %s\n",error);
        exit(-1);    
    }    
    
    rc = drmaa_set_attribute(*jt,
                             DRMAA_JOB_NAME,
                             "ht2",
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

    if ( rc != DRMAA_ERRNO_SUCCESS )
    {
        fprintf(stderr,"Error setting remote command arguments: %s\n",error);
        exit(-1);    
    }
    
    rc = drmaa_set_attribute(*jt, 
                             DRMAA_OUTPUT_PATH, 
                             ":stdout."DRMAA_GW_JOB_ID, 
                             error,
                             DRMAA_ERROR_STRING_BUFFER);
     
    rc = drmaa_set_attribute(*jt, 
                             DRMAA_ERROR_PATH, 
                             ":stderr."DRMAA_GW_JOB_ID, 
                             error,
                             DRMAA_ERROR_STRING_BUFFER);
}


