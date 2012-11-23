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

#ifndef CREAMEMMAD_H_
#define CREAMEMMAD_H_
#define MAX_THREADS 1024

/* 
CREAM CLIENT API C++ includes
*/

#include <glite/ce/cream-client-api-c/CreamProxyFactory.h>
#include <glite/ce/cream-client-api-c/JobDescriptionWrapper.h>
#include <glite/ce/cream-client-api-c/JobFilterWrapper.h>
#include <glite/ce/cream-client-api-c/JobStatusWrapper.h>
#include <glite/ce/cream-client-api-c/job_statuses.h>  
/**
  C++ STL includes
*/
#include <unistd.h>
#include <sys/wait.h>
#include <iostream>
#include <string> 
#include <map>
#include <vector>
#include <ctime> 
#include <fstream> 
#include <sstream>

/** 
  boost includes
*/
#include <boost/tuple/tuple.hpp>

using namespace std;
  
namespace API = glite::ce::cream_client_api::soap_proxy;

class CreamJob
{
    private:
	int gridwayID;
        string creamJobId;
        string creamURL;
   
    public:
	CreamJob(int gridwayID, string creamJobId, string creamURL);
	int getGridWayID();
	string getCreamJobId();
	string getCreamURL();
};

struct CreamOperation
{ 
  	int code; 
  	string info;
   	CreamJob *job;
};

class CreamService
{
    private:
        int connectionTimeout;
        string certificatePath;
        map <string, string> eventID;

    public:
	CreamService();
	void setPath(string path);
        void proxyDelegate(string contact, string delegationID);
        int proxyRenew(string contact, string delegationID);
        CreamOperation submit(int jid, string contact, string JDL, string delegationID);
	CreamOperation poll(string creamJid, string serviceAddress);
	CreamOperation cancel(string creamJid, string serviceAddress);
	CreamOperation creamClientExecute(API::AbsCreamProxy* creamClient, string serviceAddress);
        multimap<string, string>* callback(string serviceAddress);
};

class CreamEmMad
{
    private:
	int jid;
	string delegationID;
	map <int, CreamJob*> creamJobs;	
	pthread_mutex_t jobMutex;
        pthread_mutex_t credentialsMutex;
        pthread_cond_t credCond;
        map<string, int> credentials;
        struct timespec timeout;
        int refreshTime;
        int pollingTime;
	CreamService *creamService;
        void proxyDelegate(string contact, string delegationID);
	string fileToString(string jdlFileName);
        int getJID(string creamJID);

    public:
	CreamEmMad(string delegation, int refreshTime);
	void init();
	void submit(int jid, string contact, string jdlFile);
	void recover(int jid, string contact);
	void cancel(int jid);
	void poll(int jid);
	void pollAfterCancel(int jid);
	void finalize();
        void timer();
        void polling();
};

#endif /*CREAMEMMAD_H_*/
