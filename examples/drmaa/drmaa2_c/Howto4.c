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
    int         i,j;
    char        cwd[100];                                              
    drmaa2_error error;
    drmaa2_jstate jstate;
    drmaa2_string substate = (char*) malloc(50);
    drmaa2_string  jid;
    drmaa2_string statestr;


    printf("==== Create a job session with given session name.\n");
    drmaa2_jsession js = drmaa2_create_jsession("mysession", NULL);

    // To test the random session name generation.
    printf("==== We create another 2 job sessions with random session name generation.\n");
    drmaa2_jsession js2 = drmaa2_create_jsession(NULL, NULL);
    drmaa2_jsession js3 = drmaa2_create_jsession(NULL, NULL);

    drmaa2_string_list session_names = DRMAA2_UNSET_LIST;
    session_names = (drmaa2_string_list) drmaa2_get_jsession_names();

    printf("==== There are %ld job sessions altogether.\n", drmaa2_list_size(session_names));
    for(i=0;i<drmaa2_list_size(session_names);i++)
    {
       printf("=== Session name: %s\n", (char*) drmaa2_list_get(session_names,i));
    }

    // Delete this two job session as we do not use them anymore.
    printf("==== Deleting the last two job sessions as we do not use them anymore.\n\n");
    drmaa2_destroy_jsession((char*) js2->name);
    drmaa2_destroy_jsession((char*) js3->name);


    printf("==== Creating the job template.\n");
    drmaa2_jtemplate jt = drmaa2_jtemplate_create();
    drmaa2_jtemplate jt2 = drmaa2_jtemplate_create();
    jt->jobName = strdup("ht4");
    jt->remoteCommand = strdup("/bin/ls");

    jt2->jobName = strdup("ht5");
    jt2->remoteCommand = strdup("/bin/ls");

    if ( getcwd(cwd, DRMAA2_ATTR_BUFFER) == NULL )
    {   
        perror("Error getting current working directory");
        exit(-1);    
    }   
    jt->workingDirectory = strdup(cwd);
    jt2->workingDirectory = strdup(cwd);
  
    jt->args=drmaa2_list_create(DRMAA2_STRINGLIST,DRMAA2_UNSET_CALLBACK);
    drmaa2_list_add(jt->args,"-l");
    drmaa2_list_add(jt->args,"-a");
    drmaa2_list_add(jt->args,"/tmp");

    jt2->args=drmaa2_list_create(DRMAA2_STRINGLIST,DRMAA2_UNSET_CALLBACK);
    drmaa2_list_add(jt2->args,"-l");
    drmaa2_list_add(jt2->args,"-a");
    drmaa2_list_add(jt2->args,".");

    jt->outputPath=strdup("stdout."DRMAA2_GW_JOB_ID);
    jt->errorPath =strdup("stderr."DRMAA2_GW_JOB_ID);

//    jt->outputPath=strdup("stdout."DRMAA2_PARAMETRIC_INDEX);
//    jt->errorPath =strdup("stderr."DRMAA2_PARAMETRIC_INDEX);

    jt2->outputPath=strdup("stdout."DRMAA2_GW_JOB_ID);
    jt2->errorPath =strdup("stderr."DRMAA2_GW_JOB_ID);

//  We hold the job when submitting.
    jt->submitAsHold=DRMAA2_TRUE;
    jt2->submitAsHold=DRMAA2_TRUE;

    drmaa2_jarray jarray1= (drmaa2_jarray) malloc(sizeof(drmaa2_jarray_s));
    drmaa2_jarray jarray2= (drmaa2_jarray) malloc(sizeof(drmaa2_jarray_s));

    printf("==== Submiting the bulk jobs but hold them first.\n");
    jarray1 = drmaa2_jsession_run_bulk_jobs(js, jt, 0, 1, 1, 1); 
    printf("==== jarray id is: %s\n", jarray1->jarray_id);

    jarray2 = drmaa2_jsession_run_bulk_jobs(js, jt2, 0, 1, 1, 1); 
    printf("==== jarray id is: %s\n", jarray2->jarray_id);

    jarray1 = drmaa2_jsession_get_job_array(js, jarray1->jarray_id);
    jarray2 = drmaa2_jsession_get_job_array(js, jarray2->jarray_id);

    if(jarray1 == NULL)
    {
       printf("==== Job array id not found!\n");
       return (-1);
    }
    else
       printf("==== There are %ld jobs in jarray %s\n", drmaa2_list_size(jarray1->jobs), jarray1->jarray_id);

    if(jarray2 == NULL)
    {
       printf("==== Job array id not found!\n");
       return (-1);
    }
    else
       printf("==== There are %ld jobs in jarray %s\n\n", drmaa2_list_size(jarray2->jobs), jarray2->jarray_id);


    drmaa2_j job = (drmaa2_j) malloc(sizeof(drmaa2_j_s));
    for(i=0;i<drmaa2_list_size(jarray1->jobs);i++)
    {
       job = (drmaa2_j) drmaa2_list_get(jarray1->jobs,i);
       jid = drmaa2_j_get_id(job);
       printf("==== Your job has been submitted with id: %s\n", jid);
       jstate = drmaa2_j_get_state(job,&substate);
       statestr = drmaa2_gw_strstatus(jstate);
       printf("\t Job DRMAA2 state is: %s\n", statestr);
       printf("\t Job Gridway substate is: %s\n",substate);
    }

    for(i=0;i<drmaa2_list_size(jarray2->jobs);i++)
    {
       job = (drmaa2_j) drmaa2_list_get(jarray2->jobs,i);
       jid = drmaa2_j_get_id(job);
       printf("==== Your job has been submitted with id: %s\n", jid);
       jstate = drmaa2_j_get_state(job,&substate);
       statestr = drmaa2_gw_strstatus(jstate);
       printf("\t Job DRMAA2 state is: %s\n", statestr);
       printf("\t Job Gridway substate is: %s\n",substate);
    }



