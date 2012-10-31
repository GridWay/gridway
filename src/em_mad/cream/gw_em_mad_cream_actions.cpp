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
#include <pthread.h>
#include <time.h>

/************* 
CreamJob class
*************/

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

/************************** 
CreamCredentialStatus class
**************************/

CreamCredentialStatus::CreamCredentialStatus(string status)
{
    this->status = status;
    this->connectionTimeout = 30;
    pthread_mutex_init(&credMutex, 0);
    pthread_cond_init(&credCond, 0);
}

string CreamCredentialStatus::getStatus()
{
    return this->status;
}

int CreamCredentialStatus::waitForDelegation()
{
    struct timeval now;
    struct timezone zone;
    struct timespec timeout;
    int retval = -3;
    int rc;

    gettimeofday(&now, &zone);
    timeout.tv_sec = now.tv_sec + connectionTimeout;
    timeout.tv_nsec = now.tv_usec * 1000;

    pthread_mutex_lock(&credMutex);
        rc = pthread_cond_timedwait(&credCond, &credMutex, &timeout);
        if (status.compare("DONE") == 0)
            retval = 0;
        else if (status.compare("FAILED") == 0)
            retval = -1;
        else if (rc == ETIMEDOUT)
            retval = -2;
    pthread_mutex_unlock(&credMutex);

    return retval;
}

void CreamCredentialStatus::setDelegation(string status)
{

    pthread_mutex_lock(&credMutex);
   	this->status = status;
    	if ((status.compare("DONE") == 0) || (status.compare("FAILED") == 0))
            pthread_cond_broadcast(&credCond);
    pthread_mutex_unlock(&credMutex);
}

/*************** 
CreamEmMad class
***************/

CreamEmMad::CreamEmMad(string delegation, int refreshTime)
{
    if (delegation == "")
	this->delegationID = "GridWay";
    else
    	this->delegationID = delegation;
    this->refreshTime = 21600;
    if (refreshTime > 0)
        this->refreshTime = refreshTime;
    this->pollingTime = 30;
    this->creamJobs.clear();
    this->credentials.clear();
    this->creamService = new CreamService();
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
    	creamService->setPath(path);
    pthread_mutex_unlock(&jobMutex);

    cout << "INIT - SUCCESS -" << endl;
    
    return;
}

void CreamEmMad::submit(int jid, string contact, string jdlFile)
{
    if (proxyDelegate("SUBMIT", jid, contact, delegationID) != 0)
        return;

    string JDL = fileToString(jdlFile);
    if ( JDL.empty() )
    {
        cout << "SUBMIT " << jid << " FAILURE " << "Error open the JDL file: " + jdlFile << endl;
        return;
    }

    CreamOperation result = creamService->submit(jid, contact, JDL, delegationID);

    if (result.code == -1)
    {	
	cout << "SUBMIT " << jid << " FAILURE " << result.info << endl;
	return;
    }

    pthread_mutex_lock(&jobMutex); 
        map<int, CreamJob *>::iterator it = creamJobs.find(jid);

	if (it != creamJobs.end())
	{
	    if (it->second != NULL) 
		delete it->second;
	    it->second = result.job;
	}
	else
            creamJobs.insert(pair<int, CreamJob *>(jid, result.job));

        contact = result.job->getCreamURL();
        string creamJobId = result.job->getCreamJobId();   
    pthread_mutex_unlock(&jobMutex); 

    cout << "SUBMIT " << jid << " SUCCESS " << contact.substr(0, contact.find("/ce-cream")) << "/" << creamJobId << endl;  

    return;
}

void CreamEmMad::poll(int jid)
{
    map<int, CreamJob *>::iterator it = creamJobs.find(jid);
 
    if (it == creamJobs.end())
    {
        cout << "POLL " << jid << " FAILURE " << "The job ID does not exist" << endl;
        return;
    }

    pthread_mutex_lock(&jobMutex);
        CreamJob *creamJob = it->second;
	string creamJid = creamJob->getCreamJobId();
	string serviceAddress = creamJob->getCreamURL();
    pthread_mutex_unlock(&jobMutex);

    CreamOperation result = creamService->poll(creamJid, serviceAddress, delegationID); 

    if (result.code == 0)
        cout << "POLL " << jid << " SUCCESS " << result.info << endl;
    else 
        cout << "POLL " << jid << " FAILURE " << result.info << endl;

    if ((result.info.compare("DONE") == 0) || (result.info.compare("FAILED") == 0))
    {
	pthread_mutex_lock(&jobMutex);
	    delete creamJobs.find(jid)->second;
	    creamJobs.erase(jid);
	pthread_mutex_unlock(&jobMutex);
    }

    return;
}

