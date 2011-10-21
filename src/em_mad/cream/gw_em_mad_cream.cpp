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

void *CreamAction(void *thread_data);
void *Timer(void *thread_data);
int getthreadid();

using namespace std;

typedef struct thread_operation_s{
    string *action;
    int jidCREAM;
    int threadid;
    string *contact;
    string *jdlFile;
    CreamEmMad *creamEmMad;
} thread_operation_t;
thread_operation_t thread_operation[MAX_THREADS];


int main( int argc, char *argv[]) 
{
    char str[4096];
    char str1[20];
    char str2[20];
    char str3[500];
    char str4[1024];
    bool end=false;
    string *action;
    string *contact;
    string *jdlFile;
    int paramNum;
    int jidCREAM;
    int i = 1;
    CreamEmMad *creamEmMad=NULL; 

    pthread_t timer;
    pthread_t operation;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    for(i=0;i<MAX_THREADS;i++)
        thread_operation[i].threadid = -1;

    while (!end)
    {
        cin.getline(str,4096,'\n');

        paramNum = sscanf(str, "%s %s %s %[^\n]", str1, str2, str3, str4);

        action = new string(str1);
        jidCREAM = atoi(str2);
        contact = new string(str3);
        jdlFile = new string(str4);
        if (paramNum != 4)
        {
            cout << "FAILURE Not all four arguments defined" << endl;
            continue;
        }
        else if (creamEmMad == NULL)
            if (action->compare("INIT") == 0)
            {
                //TODO: Use other delegationID?
                if (argc == 1)
                    creamEmMad = new CreamEmMad("GridWay");
                else if (argc == 2)
                    creamEmMad = new CreamEmMad(argv[1]);

                creamEmMad->init();
                pthread_create(&timer, &attr, Timer, (void *) creamEmMad);
            }
            else
               cout << action->c_str() << " " << jidCREAM << " FAILURE Not initialized" << endl;
        else if (action->compare("INIT") == 0)
               cout << action->c_str() << " " << jidCREAM << " FAILURE Already initialized" << endl;
        else if (end = (action->compare("FINALIZE") == 0))
        {
            creamEmMad->finalize();
            pthread_kill(timer,9);
            return 0;
        }
        else {
	    while ( (i=getthreadid()) == -1 );
            thread_operation[i].action = action;
            thread_operation[i].jidCREAM = jidCREAM;
	    thread_operation[i].threadid = i;
            thread_operation[i].contact = contact;
            thread_operation[i].jdlFile = jdlFile;
            thread_operation[i].creamEmMad = creamEmMad;
            pthread_create(&operation, &attr, CreamAction, &thread_operation[i]);
        }
  }

  pthread_exit(NULL);
}

int getthreadid()
{
    int i;

    for(i=1;i<MAX_THREADS;i++)
        if(thread_operation[i].threadid == -1) return i;
    return -1;
}

void *Timer(void *thread_data)
{
    CreamEmMad *my_data;
    my_data = (CreamEmMad *) thread_data;

    my_data->timer();
    pthread_exit(NULL);
}

void *CreamAction(void *thread_data)
{
    thread_operation_t *my_data;
    my_data = (thread_operation_t *) thread_data;

    string *action =  my_data->action;
    string *contact = my_data->contact;
    string *jdlFile = my_data->jdlFile;
    string host;
    int jidCREAM = my_data->jidCREAM;
    int threadid = my_data->threadid;
    CreamEmMad *creamEmMad = my_data->creamEmMad;
    int status = -1;

    if (action->compare("SUBMIT") == 0)
    {
        host = contact->substr(0, contact->find("/"));
        status = creamEmMad->submit(jidCREAM, &host, jdlFile, threadid);
    }
    else if (action->compare("RECOVER") == 0)
    {
        status = creamEmMad->recover(jidCREAM, contact, threadid);
    }
    else if (action->compare("CANCEL") == 0)
        status = creamEmMad->cancel(jidCREAM, threadid);
    else if (action->compare("POLL") == 0)
        status = creamEmMad->poll(jidCREAM, threadid);

    if (status != 0)
        cout << action->c_str() << " " << jidCREAM << " FAILURE " << (creamEmMad->getInfo(threadid))->c_str() << endl;

    thread_operation[threadid].threadid = -1;
    delete action;
    delete contact;
    delete jdlFile;
    pthread_exit(NULL);
}