//======================================================================
    drmaa2_j_list jobs = NULL; 
    printf("\n==== There are %ld job arrays in the job session\n", drmaa2_list_size(js->jarray_list));
    jobs = drmaa2_jsession_get_jobs(js, NULL);
    printf("==== There are %ld jobs in the job session\n\n", drmaa2_list_size(jobs));
    drmaa2_list_free(&jobs);

//  Now we release the job
    printf("==== Now we release the jobs:\n");
   
    error = drmaa2_jarray_release(jarray1);
    if(error != DRMAA2_SUCCESS)
    {
      printf("==== Releasing job array failed!\n");
      return (-1);
    }
    error = drmaa2_jarray_release(jarray2);
    if(error != DRMAA2_SUCCESS)
    {
      printf("==== Releasing job array failed!\n");
      return (-1);
    }


    drmaa2_jinfo jinfo = (drmaa2_jinfo) malloc(sizeof(drmaa2_jinfo_s));
    drmaa2_string_list allocatedMachines = drmaa2_list_create(DRMAA2_STRINGLIST, DRMAA2_UNSET_CALLBACK);
    for(i=0;i<drmaa2_list_size(jarray1->jobs);i++)
    {
       job   = (drmaa2_j) drmaa2_list_get(jarray1->jobs,i);
       jid = drmaa2_j_get_id(job);
       jstate = drmaa2_j_get_state(job,&substate);
       statestr = drmaa2_gw_strstatus(jstate);
       printf("    Job %s released.\n", jid);
       printf("    Job DRMAA2 state is: %s\n", statestr);
       printf("    Job Gridway substate is: %s\n",substate);
       printf("    Wait for job %s to finish.\n", jid);

       printf("    Info about the job %s\n", jid);
       drmaa2_j_wait_terminated(job, DRMAA2_INFINITE_TIME);
       jinfo = drmaa2_j_get_info(job);
       allocatedMachines = jinfo->allocatedMachines;
      
       printf("\tjob->jobId=%s\n", jinfo->jobId);
       printf("\tjob->exitStatus=%d\n", jinfo->exitStatus);
       printf("\tjob->jobOwner=%s\n", jinfo->jobOwner);
       printf("\tjob->submissionMachine=%s\n", jinfo->submissionMachine);
       for(j=0;j<drmaa2_list_size(allocatedMachines);j++)
       {
           printf("\tjob->allocatedMachines=%s\n", (char*) drmaa2_list_get(allocatedMachines,j));
       }
       printf("\tjob->queueName=%s\n", jinfo->queueName);
       printf("\tjob->wallclockTime=%lld\n", (long long)jinfo->wallclockTime);
       printf("\tjob->cpuTime=%lld\n", jinfo->cpuTime);
       printf("\tjob->submissionTime=%lld\n", (long long)jinfo->submissionTime);
       printf("\tjob->dispatchTime=%lld\n", (long long)jinfo->dispatchTime);
       printf("\tjob->finishTime=%lld\n", (long long)jinfo->finishTime);
       printf("\n"); 
    }
    drmaa2_jinfo_free(&jinfo);

    jinfo = (drmaa2_jinfo) malloc(sizeof(drmaa2_jinfo_s));
    allocatedMachines = drmaa2_list_create(DRMAA2_STRINGLIST, DRMAA2_UNSET_CALLBACK);
    for(i=0;i<drmaa2_list_size(jarray2->jobs);i++)
    {
       job   = (drmaa2_j) drmaa2_list_get(jarray2->jobs,i);
       jid = drmaa2_j_get_id(job);
       jstate = drmaa2_j_get_state(job,&substate);
       statestr = drmaa2_gw_strstatus(jstate);
       printf("    Job %s released.\n", jid);
       printf("    Job DRMAA2 state is: %s\n", statestr);
       printf("    Job Gridway substate is: %s\n",substate);
       printf("    Wait for job %s to finish.\n", jid);

       printf("    Info about the job %s\n", jid);
       drmaa2_j_wait_terminated(job, DRMAA2_INFINITE_TIME);
       jinfo = drmaa2_j_get_info(job);
       allocatedMachines = jinfo->allocatedMachines;
      
       printf("\tjob->jobId=%s\n", jinfo->jobId);
       printf("\tjob->exitStatus=%d\n", jinfo->exitStatus);
       printf("\tjob->jobOwner=%s\n", jinfo->jobOwner);
       printf("\tjob->submissionMachine=%s\n", jinfo->submissionMachine);
       for(j=0;j<drmaa2_list_size(allocatedMachines);j++)
       {
           printf("\tjob->allocatedMachines=%s\n", (char*) drmaa2_list_get(allocatedMachines,j));
       }
       printf("\tjob->queueName=%s\n", jinfo->queueName);
       printf("\tjob->wallclockTime=%lld\n", (long long)jinfo->wallclockTime);
       printf("\tjob->cpuTime=%lld\n", jinfo->cpuTime);
       printf("\tjob->submissionTime=%lld\n", (long long)jinfo->submissionTime);
       printf("\tjob->dispatchTime=%lld\n", (long long)jinfo->dispatchTime);
       printf("\tjob->finishTime=%lld\n", (long long)jinfo->finishTime);
       printf("\n"); 
    }