void CreamEmMad::pollAfterCancel(int jid)
{
    map<int, CreamJob *>::iterator it = creamJobs.find(jid);

    if (it == creamJobs.end())
        return;

    pthread_mutex_lock(&jobMutex);
        CreamJob *creamJob = it->second;
        string creamJid = creamJob->getCreamJobId();
        string serviceAddress = creamJob->getCreamURL();
    pthread_mutex_unlock(&jobMutex);

    CreamOperation result = creamService->poll(creamJid, serviceAddress, delegationID);

    if (result.info.compare("DONE") == 0)
        cout << "POLL " << jid << " SUCCESS " << result.info << endl;

    if ((result.info.compare("DONE") == 0) || (result.info.compare("FAILED") == 0))
    {
        pthread_mutex_lock(&jobMutex);
            delete creamJobs.find(jid)->second;
            creamJobs.erase(jid);
        pthread_mutex_unlock(&jobMutex);
    }

    return;
}

void CreamEmMad::cancel(int jid)
{
    map<int, CreamJob *>::iterator it = creamJobs.find(jid);
   
    if (it == creamJobs.end())
    {
        cout << "CANCEL " << jid << " FAILURE " << "The job ID does not exist" << endl;
        return;
    }

    pthread_mutex_lock(&jobMutex);
        CreamJob *creamJob = it->second;
	if (creamJob == NULL)
	{
            pthread_mutex_unlock(&jobMutex);
            return;
	}
        string creamJid = creamJob->getCreamJobId();
        string serviceAddress = creamJob->getCreamURL();
    pthread_mutex_unlock(&jobMutex);

    CreamOperation result = creamService->cancel(creamJid, serviceAddress, delegationID);

    if (result.code == 0)
    {
        cout << "CANCEL " << jid << " SUCCESS -" << endl;  
        sleep(20);
        pollAfterCancel(jid);
    }
    else
        cout << "CANCEL " << jid << " FAILURE " << result.info << endl;

    return;
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
    int rc;

    pthread_mutex_lock(&credentialsMutex);
	map<string, CreamCredentialStatus *>::iterator it = credentials.find(contact);

	if (it != credentials.end())
	{
	    if (it->second->getStatus().compare("DONE") == 0)
	    {
		pthread_mutex_unlock(&credentialsMutex);
		return 0;
	    }
	    else if (it->second->getStatus().compare("PENDING") == 0) 
	    {
                pthread_mutex_unlock(&credentialsMutex);
                rc = it->second->waitForDelegation();
                if (rc == -1)
                {
                    cout << action << " " << jid << " FAILURE " << "Error delegating proxy" << endl;
                    return -1;
                }
                else if (rc == -2)
                {
                    cout << action << " " << jid << " FAILURE " << "Connection timed out" << endl;
                    return -1;
                }
                else if (rc == -3)
                {
                    cout << action << " " << jid << " FAILURE " << "Error while waiting for delegation" << endl;
                    return -1;
                }
                return 0;
            }
	    else if (it->second->getStatus().compare("FAILED") == 0)
		credentials[contact]->setDelegation("PENDING");
	}
        else 
	    credentials.insert(pair<string, CreamCredentialStatus *>(contact, new CreamCredentialStatus("PENDING"))); 
    pthread_mutex_unlock(&credentialsMutex);

    CreamOperation result = creamService->proxyDelegate(contact, delegationID);

    if (result.code != 0)
    {
	cout << action << " " << jid << " FAILURE " << result.info << endl;
	credentials[contact]->setDelegation("FAILED");
	return -1;
    }

    credentials[contact]->setDelegation("DONE");
    return 0;
}

