/* -------------------------------------------------------------------------- */
/* Copyright 2002-2013 GridWay Project Leads (GridWay.org)                    */
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
#include <arc/compute/Job.h>
//#include <arc/client/Job.h>
#include <arc/UserConfig.h>
#include <arc/Utils.h>

using namespace std;

/*************** 
EMIESEmMad class
***************/

EMIESEmMad::EMIESEmMad()
{
    this->emiesJobs.clear();
    this->emiesService = new EMIESService();
}

void EMIESEmMad::init()
{
    FILE *file;
    char path[512];

    pthread_mutex_init(&jobMutex, 0);

    file = popen("grid-proxy-info -path", "r");

    if (file == NULL)
    {
        cout << "INIT - FAILURE Error getting proxy path" << endl;
        return;
    }

    while( fgets(path, sizeof(path), file) != NULL )
    {
        // Keep looping even if we just expect one line
    }

    pclose(file);

    if (path == NULL)
    {
        cout << "INIT - FAILURE Error reading proxy path" << endl;
        return;
    }

    cout << "INIT - SUCCESS -" << endl;

    return;
}

void EMIESEmMad::submit(int jid, string contact, string adlFile)
{
    string serviceAddress;
    string emiesURL;
    string emiesJid;
    string host;
    string lrms;

    host = contact.substr(0, contact.find("/"));
    lrms = contact.substr(contact.find("/"), contact.length()-1);
    if (lrms.find("arc") != string::npos)
        serviceAddress = "https://" + host + ":60000/arex";
    else if (lrms.find("glite") != string::npos)
        serviceAddress = "https://" + host + ":8443/ce-cream-es/services/ActivityCreationService";
    else if (lrms.find("unicore") != string::npos)
        serviceAddress = "https://" + host + ":8080/EMI-ES/services/CreateActivityService";
    else
    {
        cout << "SUBMIT " << jid << " FAILURE Contact not valid" << endl;
        return;
    }

    EMIESOperation result = emiesService->submit(serviceAddress, adlFile);

    if (result.code == -1)
    {        
        cout << "SUBMIT " << jid << " FAILURE " << result.info << endl;
        return;
    }

    pthread_mutex_lock(&jobMutex); 
        map<int, Arc::EMIESJob *>::iterator it = emiesJobs.find(jid);

        if (it != emiesJobs.end())
        {
            if (it->second != NULL) 
                delete it->second;
            it->second = result.job;
        }
        else
            emiesJobs.insert(pair<int, Arc::EMIESJob *>(jid, result.job));
        emiesURL = result.job->manager.fullstr();
        emiesJid = result.job->id;
    pthread_mutex_unlock(&jobMutex); 

    cout << "SUBMIT " << jid << " SUCCESS " << emiesURL << "/" << emiesJid << endl;

    return;
}

void EMIESEmMad::poll(int jid)
{
    map<int, Arc::EMIESJob *>::iterator it = emiesJobs.find(jid);

    if (it == emiesJobs.end())
    {
        cout << "POLL " << jid << " FAILURE " << "The job ID does not exist" << endl;
        return;
    }

    pthread_mutex_lock(&jobMutex);
        Arc::EMIESJob* emiesJob = it->second;
        string emiesJid = emiesJob->id;
        string serviceAddress = emiesJob->manager.fullstr();
    pthread_mutex_unlock(&jobMutex);

    EMIESOperation result = emiesService->poll(emiesJid, serviceAddress);

    if (result.code == 0)
        cout << "POLL " << jid << " SUCCESS " << result.info << endl;
    else
        cout << "POLL " << jid << " FAILURE " << result.info << endl;

    if ((result.info.compare("DONE") == 0) || (result.info.compare("FAILED") == 0))
    {
        pthread_mutex_lock(&jobMutex);
            delete emiesJobs.find(jid)->second;
            emiesJobs.erase(jid);
        pthread_mutex_unlock(&jobMutex);
    }

    return;
}

void EMIESEmMad::cancel(int jid)
{
    map<int, Arc::EMIESJob *>::iterator it = emiesJobs.find(jid);

    if (it == emiesJobs.end())
    {
        cout << "CANCEL " << jid << " FAILURE " << "The job ID does not exist" << endl;
        return;
    }

    pthread_mutex_lock(&jobMutex);
        Arc::EMIESJob *emiesJob = it->second;
        if (emiesJob == NULL)
        {
            pthread_mutex_unlock(&jobMutex);
            return;
        }
        string emiesJid = emiesJob->id;
        string serviceAddress = emiesJob->manager.fullstr();
    pthread_mutex_unlock(&jobMutex);

    EMIESOperation result = emiesService->cancel(emiesJid, serviceAddress);

    if (result.code == 0)
        cout << "CANCEL " << jid << " SUCCESS - " << endl;
    else
        cout << "CANCEL " << jid << " FAILURE " << result.info << endl;
}

void EMIESEmMad::finalize()
{
    pthread_mutex_destroy(&jobMutex);
    cout << "FINALIZE SUCCESS - -" << endl;
    return; 
}

