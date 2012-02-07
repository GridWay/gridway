/* -------------------------------------------------------------------------- */
/* Copyright 2002-2012 GridWay Project Leads (GridWay.org)                    */
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
int getFreeThread();

using namespace std;
	
extern char *optarg;
extern int   optopt;

typedef struct thread_operation_s{
    string action;
    int jidCREAM;
    string contact;
    string jdlFile;
    pthread_mutex_t mutex;
    bool free;
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
    bool end = false;
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
    { 
        pthread_mutex_init(&(operation[i].mutex), 0);
        operation[i].free = true;
    }

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
            if (action.compare("INIT") == 0)
            {
		creamEmMad = new CreamEmMad(delegation, refreshTime);
                creamEmMad->init();

                pthread_create(&creamTimer, &attr, timer, NULL);
            }
            else
               cout << action << " " << jidCREAM << " FAILURE Not initialized" << endl;
        else if (action.compare("INIT") == 0)
               cout << action << " " << jidCREAM << " FAILURE Already initialized" << endl;
        else if (action.compare("FINALIZE") == 0)
        {
            end = true;
	    pthread_cancel(creamTimer);
            pthread_attr_destroy(&attr); 
            creamEmMad->finalize();
        }
        else {
	    while ((i=getFreeThread()) == -1);
            operation[i].action = action;
            operation[i].jidCREAM = jidCREAM;
            operation[i].contact = contact;
            operation[i].jdlFile = jdlFile;
            pthread_create(&creamOperation[i], &attr, creamAction, &operation[i]);
        }
  } 
}

int getFreeThread()
{
    bool found = false;

    for (int i=0;i<MAX_THREADS;i++)
    {
	pthread_mutex_lock(&(operation[i].mutex));
            if (operation[i].free == true) 
            {
            	operation[i].free = false;
		found = true;
            }
        pthread_mutex_unlock(&(operation[i].mutex));

	if (found == true) return i;
    }

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

    if (action.compare("SUBMIT") == 0)
    {
        host = contact.substr(0, contact.find("/"));
        status = creamEmMad->submit(jidCREAM, host, jdlFile);
    }
    else if (action.compare("RECOVER") == 0)
    {
        status = creamEmMad->recover(jidCREAM, contact);
    }
    else if (action.compare("CANCEL") == 0)
        status = creamEmMad->cancel(jidCREAM);
    else if (action.compare("POLL") == 0)
        status = creamEmMad->poll(jidCREAM);

    pthread_mutex_lock(&(data->mutex));
        data->free = true;
    pthread_mutex_unlock(&(data->mutex));

    pthread_exit(NULL);
}