void CreamEmMad::recover(int jid, string contact)
{
    string creamJobId;
    string creamURL;
    CreamJob *creamJob;
    string host;

    size_t pos = contact.find(":8443/");
    host = contact.substr(8, pos-8);

    if (proxyDelegate("RECOVER", jid, host, delegationID) != 0)
        return; 

    pos = contact.find("/CREAM");

    creamURL = contact.substr(0, pos);
    creamURL += "/ce-cream/services/CREAM2";
    creamJobId = contact.substr(pos+1);

    creamJob = new CreamJob(jid, creamJobId, creamURL);
    pthread_mutex_lock(&jobMutex);
        map<int, CreamJob *>::iterator it = creamJobs.find(jid);

        if (it != creamJobs.end())
        {
            if (it->second != NULL)
                delete it->second;
            it->second = creamJob;
        }
        else
            creamJobs.insert(pair<int, CreamJob *>(jid, creamJob));
    pthread_mutex_unlock(&jobMutex);
  
    CreamOperation result = creamService->poll(creamJobId, creamURL, delegationID);

    if ((result.info.compare("DONE") == 0) || (result.info.compare("FAILED") == 0))
    {
        pthread_mutex_lock(&jobMutex);
            delete creamJobs.find(jid)->second;
            creamJobs.erase(jid);
        pthread_mutex_unlock(&jobMutex);
    }

    if (result.code == 0)
        cout << "RECOVER " << jid << " SUCCESS " << result.info << endl;
    else
        cout << "RECOVER " << jid << " FAILURE " << result.info << endl;

    return;
}

string CreamEmMad::fileToString(string jdlFileName)
{
    string jdlString = "";
    ifstream *jdlFile = new ifstream(jdlFileName.c_str());
    char str[4096];

    if (!jdlFile->is_open())
        return "";
 
    while (jdlFile->getline(str,4096,'\n'))
    {
        string aux = str;
        jdlString += aux;
    }

    delete jdlFile;
    return jdlString;
}

void CreamEmMad::timer()
{
    string contact;
    CreamOperation result;

    map<string, CreamCredentialStatus *>::iterator it;
    for (;;)
    {
        sleep(refreshTime);
        for (it=credentials.begin(); it!=credentials.end(); it++)
	{   
	     contact = it->first;
	     if (it->second->getStatus().compare("DONE") == 0)
	     {
                 result = creamService->proxyRenew(contact, delegationID);
	     	 if (result.code != 0)
		 {
		     credentials[contact]->setDelegation("FAILED");
		     cout << "TIMER - FAILURE " << result.info << endl;
		 }
	     	 else
		     cout << "TIMER - SUCCESS -" << endl;
	     }
	 }
    }
}

void CreamEmMad::polling()
{
    int jid;
    multimap<string, string> *result;
    list<string> serviceAddress;
    map<int, CreamJob *>::iterator jit;
    multimap<string, string>::iterator rit;

    for (;;)
    {
         sleep(pollingTime);
         for (jit=creamJobs.begin(); jit!=creamJobs.end(); jit++)
              serviceAddress.push_back(jit->second->getCreamURL());
         serviceAddress.sort();
         serviceAddress.unique();
         while (!serviceAddress.empty())
         {
              result = creamService->callback(serviceAddress.back(), delegationID);
              for (rit=result->begin(); rit!=result->end(); rit++)
              {
                  jid = getJID(rit->first);
                  if (jid != -1)
                  {
                      cout << "CALLBACK " << jid << " SUCCESS " << rit->second << endl;
                      if ((rit->second.compare("DONE") == 0) || (rit->second.compare("FAILED") == 0))
                      {
                          pthread_mutex_lock(&jobMutex);
                              delete creamJobs.find(jid)->second;
                              creamJobs.erase(jid);
                          pthread_mutex_unlock(&jobMutex);
                      }
                  }

              }
              serviceAddress.pop_back();
              delete result;
         }
    }
}

int CreamEmMad::getJID(string creamJID)
{
    map<int, CreamJob *>::iterator it;
    for (it=creamJobs.begin(); it!=creamJobs.end(); it++) 
    {
         if (creamJID.compare(it->second->getCreamJobId()) == 0)
             return it->second->getGridWayID();
    }
    return -1;
}

/***************** 
CreamService class
*****************/

CreamService::CreamService()
{
    this->connectionTimeout = 30;
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

    serviceAddress = "https://" + contact + ":8443/ce-cream/services/CREAM2";

    result = creamClientExecute(creamClient, serviceAddress, contact, delegationID);
    if (result.code != 0)
        return result;

    boost::tuple<bool, API::JobIdWrapper, string> registrationResponse = resp[jidCREAM];

    if(registrationResponse.get<0>()!=API::JobIdWrapper::OK)
    {
        result.code = -1;
        result.info = registrationResponse.get<2>();
        return result;
    }

    creamURL  = registrationResponse.get<1>().getCreamURL();
    creamJobId  = registrationResponse.get<1>().getCreamJobID();

    result.job  = new CreamJob(jid, creamJobId, creamURL);

    return result;
}

