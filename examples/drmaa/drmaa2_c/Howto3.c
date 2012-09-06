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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "drmaa2.h"
#include <unistd.h>

int main(int argc, char *argv[])
{
    char        cwd[100];                                              
    drmaa2_error error;
    drmaa2_jstate jstate;
    drmaa2_string substate = (drmaa2_string) malloc(50);
    drmaa2_string jid;
    drmaa2_string statestr;

    printf("==== Create a job session with given session name.\n");
    drmaa2_jsession js = drmaa2_create_jsession("mysession", NULL);

    printf("==== Creating the job template.\n");

    drmaa2_jtemplate jt = drmaa2_jtemplate_create();
    jt->jobName = strdup("ht2");
    jt->remoteCommand = strdup("/bin/ls");
    if ( getcwd(cwd, DRMAA2_ATTR_BUFFER) == NULL )
    {   
        perror("Error getting current working directory");
        exit(-1);    
    }   
    jt->workingDirectory = strdup(cwd);
  
    jt->args=drmaa2_list_create(DRMAA2_STRINGLIST,DRMAA2_UNSET_CALLBACK);
    drmaa2_list_add(jt->args,"-l");
    drmaa2_list_add(jt->args,"-a");
    drmaa2_list_add(jt->args,"/tmp");

    jt->outputPath=strdup("stdout."DRMAA2_GW_JOB_ID);
    jt->errorPath =strdup("stderr."DRMAA2_GW_JOB_ID);

//  We hold the job when submitting.
    jt->submitAsHold=DRMAA2_TRUE;


    drmaa2_j j1=NULL;

    printf("==== Submiting the job but hold it first.\n");
    j1 = drmaa2_jsession_run_job(js, jt); 

    jid = drmaa2_j_get_id(j1);
    printf("==== Your job has been submitted with id: %s\n", jid);

    jstate = drmaa2_j_get_state(j1,&substate);
    statestr = drmaa2_gw_strstatus(jstate);
    printf("==== Job DRMAA2 state is: %s\n", statestr);
    printf("==== Job Gridway substate is: %s\n",substate);

    drmaa2_j_list jobs = drmaa2_jsession_get_jobs(js, NULL);
    printf("==== There are %ld jobs in the job list\n", drmaa2_list_size(jobs));
    drmaa2_list_free(&jobs);

//  Now we release the job
    printf("==== We now release the job.\n");
    error = drmaa2_j_release(j1);
    if(error != DRMAA2_SUCCESS)
    {
       printf("==== Releasing job failed!\n");
       return -1;
    }

    jstate = drmaa2_j_get_state(j1,&substate);
    statestr = drmaa2_gw_strstatus(jstate);
    printf("==== Job DRMAA2 state is: %s\n", statestr);
    printf("==== Job Gridway substate is: %s\n",substate);


    printf("==== We now terminate the job.\n");
    error = drmaa2_j_terminate(j1);
    if(error != DRMAA2_SUCCESS)
       printf("==== Terminating the job failed!\n");
    else
       printf("==== Your job has been deleted\n");

//    drmaa2_j_wait_terminated(j1, DRMAA2_INFINITE_TIME);

    printf("==== Destroying job template and job session.\n");
    drmaa2_jtemplate_free(&jt);
    drmaa2_destroy_jsession("mysession");

    printf("==== Exiting now.\n");

    return 0;
    
}


