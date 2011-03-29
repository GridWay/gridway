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

void print_job_status(char *id)
{
	int  rc;
    char error[DRMAA_ERROR_STRING_BUFFER];
    int  status;

    /* -------- Check job state ------------*/

	rc = drmaa_job_ps(id, &status, error, DRMAA_ERROR_STRING_BUFFER-1);

    if ( rc != DRMAA_ERRNO_SUCCESS)
    {
        fprintf(stderr,"drmaa_job_ps() failed: %s\n", error);
        exit(-1);
    }

    /***********************************************************/
    /* drmaa_gw_strstatus is not a DRMAA 1.0 function          */
    /* it is only provided by Gridway                          */
 	  /***********************************************************/

    fprintf(stdout,"Job state is: %s\n",drmaa_gw_strstatus(status));
}


int main(int argc, char *argv[])
{
    char                   error[DRMAA_ERROR_STRING_BUFFER];
    int                    rc;
    drmaa_job_template_t * jt;
    char                   job_id[DRMAA_JOBNAME_BUFFER];
    const char             *job_ids[2]={DRMAA_JOB_IDS_SESSION_ALL,NULL};


    /* -------- INIT DRMAA Library ------------*/

	rc = drmaa_init (NULL, error, DRMAA_ERROR_STRING_BUFFER-1);

    if ( rc != DRMAA_ERRNO_SUCCESS)
    {
      fprintf(stderr,"drmaa_init() failed: %s\n", error);
      return -1;
    }

    /* -------- Set Up job template ------------*/

    setup_job_template(&jt);

    /* -------- Set Up job submission state ------------*/

    rc = drmaa_set_attribute(jt,
                             DRMAA_JS_STATE,
                             DRMAA_SUBMISSION_STATE_HOLD,
                             error,
                             DRMAA_ERROR_STRING_BUFFER-1);

    /* -------- Submit the job ------------*/

    rc = drmaa_run_job(job_id,
                       DRMAA_JOBNAME_BUFFER-1,
                       jt,
                       error,
          			   DRMAA_ERROR_STRING_BUFFER-1);

    if ( rc != DRMAA_ERRNO_SUCCESS)
    {
        fprintf(stderr,"drmaa_run_job() failed: %s\n", error);
        return -1;
    }

    fprintf(stdout,"Your job has been submitted with id: %s\n", job_id);

    /* -------- Check job state -----------*/

    sleep(5);

	print_job_status(job_id);

	sleep(1);

    /* -------- Control op. ------------*/

    fprintf(stdout,"Releasing the Job\n");

    rc = drmaa_control(job_id,
                       DRMAA_CONTROL_RELEASE,
                       error,
                       DRMAA_ERROR_STRING_BUFFER-1);

    if ( rc != DRMAA_ERRNO_SUCCESS)
    {
        fprintf(stderr,"drmaa_control() failed: %s\n", error);
        return -1;
    }

    /* -------- Check job state again ------------*/

	print_job_status(job_id);

    /* -------- synchronize() ------------*/

    fprintf(stdout,"Synchronizing with job...\n");

    rc = drmaa_synchronize(job_ids,
                           DRMAA_TIMEOUT_WAIT_FOREVER,
                           0,
	                       error,
    	      			   DRMAA_ERROR_STRING_BUFFER-1);

    if ( rc != DRMAA_ERRNO_SUCCESS)
    {
        fprintf(stderr,"drmaa_synchronize failed: %s\n", error);
        return -1;
    }

	/* -------- Control op. -------------------------------------*/
	/* You can kill the job using dispose=1 in the previous call */
	/* You could also use drmaa_wait to get rusage and remove job*/
	/*-----------------------------------------------------------*/

    fprintf(stdout,"Killing the Job\n");

    rc = drmaa_control(job_id,
                       DRMAA_CONTROL_TERMINATE,
                       error,
                       DRMAA_ERROR_STRING_BUFFER-1);

    if ( rc != DRMAA_ERRNO_SUCCESS)
    {
        fprintf(stderr,"drmaa_control() failed: %s\n", error);
        return -1;
    }

    fprintf(stdout,"Your job has been deleted\n");

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
                             "ht3",
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

    rc = drmaa_set_attribute(*jt,
                             DRMAA_OUTPUT_PATH,
                             "stdout."DRMAA_GW_JOB_ID,
                             error,
                             DRMAA_ERROR_STRING_BUFFER);

    rc = drmaa_set_attribute(*jt,
                             DRMAA_ERROR_PATH,
                             "stderr."DRMAA_GW_JOB_ID,
                             error,
                             DRMAA_ERROR_STRING_BUFFER);
}