//======================================================================
//  Test Monitoring session
    printf("\n==== Testing monitoring sessions.\n");

//  To get all jobs
    struct passwd *pw_ent;
    pw_ent = getpwuid(getuid());

    drmaa2_jinfo filter = NULL;
    filter = drmaa2_jinfo_create();
//    filter->jobId = strdup("3");
    filter->jobOwner = strdup(pw_ent->pw_name);

    drmaa2_msession ms = drmaa2_open_msession("msession1");
    jobs = drmaa2_msession_get_all_jobs(ms, filter);
    printf("==== There are %ld jobs found.\n", drmaa2_list_size(jobs));
    drmaa2_list_free(&jobs);

//  To get all machines
    drmaa2_machineinfo_list mlist = NULL;
    drmaa2_string_list mnames = DRMAA2_UNSET_LIST;
    mlist = drmaa2_msession_get_all_machines(ms, mnames);
    printf("\n=== There are %ld machines found.\n", drmaa2_list_size(mlist));
    for(i=0; i<drmaa2_list_size(mlist);i++)
    {   
       drmaa2_machineinfo minfo= (drmaa2_machineinfo) malloc(sizeof(drmaa2_machineinfo_s));
       minfo = (drmaa2_machineinfo) drmaa2_list_get(mlist,i);
       printf("     minfo->name=%s\n", minfo->name);
       printf("     minfo->available=%d\n", minfo->available);
       printf("     minfo->physMemory=%lld\n", minfo->physMemory);
       printf("     minfo->virtMemory=%lld\n", minfo->virtMemory);
       printf("     minfo->machineOS=%d\n", minfo->machineOS);
       printf("     minfo->machineArch=%d\n", minfo->machineArch);
       printf("     minfo->machineOSVersion->major=%s\n", minfo->machineOSVersion->major);
       printf("     minfo->machineOSVersion->minor=%s\n", minfo->machineOSVersion->minor);
    }   
    

//  To get all queues
    drmaa2_queueinfo_list qlist = NULL;
    drmaa2_string_list qnames = DRMAA2_UNSET_LIST;

    qlist = drmaa2_msession_get_all_queues(ms, qnames); 
    printf("\n==== There are %ld queues available\n", drmaa2_list_size(qlist));
    for(i=0; i<drmaa2_list_size(qlist);i++)
    {
       drmaa2_queueinfo qinfo= (drmaa2_queueinfo) malloc(sizeof(drmaa2_queueinfo_s));
       qinfo = (drmaa2_queueinfo) drmaa2_list_get(qlist,i);
       printf("==== Found queue with name=%s\n", qinfo->name);
    }

    printf("\n==== Destroying job template and job session.\n");
    drmaa2_jtemplate_free(&jt);
    drmaa2_close_msession(ms);
    drmaa2_msession_free(&ms);
    drmaa2_destroy_jsession("mysession");
    drmaa2_jsession_free(&js);
    drmaa2_jsession_free(&js2);
    drmaa2_jsession_free(&js3);

    printf("==== Exiting now!\n");
    return 0;
    
}