void EMIESEmMad::recover(int jid, string contact)
{
    Arc::EMIESJob* job = new Arc::EMIESJob();

    size_t pos = contact.find_last_of("/");
    string emiesURL = contact.substr(0, pos);
    string emiesJid = contact.substr(pos+1);

    Arc::URL url(emiesURL);
    job->manager = url;
    job->id = emiesJid;

    pthread_mutex_lock(&jobMutex);
        map<int, Arc::EMIESJob *>::iterator it = emiesJobs.find(jid);

        if (it != emiesJobs.end())
        {
            if (it->second != NULL)
                delete it->second;
            it->second = job;
        }
        else
            emiesJobs.insert(pair<int, Arc::EMIESJob *>(jid, job));
    pthread_mutex_unlock(&jobMutex);

    EMIESOperation result = emiesService->poll(emiesJid, emiesURL);

    if ((result.info.compare("DONE") == 0) || (result.info.compare("FAILED") == 0))
    {
        pthread_mutex_lock(&jobMutex);
            delete emiesJobs.find(jid)->second;
            emiesJobs.erase(jid);
        pthread_mutex_unlock(&jobMutex);
    }

    if (result.code == 0)
        cout << "RECOVER " << jid << " SUCCESS " << result.info << endl;
    else
        cout << "RECOVER " << jid << " FAILURE " << result.info << endl;

    return;
}

/***************** 
EMIESService class
*****************/

EMIESService::EMIESService() : clients(this->usercfg) {}

EMIESOperation EMIESService::submit(string serviceAddress, string adlFile)
{
    EMIESOperation result;
    Arc::EMIESJob* job = new Arc::EMIESJob();
    Arc::EMIESJobState state;

    Arc::URL url(serviceAddress);
    Arc::AutoPointer<Arc::EMIESClient> emiesClient(clients.acquire(serviceAddress));

    Arc::XMLNode adl;
    adl.ReadFromFile(adlFile);

    string delegation_id = emiesClient->delegation();
    if (delegation_id.empty()) 
    {
        result.info = emiesClient->failure();
        result.code = -1;
        clients.release(emiesClient.Release());
        return result;
    }
    if (!emiesClient->submit(adl, *job, state, delegation_id))
    {
        result.info = emiesClient->failure();
        result.code = -1;
        clients.release(emiesClient.Release());
        return result;
    }

    result.job = job;
    result.code = 0;
    clients.release(emiesClient.Release());
    return result;
}

EMIESOperation EMIESService::poll(string emiesJid, string serviceAddress)
{
    EMIESOperation result;
    Arc::EMIESJob job;
    string status = "UNKNOWN";
    string attr;
    Arc::EMIESJobState emiesState;

    Arc::URL url(serviceAddress);
    job.manager = url;
    job.id = emiesJid;
    Arc::AutoPointer<Arc::EMIESClient> emiesClient(clients.acquire(url));

    if (!emiesClient->stat(job, emiesState)) 
    {
        result.info = emiesClient->failure(); 
        result.code = -1;
        clients.release(emiesClient.Release());
        return result;
    }

    for (list<string>::const_iterator emiesAttr = emiesState.attributes.begin(); emiesAttr != emiesState.attributes.end(); ++emiesAttr) 
         attr = *emiesAttr;

    if (emiesState.state.compare("accepted") == 0)
        status = "PENDING";
    else if (emiesState.state.compare("preprocessing") == 0)
        status = "PENDING";
    else if (emiesState.state.compare("processing") == 0)
        status = "PENDING";
    else if (emiesState.state.compare("processing-accepting") == 0)
        status = "PENDING";
    else if (emiesState.state.compare("processing-queued") == 0)
        status = "PENDING";
    else if (emiesState.state.compare("processing-running") == 0)
        status = "ACTIVE";
    else if (emiesState.state.compare("postprocessing") == 0)
        status = "ACTIVE";
    else if (emiesState.state.compare("terminal") == 0)
    {
        if (attr == "preprocessing-cancel" || "processing-cancel" || "postprocessing-cancel" || "validation-failure" || "preprocessing-failure" || "processing-failure" || "postprocessing-failure" || "app-failure")
            status = "FAILED";
        status = "DONE";
    }

    result.info = status;
    result.code = 0;
    clients.release(emiesClient.Release());
    return result;
}

EMIESOperation EMIESService::cancel(string emiesJid, string serviceAddress)
{
    EMIESOperation result;
    Arc::EMIESJob job;

    Arc::URL url(serviceAddress);
    job.manager = url;
    job.id = emiesJid;
    Arc::AutoPointer<Arc::EMIESClient> emiesClient(clients.acquire(url));

    if (!emiesClient->kill(job)) 
    {
        result.info = emiesClient->failure();
        result.code = -1;
        clients.release(emiesClient.Release());
        return result;
    }

    result.code = 0;
    clients.release(emiesClient.Release());
    return result;
}