CreamOperation CreamService::poll(string creamJid, string serviceAddress, string delegationID)
{
    string status;
    CreamOperation result;

    API::JobIdWrapper job1(creamJid, serviceAddress, vector<API::JobPropertyWrapper>());

    vector< API::JobIdWrapper > JobVector;

    JobVector.push_back( job1 );

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

    size_t pos = serviceAddress.find(":8443/");
    string contact = string(serviceAddress.substr(8, pos-8));

    result = creamClientExecute(creamClient, serviceAddress, contact, delegationID);
    if (result.code != 0)
        return result;

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

	    if ((status.compare("DONE") == 0) || (status.compare("FAILED") == 0))
	    {
		API::ResultWrapper resultWrapper;

		API::AbsCreamProxy* creamClient = API::CreamProxyFactory::make_CreamProxyPurge(&jfw, &resultWrapper, connectionTimeout);
		if (creamClient == NULL)
    		{
        	    delete creamClient;
       	 	    result.code = -1;
        	    result.info = "Error creating Cream client";
        	    return result;
    		}
		creamClientExecute(creamClient, serviceAddress, contact, delegationID);
	    }
            break;
        }
        else
        {
            result.code = -1;
            result.info = jobIt->second.get<2>();
            return result;
        }
        jobIt++;
   }

   result.info = status;
   return result;
}

CreamOperation CreamService::cancel(string creamJid, string serviceAddress, string delegationID)
{
    CreamOperation result;

    API::JobIdWrapper job1(creamJid, serviceAddress, vector<API::JobPropertyWrapper>());

    vector< API::JobIdWrapper > JobVector;

    JobVector.push_back( job1 );

    int fromDate = -1;
    int toDate   = -1;
    vector<string> statusVec;

    API::JobFilterWrapper jfw( JobVector, statusVec, fromDate, toDate, "", "" );

    API::ResultWrapper resultwrapper;

    API::AbsCreamProxy* creamClient = API::CreamProxyFactory::make_CreamProxyCancel( &jfw, &resultwrapper, connectionTimeout);

    if (creamClient == NULL)
    {
        delete creamClient;
        result.code = -1;
        result.info = "Error creating Cream client to cancel job";
        return result;
    }

    size_t pos = serviceAddress.find(":8443/");
    string contact = string(serviceAddress.substr(8, pos-8));

    result = creamClientExecute(creamClient, serviceAddress, contact, delegationID);
    return result;
}

