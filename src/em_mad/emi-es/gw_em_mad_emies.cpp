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

#include "gw_em_mad_emies.h"
	
#include <pthread.h>
#include <string>
#include <iostream>

const char * usage =
"USAGE\n gw_em_mad_emies [-h] [-t] \n\n"
"SYNOPSIS\n"
"  Execution driver to interface with EMI-ES services. It is not intended to be used from CLI.\n\n"
"OPTIONS\n"
"  -h    print this help";

const char * susage =
"usage: gw_em_mad_emies [-h]";

void *emiesAction(void *thread_data);

using namespace std;

typedef struct thread_operation_s{
    string action;
    int jidEMIES;
    string contact;
    string adlFile;
} thread_operation_t;

pthread_mutex_t mutex;
pthread_cond_t cond;
int num_active = 0;
EMIESEmMad *emiesEmMad = NULL;

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
    string adlFile;
    int jidEMIES;
    char opt;
    int paramNum;
    thread_operation_t *operation;

    pthread_t thread_id;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_mutex_init(&mutex, 0);
    pthread_cond_init(&cond, 0);

    while((opt = getopt(argc, argv, "t:h")) != -1)
        switch(opt)
        {
            case 'h':
                cout << usage << endl;
                exit(0);
                break;
            case '?':
                cout << "error: invalid option " << (char)optopt << endl;
                cout << susage << endl;
                exit(1);
                break;                
        }

    while (!end)
    {
        cin.getline(str,4096,'\n');

        paramNum = sscanf(str, "%s %s %s %[^\n]", str1, str2, str3, str4);

        if (paramNum != 4)
        {
            cout << "FAILURE Not all four arguments defined" << endl;
            continue;
        }

        action = str1;
        jidEMIES = atoi(str2);
        contact = str3;
        adlFile = str4;

        if (emiesEmMad == NULL)
            if (action.compare("INIT") == 0)
            {
                emiesEmMad = new EMIESEmMad();
                emiesEmMad->init();
            }
            else
               cout << action << " " << jidEMIES << " FAILURE Not initialized" << endl;
        else if (action.compare("INIT") == 0)
               cout << action << " " << jidEMIES << " FAILURE Already initialized" << endl;
        else if (action.compare("FINALIZE") == 0)
        {
            end = true;
            pthread_attr_destroy(&attr); 
            pthread_mutex_destroy(&mutex);
            pthread_cond_destroy(&cond);
            emiesEmMad->finalize();
        }
        else if ( (action.compare("SUBMIT") == 0) || (action.compare("POLL") == 0) || (action.compare("CANCEL") == 0) || (action.compare("RECOVER") == 0) ) {
            operation = new thread_operation_t;
            operation->action = action;
            operation->jidEMIES = jidEMIES;
            operation->contact = contact;
            operation->adlFile = adlFile;
            pthread_mutex_lock(&mutex);
                while (num_active >= MAX_THREADS)
                    pthread_cond_wait(&cond, &mutex); 
                num_active++;
            pthread_mutex_unlock(&mutex);
            pthread_create(&thread_id, &attr, emiesAction, operation);
        }
        else
            cout << "FAILURE " << action << " is not a valid action" << endl;
  } 
}

void *emiesAction(void *thread_data)
{
    thread_operation_t *data;
    data = (thread_operation_t*) thread_data;

    string action =  data->action;
    string contact = data->contact;
    string adlFile = data->adlFile;
    int jidEMIES = data->jidEMIES;
    delete data;

    pthread_detach(pthread_self());

    if (action.compare("SUBMIT") == 0)
        emiesEmMad->submit(jidEMIES, contact, adlFile);
    else if (action.compare("RECOVER") == 0)
        emiesEmMad->recover(jidEMIES, contact); 
    else if (action.compare("CANCEL") == 0)
        emiesEmMad->cancel(jidEMIES);
    else if (action.compare("POLL") == 0)
        emiesEmMad->poll(jidEMIES);

    pthread_mutex_lock(&mutex);
        num_active--;
        pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
    pthread_exit(NULL);
}

