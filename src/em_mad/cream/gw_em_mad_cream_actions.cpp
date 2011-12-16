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

/* 
CreamJob class
*/
CreamJob::CreamJob(int gridwayID, string creamJobId, string creamURL)
{
    this->gridwayID = gridwayID;
    this->creamJobId = creamJobId;
    this->creamURL = creamURL;
}

int CreamJob::getGridWayID()
{
    return this->gridwayID;
}

string CreamJob::getCreamURL()
{
    return this->creamURL;
}

string CreamJob::getCreamJobId()
{
    return this->creamJobId;
}

/* 
CreamEmMad class
*/

CreamEmMad::CreamEmMad(string delegation, int refreshTime)
{
    if (delegation == "")
	this->delegationID = "GridWay";
    else
    	this->delegationID = delegation;
    this->refreshTime = 43200;
    if (refreshTime > 0)
        this->refreshTime = refreshTime;
    this->creamJobs = new map <int, CreamJob> ();
    this->credentials = new list<string> ();
    this->creamService = new CreamService();
}

CreamEmMad::~CreamEmMad() 
{
    delete this->creamJobs;
    delete this->credentials;
    delete this->creamService;
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

    pthread_mutex_lock(&jobMutex);
    	this->creamService->setPath(path);
    pthread_mutex_unlock(&jobMutex);

    cout << "INIT - SUCCESS -" << endl;
    
    return;
}

int CreamEmMad::submit(int jid, string contact, string jdlFile)
{
    if (this->proxyDelegate("SUBMIT", jid, contact, delegationID) != 0)
    {
        return -1;
    }

    string JDL = this->fileToString(jdlFile);
    CreamOperation result = creamService->submit(jid, contact, JDL, delegationID);

    if (result.job == NULL)
        return -1;

    pthread_mutex_lock(&jobMutex);
        this->creamJobs->erase(jid);
        this->creamJobs->insert(pair<int, CreamJob>(jid, *result.job));
    pthread_mutex_unlock(&jobMutex);

    contact = result.job->getCreamURL();
    
    if (result.code == 0)
    	cout << "SUBMIT " << jid << " SUCCESS " << contact.substr(0, contact.find("/ce-cream")) << "/" << (result.job->getCreamJobId()) << endl;  
    else
        cout << "SUBMIT " << jid << " FAILURE " << result.info << endl;

    delete result.job;   
    return 0;
}

int CreamEmMad::poll(int jid)
{
    map<int,CreamJob>::iterator it = this->creamJobs->find(jid);
 
    if (it == creamJobs->end())
    {
        cout << "POLL " << jid << " FAILURE " << "The job ID does not exist" << endl;
        return -1;
    }

    CreamJob creamJob = it->second;

    CreamOperation result = creamService->poll(creamJob); 

    if (result.code == 0)
        cout << "POLL " << jid << " SUCCESS " << result.info << endl;
    else 
        cout << "POLL " << jid << " FAILURE " << result.info << endl;

    if (result.info.compare("DONE") == 0 || result.info.compare("FAILED") == 0)
        this->creamJobs->erase(jid);

    return 0;
}

int CreamEmMad::cancel(int jid)
{
    map<int,CreamJob>::iterator it = this->creamJobs->find(jid);
   
    if (it == creamJobs->end())
    {
        cout << "CANCEL " << jid << " FAILURE " << "The job ID does not exist" << endl;
        return -1;
    }

    CreamJob creamJob = it->second;

    CreamOperation result = creamService->cancel(creamJob);

    if (result.code == 0)
        cout << "CANCEL " << jid << " SUCCESS" << endl;  
    else
        cout << "CANCEL " << jid << " FAILURE " << result.info << endl;

    return 0;
}

void CreamEmMad::finalize()
{
    pthread_mutex_destroy(&jobMutex);
    pthread_mutex_destroy(&credentialsMutex);
    cout << "FINALIZE SUCCESS - -" << endl;
    return; 
}

int CreamEmMad::proxyDelegate(string action, int jid, string contact, string delegationID)
{
    list<string>::iterator it;
    pthread_mutex_lock(&credentialsMutex);
        for (it=this->credentials->begin(); it != this->credentials->end(); it++) 
        {
             if (contact == *it)
             {
                  pthread_mutex_unlock(&credentialsMutex); 
                  return 0;
             }
        }
        this->credentials->push_back(contact);
	CreamOperation result = creamService->proxyDelegate(contact, delegationID);
    pthread_mutex_unlock(&credentialsMutex);

    if (result.code != 0)
    {
	cout << action << " " << jid << " FAILURE " << result.info << endl;
	return -1;
    }
    return 0;
}

