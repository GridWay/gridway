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
#include <pthread.h>
#include <time.h>

CreamJob::CreamJob(int gridwayID, string *creamJobId, string *creamURL)
{
    this->gridwayID = gridwayID;
    this->creamJobId = new string(*creamJobId);
    this->creamURL = new string(*creamURL);
}

int CreamJob::getGridWayID()
{
    return this->gridwayID;
}

string *CreamJob::getCreamURL()
{
    return this->creamURL;
}

string *CreamJob::getCreamJobId()
{
    return this->creamJobId;
}

CreamEmMad::CreamEmMad(char *delegation)
{
    this->delegationID = new string(delegation);
    this->info = new map<int, string> ();
    this->baseAddress = NULL;
    this->localCreamJID = NULL;
    this->creamJobs = new map <int, CreamJob> ();
    this->credentials = new list<string> ();
    this->refreshTime = 300;
}

void CreamEmMad::init()
{
    FILE *file;
    char path[512], *pline;

    pthread_mutex_init(&credentialsMutex, 0);
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

    pline =  strchr(path, '\n');
    if (pline != NULL)
        *pline = '\0';

    this->certificatePath = new string(path);

    cout << "INIT - SUCCESS -" << endl;
    
    return;
}

int CreamEmMad::submit(int jid, string *contact, string *jdlFile, int threadid)
{
    if (this->proxyDelegate(contact, threadid) != 0)
    {
        return -1;
    }

    CreamJob *job = this->jobSubmit(jid, contact, jdlFile, threadid);

    if (job == NULL)
        return -1;

    contact = job->getCreamURL();
    
    cout << "SUBMIT " << jid << " SUCCESS " << contact->substr(0, contact->find("/ce-cream")) << "/" << *(job->getCreamJobId()) << endl;  
    
    return 0;
}

int CreamEmMad::poll(int jid, int threadid)
{
    string status;
 
    map<int,CreamJob>::iterator it = this->creamJobs->find(jid);
 
    if (it == creamJobs->end())
    {
        pthread_mutex_lock(&jobMutex);
            this->info->insert(pair<int, string>(threadid, "The job ID does not exist"));
        pthread_mutex_unlock(&jobMutex);
        return -1;
    }

    CreamJob creamJob = it->second;
   
    API::JobIdWrapper job1(*(creamJob.getCreamJobId()), *(creamJob.getCreamURL()), vector<API::JobPropertyWrapper>());

    vector< API::JobIdWrapper > JobVector;

    JobVector.push_back( job1 );

    string delegationID = "";

    int fromDate = -1;
    int toDate   = -1;
    vector<string> statusVec;

    API::JobFilterWrapper jfw( JobVector, statusVec, fromDate, toDate, "", "");

    API::ResultWrapper result;

    API::AbsCreamProxy::StatusArrayResult Sresult; 

    API::AbsCreamProxy* creamClient = API::CreamProxyFactory::make_CreamProxyStatus( &jfw, &Sresult, connectionTimeout );
  
    if (creamClient == NULL)
    {
        pthread_mutex_lock(&jobMutex);
            this->info->insert(pair<int, string>(threadid, "Error creating Cream client"));
        pthread_mutex_unlock(&jobMutex);
        return -1;
    }

    string serviceAddress = *(creamJob.getCreamURL());

    try 
    {
        creamClient->setCredential(this->certificatePath->c_str());
        creamClient->execute(serviceAddress);
    }
    catch(exception& ex) 
    {
        pthread_mutex_lock(&jobMutex);
            this->info->insert(pair<int, string>(threadid, ex.what()));
        pthread_mutex_unlock(&jobMutex);
        delete creamClient;
        return -1;
    }

    map<string, boost::tuple<API::JobStatusWrapper::RESULT, API::JobStatusWrapper, std::string> >::const_iterator jobIt = Sresult.begin();

    while( jobIt != Sresult.end() ) 
    {
        if( jobIt->second.get<0>() == API::JobStatusWrapper::OK ) 
        {
            status = jobIt->second.get<1>().getStatusName();
            if (status.compare("REGISTERED") == 0)
                status = "PENDING";
            else if (status.compare("PENDING") == 0) // For completeness
                status = "PENDING";
            else if (status.compare("IDLE") == 0)
                status = "PENDING";
            else if (status.compare("RUNNING") == 0)
                status = "ACTIVE";
            else if (status.compare("REALLY-RUNNING") == 0)
                status = "ACTIVE";
            else if (status.compare("HELD") == 0)
                status = "PENDING";
            else if (status.compare("CANCELLED") == 0)
                status = "DONE";
            else if (status.compare("DONE-OK") == 0)
                status = "DONE";
            else if (status.compare("DONE-FAILED") == 0)
                status = "FAILED";
            else if (status.compare("ABORTED") == 0)
                status = "FAILED";
 
            cout << "POLL " << jid << " SUCCESS " << status << endl;
        }
        else 
        {
            pthread_mutex_lock(&jobMutex);
                this->info->insert(pair<int, string>(threadid, jobIt->second.get<2>()));
            pthread_mutex_lock(&jobMutex);
            delete creamClient;
            return -1;    
        }
        jobIt++;
   }
 
   delete creamClient;

   return 0;
}

