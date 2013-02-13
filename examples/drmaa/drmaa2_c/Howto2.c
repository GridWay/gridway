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
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "drmaa2.h"
#include <unistd.h>

int main(int argc, char *argv[])
{
    char        cwd[100]; 
    drmaa2_string        jid1, jid2;

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

    drmaa2_j j1=NULL;
    drmaa2_j j2=NULL;

    printf("==== Submiting 2 jobs.\n");
    j1 = drmaa2_jsession_run_job(js, jt); 
    j2 = drmaa2_jsession_run_job(js, jt);
    jid1 = drmaa2_j_get_id(j1);
    jid2 = drmaa2_j_get_id(j2);
    printf("==== Your jobs have been submitted with id: %s and %s\n", jid1, jid2);

    drmaa2_j_list jobs = drmaa2_jsession_get_jobs(js, NULL);
    printf("==== There are %ld jobs in the job list\n", drmaa2_list_size(jobs));
  
    drmaa2_j_wait_terminated(j1, DRMAA2_INFINITE_TIME);
    drmaa2_j_wait_terminated(j2, DRMAA2_INFINITE_TIME); 

    printf("==== Destroying job template and job session.\n");
    drmaa2_list_free(&jobs);
    drmaa2_jtemplate_free(&jt);
    drmaa2_destroy_jsession("mysession");
    drmaa2_jsession_free(&js);

    printf("==== Exiting now.\n");

    return 0;
    
}