int CreamEmMad::recover(int jid, string contact)
{
    string creamJobId;
    string creamURL;
    CreamJob *creamJob;
    string host;

    size_t pos = contact.find(":8443/");
    host = contact.substr(8, pos-8);

    if (this->proxyDelegate("RECOVER", jid, host, delegationID) != 0)
        return -1; 

    pos = contact.find("/CREAM");

    creamURL = contact.substr(0, pos);
    creamURL += "/ce-cream/services/CREAM2";
    creamJobId = contact.substr(pos+1);

    creamJob = new CreamJob(jid, creamJobId, creamURL);

    pthread_mutex_lock(&jobMutex);
        this->creamJobs->erase(jid); 
        this->creamJobs->insert(pair<int, CreamJob>(jid, *creamJob));
    pthread_mutex_unlock(&jobMutex);

    delete creamJob;
    return this->poll(jid);
}

string CreamEmMad::fileToString(string jdlFileName)
{
    string jdlString = "";
    ifstream *jdlFile = new ifstream(jdlFileName.c_str());
    char str[4096];

    if (!jdlFile->is_open())
    {
        cout << "SUBMIT " << jid << " FAILURE " << "Error open the JDL file: " + jdlFileName << endl;
        return NULL;
    }
    
    while (jdlFile->getline(str,40096,'\n'))
    {
        string aux = str;
        jdlString += aux;
    }
 
    delete jdlFile;
    return jdlString;
}

void CreamEmMad::timer()
{
    list<string>::iterator it;
    string contact;
    CreamOperation result;

    for (;;)
    {
        sleep(this->refreshTime);
        for (it=this->credentials->begin(); it != this->credentials->end(); it++){   
	     contact=*it; 
             result = creamService->proxyRenew(contact, delegationID);
	     if (result.code != 0)
		cout << "TIMER - " << " FAILURE " << result.info << endl;
	     else
		cout << "TIMER - " << " SUCCESS " << endl;
	 }
    }
}

/* 
CreamService class
*/

CreamService::CreamService()
{
    //this->baseAddress = NULL;
    this->connectionTimeout = 300;
    this->certificatePath = "";
}

void CreamService::setPath(string path)
{
    this->certificatePath = path;
}

CreamOperation CreamService::submit(int jid, string contact, string JDL, string delegationID)
{
    CreamOperation result;
    string jidCREAM = "GridWayJob";
    string serviceAddress;
    string creamURL;
    string creamJobId;

    if (JDL == "")
    {
	result.code = -1;
	return result;
    }

    stringstream jidss;
    jidss << jid;
    jidCREAM += jidss.str();

    API::JobDescriptionWrapper jd(JDL, delegationID, "", "", true, jidCREAM);

    API::AbsCreamProxy::RegisterArrayRequest reqs;
    API::AbsCreamProxy::RegisterArrayResult resp;

    reqs.push_back( &jd );

    API::AbsCreamProxy* creamClient = API::CreamProxyFactory::make_CreamProxyRegister( &reqs, &resp, connectionTimeout );

    if (creamClient == NULL)
    {
        delete creamClient;
	result.code = -1;
	result.info = "Error creating Cream client";
	return result; 
    }

    /*if (contact == "" || (contact.size() == 1 && contact.find("-") != string::npos))
        serviceAddress = new string(*(this->baseAddress)+":8443/ce-cream/services/CREAM2");
    else*/ if (contact.find("https") == string::npos)
        serviceAddress = "https://" + (contact)+ ":8443/ce-cream/services/CREAM2";
    else
        serviceAddress = contact;

    try
    {
        creamClient->setCredential(this->certificatePath);
        creamClient->execute(serviceAddress);
    }
    catch(exception& ex)
    {
        delete creamClient;
        result.code = -1;
        result.info = ex.what();
        return result;
    }

    boost::tuple<bool, API::JobIdWrapper, string> registrationResponse = resp[jidCREAM];

    if(registrationResponse.get<0>()!=API::JobIdWrapper::OK)
    {
        delete creamClient;
        result.code = -1;
        result.info = registrationResponse.get<2>();
        return result;
    }

    creamURL  = registrationResponse.get<1>().getCreamURL();
    creamJobId  = registrationResponse.get<1>().getCreamJobID();

    result.job  = new CreamJob(jid, creamJobId, creamURL);

    delete creamClient;
    result.code = 0;
    return result;
}