int CreamEmMad::cancel(int jid, int threadid)
{
    map<int,CreamJob>::iterator it = this->creamJobs->find(jid);
   
    if (it == creamJobs->end())
    {
        pthread_mutex_lock(&jobMutex);
            this->info->insert(pair<int, string>(threadid, "The job ID does not exist"));
        pthread_mutex_unlock(&jobMutex);
        return -1;
    }
  
    CreamJob creamJob = it->second;
   
    API::JobIdWrapper job1(*(creamJob.getCreamJobId()), *(creamJob.getCreamURL()), vector<API::JobPropertyWrapper>());

    vector< API::JobIdWrapper > JobVector;

    JobVector.push_back( job1 );

    int fromDate = -1;
    int toDate   = -1;
    vector<string> statusVec;

    API::JobFilterWrapper jfw( JobVector, statusVec, fromDate, toDate, "", "" );

    API::ResultWrapper result;

    API::AbsCreamProxy* creamClient = API::CreamProxyFactory::make_CreamProxyCancel( &jfw, &result, this->connectionTimeout );
  
    if (creamClient == NULL)
    {
        pthread_mutex_lock(&jobMutex);
            this->info->insert(pair<int, string>(threadid, "Error creating Cream client to cancel job"));
        pthread_mutex_unlock(&jobMutex);
        return -1;
    }

    string serviceAddress = *(creamJob.getCreamURL());

    try 
    {
        creamClient->setCredential(this->certificatePath->c_str());
        creamClient->execute(serviceAddress);
    } 
    catch(exception& ex) 
    {
        pthread_mutex_lock(&jobMutex);
	    this->info->insert(pair<int, string>(threadid, ex.what()));
        pthread_mutex_unlock(&jobMutex);
        delete creamClient;
        return -1;
    }

    cout << "CANCEL " << jid << " SUCCESS" << endl;  
    delete creamClient;

    return 0;
}


void CreamEmMad::finalize()
{
    cout << "FINALIZE SUCCESS - -" << endl;
    return;
}

string *CreamEmMad::getInfo(int threadid)
{
    map<int,string>::iterator it = this->info->find(threadid);
    return &(it->second);
}

int CreamEmMad::proxyDelegate(string *contact, int threadid)
{
    API::AbsCreamProxy *creamClient; 
    string *serviceAddress;

    list<string>::iterator it;
    pthread_mutex_lock(&credentialsMutex);
        for (it=this->credentials->begin(); it != this->credentials->end(); it++) 
        {
             if (*contact == *it)
             {
                  pthread_mutex_unlock(&credentialsMutex); 
                  return 0;
             }
        }
        this->credentials->push_back(*contact);
    pthread_mutex_unlock(&credentialsMutex);

    creamClient = API::CreamProxyFactory::make_CreamProxyDelegate(*(this->delegationID), this->connectionTimeout);

    if (creamClient == NULL)
    {
        pthread_mutex_lock(&jobMutex);
            this->info->insert(pair<int, string>(threadid, "Error creating Cream proxy"));
        pthread_mutex_unlock(&jobMutex);
        return -1;
    }

    serviceAddress = new string("https://" + *contact + ":8443/ce-cream/services/gridsite-delegation");

    try 
    {
        creamClient->setCredential(this->certificatePath->c_str());
        creamClient->execute(*serviceAddress);
    } 
    catch(DelegationException& ex)
    {
	if (this->proxyRenew(contact, threadid) != 0)
        {
            pthread_mutex_lock(&jobMutex);
                this->info->insert(pair<int, string>(threadid, ex.what()));
            pthread_mutex_unlock(&jobMutex);
            delete creamClient; 

            return -1;
        }
    }
    catch(exception& ex) 
    {
        pthread_mutex_lock(&jobMutex);
            this->info->insert(pair<int, string>(threadid, ex.what()));
        pthread_mutex_unlock(&jobMutex);
        delete creamClient;

        return -1;
    }

    delete creamClient;
 
    return 0;
}

int CreamEmMad::proxyRenew(string *contact, int threadid)
{
    API::AbsCreamProxy *creamClient;
    string *serviceAddress;

    creamClient = API::CreamProxyFactory::make_CreamProxy_ProxyRenew(*(this->delegationID), this->connectionTimeout);

    if (creamClient == NULL)
    {
        pthread_mutex_lock(&jobMutex);
            this->info->insert(pair<int, string>(threadid, "Error renewing Cream proxy"));
        pthread_mutex_unlock(&jobMutex);
        return -1;
    }

    serviceAddress = new string("https://" + *contact + ":8443/ce-cream/services/gridsite-delegation");

    try
    {
        creamClient->setCredential(this->certificatePath->c_str());
        creamClient->execute(*serviceAddress);
    }
    catch(exception& ex)
    {
        pthread_mutex_lock(&jobMutex);
            this->info->insert(pair<int, string>(threadid, ex.what()));
        pthread_mutex_unlock(&jobMutex);
        delete creamClient;

        return -1;
    }

    delete creamClient;

    return 0;
}

