/* -------------------------------------------------------------------------- */
/* Copyright 2002-2011 GridWay Project Leads (GridWay.org)                    */
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

#include "gw_em_mad_cream.h"

#include <iostream>
#include <string>
#include <pthread.h>

void *creamAction(void *thread_data);
void *timer(void *);
int getThreadID();

using namespace std;
	
extern char *optarg;
extern int   optopt;

typedef struct thread_operation_s{
    string action;
    int jidCREAM;
    int threadid; 
    string contact;
    string jdlFile;
} thread_operation_t;

thread_operation_t operation[MAX_THREADS];
CreamEmMad *creamEmMad = NULL;

int main( int argc, char **argv) 
{
    char str[4096];
    char str1[20];
    char str2[20];
    char str3[500];
    char str4[1024];
    bool end=false;
    string action;
    string contact;
    string jdlFile;
    string delegation;
    int refreshTime = 0;
    char opt;
    int paramNum;
    int jidCREAM;
    int i;

    pthread_t creamTimer;
    pthread_t creamOperation[MAX_THREADS];
    pthread_attr_t attr;

    pthread_attr_init(&attr);

    for(i=0;i<MAX_THREADS;i++) 
        operation[i].threadid = -1;

    while((opt = getopt(argc, argv, ":d:t:")) != -1)
        switch(opt)
        {
            case 'd': 
                delegation = optarg;
                break;
            case 't': 
           	refreshTime = atoi(optarg);
		break;
            case '?':
                cout << "error: invalid option " << optopt << endl;
                exit(1);
                break;                
            case ':':
                cout << "error: must provide an argument for option " << optopt << endl;
                exit(1);
                break;
	}

    while (!end)
    { 
        cin.getline(str,4096,'\n');

        paramNum = sscanf(str, "%s %s %s %[^\n]", str1, str2, str3, str4);

        action = str1;
        jidCREAM = atoi(str2);
        contact = str3;
        jdlFile = str4;
        if (paramNum != 4)
        {
            cout << "FAILURE Not all four arguments defined" << endl;
            continue;
        }
        else if (creamEmMad == NULL)
            if (action == "INIT")
            {
                //TODO: delegationID???
		creamEmMad = new CreamEmMad(delegation, refreshTime);
                creamEmMad->init();

                pthread_create(&creamTimer, &attr, timer, NULL);
            }
            else
               cout << action << " " << jidCREAM << " FAILURE Not initialized" << endl;
        else if (action == "INIT")
               cout << action << " " << jidCREAM << " FAILURE Already initialized" << endl;
        else if (action == "FINALIZE")
        {
            end = true;
	    pthread_cancel(creamTimer);
            pthread_attr_destroy(&attr); 
            creamEmMad->finalize();
            delete creamEmMad; 
        }
        else {
	    while ((i=getThreadID()) == -1);
            operation[i].action = action;
            operation[i].jidCREAM = jidCREAM;
	    operation[i].threadid = i;
            operation[i].contact = contact;
            operation[i].jdlFile = jdlFile;
            pthread_create(&creamOperation[i], &attr, creamAction, &operation[i]);
        }
  } 
}

int getThreadID()
{
    int i;

    for (i=0;i<MAX_THREADS;i++)
        if (operation[i].threadid == -1) 
            return i;
    return -1;
}

void *timer(void *)
{
    pthread_detach(pthread_self());
 
    creamEmMad->timer();
    pthread_exit(NULL);
}

void *creamAction(void *thread_data)
{
    thread_operation_t *data;
    data = (thread_operation_t *) thread_data;

    pthread_detach(pthread_self());

    string action =  data->action;
    string contact = data->contact;
    string jdlFile = data->jdlFile;
    string host;
    int jidCREAM = data->jidCREAM;
    int status = -1;

    if (action == "SUBMIT")
    {
        host = contact.substr(0, contact.find("/"));
        status = creamEmMad->submit(jidCREAM, host, jdlFile);
    }
    else if (action == "RECOVER")
    {
        status = creamEmMad->recover(jidCREAM, contact);
    }
    else if (action == "CANCEL")
        status = creamEmMad->cancel(jidCREAM);
    else if (action == "POLL")
        status = creamEmMad->poll(jidCREAM);

    data->threadid = -1;

    pthread_exit(NULL);
}