multimap<string, string>* CreamService::callback(string serviceAddress, string delegationID)
{
    multimap<string, string> *jobStatus = new multimap<string, string>;
    CreamOperation result;

    string startEvent;
    string endEvent;
    time_t startTime;
    time_t endTime;
    vector< pair<string, string> > filterStates;
    time_t execTime;
    string dbID;
    list<API::EventWrapper*> resultQueryEvent;

    string lastEventID;
    string status="UNKNOWN";
    string creamJobID;

    size_t pos = serviceAddress.find(":8443/");
    string contact = string(serviceAddress.substr(8, pos-8));
    map<string, string>::iterator it = eventID.find(contact);

    if (it == eventID.end())
    {
        startTime = time(0)-60;
        endTime = time(0);
        startEvent = "0";
        endEvent = "-1";
    }
    else
    {
        startTime = (time_t)-1;
        endTime = (time_t)-1;
        startEvent = it->second;
        endEvent = static_cast<ostringstream*>(&(ostringstream() << atoi(startEvent.c_str())+500))->str();
    }

    API::AbsCreamProxy* creamClient = API::CreamProxyFactory::make_CreamProxy_QueryEvent(make_pair(startEvent,endEvent), make_pair(startTime,endTime), "JOB_STATUS", 500, 0, filterStates, execTime, dbID, resultQueryEvent, connectionTimeout);

    if (creamClient == NULL)
    {
        delete creamClient;
        return jobStatus;
    }

    result = creamClientExecute(creamClient, serviceAddress, contact, delegationID);
    if (result.code != 0)
        return jobStatus;

    map<string, string> properties;
    list<API::EventWrapper*>::const_iterator rit;
    map<string, string>::const_iterator pit;
    pair<multimap<string, string>::iterator, multimap<string, string>::iterator> jsret;
    multimap<string, string>::const_iterator jit;

    for (rit=resultQueryEvent.begin(); rit!=resultQueryEvent.end(); ++rit)
    {
         lastEventID = (*rit)->id; 
         (*rit)->get_event_properties(properties);
         for (pit=properties.begin(); pit!=properties.end(); ++pit)
         {
              if (pit->first == "jobId")
                  creamJobID = pit->second;

              if (pit->first == "type") {
                  status = glite::ce::cream_client_api::job_statuses::job_status_str[atoi(pit->second.c_str())];
                  if (status.compare("REGISTERED") == 0 || status.compare("PENDING") == 0 || status.compare("IDLE") == 0 || status.compare("HELD") == 0)
                      status = "PENDING";
                  else if (status.compare("RUNNING") == 0 || status.compare("REALLY-RUNNING") == 0)
                      status = "ACTIVE";
                  else if (status.compare("CANCELLED") == 0 || status.compare("DONE-OK") == 0) 
                      status = "DONE";
                  else if (status.compare("DONE-FAILED") == 0 || status.compare("ABORTED") == 0)
                      status = "FAILED";

                  jsret = jobStatus->equal_range(creamJobID);
                  for (jit=jsret.first; jit!=jsret.second; jit++)
                      if (jit->second == status)
                          break;
                  if (jit == jsret.second)
                      jobStatus->insert(pair<string, string>(creamJobID, status));

              }
         }
         properties.clear();
    }
    resultQueryEvent.clear();
    filterStates.clear();

    it = eventID.find(contact);
    if (it == eventID.end() && !lastEventID.empty())
        eventID.insert(pair<string, string>(contact, static_cast<ostringstream*>(&(ostringstream() << atoi(lastEventID.c_str())+1))->str()));
    else if (it != eventID.end() && !lastEventID.empty())
        it->second = static_cast<ostringstream*>(&(ostringstream() << atoi(lastEventID.c_str())+1))->str();

    return jobStatus;
}

CreamOperation CreamService::proxyDelegate(string contact, string delegationID)
{
    CreamOperation result;
    API::AbsCreamProxy *creamClient;
    string serviceAddress;

    creamClient = API::CreamProxyFactory::make_CreamProxyDelegate(delegationID, connectionTimeout);

    if (creamClient == NULL)
    {
        delete creamClient;
        result.code = -1;
        result.info = "Error creating Cream proxy";
        return result;
    }

    serviceAddress = "https://" + contact + ":8443/ce-cream/services/gridsite-delegation";

    result = creamClientExecute(creamClient, serviceAddress, contact, delegationID);

    return result;
}

CreamOperation CreamService::proxyRenew(string contact, string delegationID)
{
    CreamOperation result;
    API::AbsCreamProxy *creamClient;
    string serviceAddress;

    creamClient = API::CreamProxyFactory::make_CreamProxy_ProxyRenew(delegationID, connectionTimeout);

    if (creamClient == NULL)
    {
        delete creamClient;
        result.code = -1;
        result.info = "Error renewing Cream proxy";
        return result;
    }

    serviceAddress = "https://" + contact + ":8443/ce-cream/services/gridsite-delegation";

    result = creamClientExecute(creamClient, serviceAddress, contact, delegationID);

    return result;
}

CreamOperation CreamService::creamClientExecute(API::AbsCreamProxy* creamClient, string serviceAddress, string contact, string delegationID)
{
    CreamOperation result;
    bool renewedProxy = false;
    bool end = false; 

    while (!end)
    {
        try
        {
            creamClient->setCredential(certificatePath);
            creamClient->execute(serviceAddress);
            end = true;
        }
        catch(DelegationException &ex)
        {
	    if ((renewedProxy == false) & (typeid(*creamClient) != typeid(API::CreamProxy_ProxyRenew)))
		result = proxyRenew(contact, delegationID);

	    if ((renewedProxy == true) | (typeid(*creamClient) == typeid(API::CreamProxy_ProxyRenew)) | (result.code == -1))
    	    {
        	delete creamClient;
        	result.code = -1;
        	result.info = ex.what();
        	return result;
	    }

            if (typeid(*creamClient) == typeid(API::CreamProxy_Delegate))
                end = true;

	    renewedProxy = true; 
	}
        catch(exception &ex)
	{
	    delete creamClient;
            result.code = -1;
            result.info = ex.what();
	    return result;
	}
    }
    
    delete creamClient;
    result.code = 0;
    return result;
}