CreamOperation CreamService::poll(CreamJob creamJob)
{
    string status;
    CreamOperation result;

    API::JobIdWrapper job1((creamJob.getCreamJobId()), (creamJob.getCreamURL()), vector<API::JobPropertyWrapper>());

    vector< API::JobIdWrapper > JobVector;

    JobVector.push_back( job1 );

    string delegationID = "";
    
    int fromDate = -1;
    int toDate   = -1;
    vector<string> statusVec;

    API::JobFilterWrapper jfw( JobVector, statusVec, fromDate, toDate, "", "");

    API::AbsCreamProxy::StatusArrayResult Sresult;

    API::AbsCreamProxy* creamClient = API::CreamProxyFactory::make_CreamProxyStatus( &jfw, &Sresult, connectionTimeout );

    if (creamClient == NULL)
    {
        delete creamClient;
        result.code = -1;
        result.info = "Error creating Cream client";
        return result;
    }

    string serviceAddress = (creamJob.getCreamURL());

    try
    {
        creamClient->setCredential(this->certificatePath);
        creamClient->execute(serviceAddress);
    }
    catch(exception& ex)
    {
        delete creamClient;
        result.code = -1;
        result.info = ex.what();
        return result;
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
            else if (status.compare("CANCELLED") == 0) {
                status = "DONE";
            }
            else if (status.compare("DONE-OK") == 0) {
                status = "DONE";
            }
            else if (status.compare("DONE-FAILED") == 0)
                status = "FAILED";
            else if (status.compare("ABORTED") == 0)
                status = "FAILED";
        }
        else
        {
            delete creamClient;
            result.code = -1;
            result.info = jobIt->second.get<2>();
            return result;
        }
        jobIt++;
   }

   delete creamClient;
   result.code = 0;
   result.info = status;
   return result;
}

CreamOperation CreamService::cancel(CreamJob creamJob)
{
    CreamOperation result;

    API::JobIdWrapper job1((creamJob.getCreamJobId()), (creamJob.getCreamURL()), vector<API::JobPropertyWrapper>());

    vector< API::JobIdWrapper > JobVector;

    JobVector.push_back( job1 );

    int fromDate = -1;
    int toDate   = -1;
    vector<string> statusVec;

    API::JobFilterWrapper jfw( JobVector, statusVec, fromDate, toDate, "", "" );

    API::ResultWrapper resultwrapper;

    API::AbsCreamProxy* creamClient = API::CreamProxyFactory::make_CreamProxyCancel( &jfw, &resultwrapper, this->connectionTimeout );

    if (creamClient == NULL)
    {
        delete creamClient;
        result.code = -1;
        result.info = "Error creating Cream client to cancel job";
        return result;
    }

    string serviceAddress = (creamJob.getCreamURL());

    try
    {
        creamClient->setCredential(this->certificatePath);
        creamClient->execute(serviceAddress);
    }
    catch(exception& ex)
    {
        delete creamClient;
        result.code = -1;
        result.info = ex.what();
        return result;
    }

    delete creamClient;
    result.code = 0;
    return result;
}

CreamOperation CreamService::proxyDelegate(string contact, string delegationID)
{
    CreamOperation result;
    API::AbsCreamProxy *creamClient;
    string serviceAddress;

    // TODO: Delegate only to new CEs (list)
    creamClient = API::CreamProxyFactory::make_CreamProxyDelegate(delegationID, this->connectionTimeout);

    if (creamClient == NULL)
    {
        delete creamClient;
        result.code = -1;
        result.info = "Error creating Cream proxy";
        return result;
    }

    serviceAddress = "https://" + contact + ":8443/ce-cream/services/gridsite-delegation";

    try
    {
        creamClient->setCredential(this->certificatePath);
        creamClient->execute(serviceAddress);
    }
    catch(DelegationException& ex)
    {
        result = this->proxyRenew(contact, delegationID);
        if (result.code != 0)
        {
            delete creamClient;
            result.code = -1;
            result.info = ex.what();
            return result;
        }
    }
    catch(exception& ex)
    {
        delete creamClient;
        result.code = 0;
        result.info = ex.what();
        return result;
    }

    delete creamClient;
    result.code = 0;
    return result;
}

CreamOperation CreamService::proxyRenew(string contact, string delegationID)
{
    CreamOperation result;
    API::AbsCreamProxy *creamClient;
    string serviceAddress;

    // TODO: Renew periodically
    creamClient = API::CreamProxyFactory::make_CreamProxy_ProxyRenew(delegationID, this->connectionTimeout);

    if (creamClient == NULL)
    {
        delete creamClient;
        result.code = -1;
        result.info = "Error renewing Cream proxy";
        return result;
    }

    serviceAddress = "https://" + contact + ":8443/ce-cream/services/gridsite-delegation";

    try
    {
        creamClient->setCredential(this->certificatePath);
        creamClient->execute(serviceAddress);
    }
    catch(exception& ex)
    {
        delete creamClient;
        result.code = -1;
        result.info = ex.what();
        return result;
    }

    delete creamClient;
    result.code = 0;
    return result;
}