CreamJob *CreamEmMad::jobSubmit(int jid,string *contact,string *jdlFile, int threadid)
{
    string *JDL = this->fileToString(jdlFile, threadid);
    string *jidCREAM = new string("GridWayJob");
    string *serviceAddress;
    string *creamURL;
    string *creamJobId;
    CreamJob *creamJob;

    if (JDL == NULL)
        return NULL;

    stringstream jidss;
    jidss << jid;
    *jidCREAM += jidss.str();

    API::JobDescriptionWrapper jd(*JDL, *(this->delegationID), "", "", true, jidCREAM->c_str());

    API::AbsCreamProxy::RegisterArrayRequest reqs;
    API::AbsCreamProxy::RegisterArrayResult resp;

    reqs.push_back( &jd );
    
    API::AbsCreamProxy* creamClient = API::CreamProxyFactory::make_CreamProxyRegister( &reqs, &resp, connectionTimeout );

    if (creamClient == NULL)
    {
        pthread_mutex_lock(&jobMutex);
            this->info->insert(pair<int, string>(threadid, "Error creating Cream client"));
        pthread_mutex_unlock(&jobMutex);
        return NULL;
    }

    if (contact == NULL || (contact->size() == 1 && contact->find("-") != string::npos))
        serviceAddress = new string(*(this->baseAddress)+":8443/ce-cream/services/CREAM2");
    else if (contact->find("https") == string::npos)
        serviceAddress = new string("https://" + *(contact)+ ":8443/ce-cream/services/CREAM2");
    else
        serviceAddress = new string(*contact);

    try 
    {
        creamClient->setCredential( this->certificatePath->c_str() );
        creamClient->execute(*serviceAddress);
    } 
    catch(exception& ex)
    {
        pthread_mutex_lock(&jobMutex);
            this->info->insert(pair<int, string>(threadid, ex.what()));
        pthread_mutex_unlock(&jobMutex);
        delete creamClient;
        return NULL;
    }

    boost::tuple<bool, API::JobIdWrapper, string> registrationResponse = resp[jidCREAM->c_str()];
       
    if(registrationResponse.get<0>()!=API::JobIdWrapper::OK) 
    {
        pthread_mutex_lock(&jobMutex);
	    this->info->insert(pair<int, string>(threadid, registrationResponse.get<2>()));
        pthread_mutex_unlock(&jobMutex);
        delete creamClient;
        return NULL;
    }  

    creamURL  = new string(registrationResponse.get<1>().getCreamURL());
    creamJobId  = new string(registrationResponse.get<1>().getCreamJobID());

    creamJob  = new CreamJob(jid, creamJobId, creamURL);

    pthread_mutex_lock(&jobMutex);
        this->creamJobs->erase(jid);
        this->creamJobs->insert(pair<int, CreamJob>(jid, *creamJob));
    pthread_mutex_unlock(&jobMutex);

    delete creamClient;
    
    return creamJob;
}

int CreamEmMad::recover(int jid, string *contact, int threadid)
{
    string *creamJobId;
    string *creamURL;
    CreamJob *creamJob;
    string *host;

    size_t pos = contact->find(":8443/");
    host = new string(contact->substr(8, pos-8));

    if (this->proxyDelegate(host, threadid) != 0)
    {
        return -1; 
    }

    pos = contact->find("/CREAM");

    creamURL = new string(contact->substr(0, pos));
    *creamURL += "/ce-cream/services/CREAM2";
    creamJobId = new string(contact->substr(pos+1));

    creamJob = new CreamJob(jid, creamJobId, creamURL);

    pthread_mutex_lock(&jobMutex);
        this->creamJobs->insert(pair<int, CreamJob>(jid, *creamJob));
    pthread_mutex_unlock(&jobMutex);

    return this->poll(jid, threadid);
}

string *CreamEmMad::fileToString(string *jdlFileName, int threadid)
{
    string *jdlString = new string("");
    ifstream *jdlFile = new ifstream(jdlFileName->c_str());
    char str[4096];

    if (!jdlFile->is_open())
    {
        pthread_mutex_lock(&jobMutex);
            this->info->insert(pair<int, string>(threadid, "Error open the JDL file: " + *jdlFileName));
        pthread_mutex_unlock(&jobMutex);
        return NULL;
    }
    
    while (jdlFile->getline(str,40096,'\n'))
    {
        string *aux = new string(str);
        (*jdlString) += *aux;
    }
 
    return jdlString;
}

void CreamEmMad::timer()
{
    list<string>::iterator it;
    string contact;

    for (;;)
    {
        sleep(this->refreshTime);
        for (it=this->credentials->begin(); it != this->credentials->end(); it++){   
	     contact=*it; 
             if (this->proxyRenew(&contact, 0) != 0) return;
	 }
    }
}
