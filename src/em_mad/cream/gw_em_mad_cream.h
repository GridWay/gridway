/* -------------------------------------------------------------------------- */
/* Copyright 2002-2010 GridWay Project Leads (GridWay.org)                    */
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

/* 
CREAM CLIENT API C++ includes
*/

#include <glite/ce/cream-client-api-c/CreamProxyFactory.h>
#include <glite/ce/cream-client-api-c/JobDescriptionWrapper.h>
#include <glite/ce/cream-client-api-c/JobFilterWrapper.h>
#include <glite/ce/cream-client-api-c/JobStatusWrapper.h>

  
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
	string *creamJobId;
        string *creamURL;
   
  public:
	CreamJob(int gridwayID, string *creamJobId, string *creamURL);
	void setGridWayID(int gridwayID);
	void setCreamJobId(string *creamJobId);
	void setCreamURL(string *creamURL);
	int getGridWayID();
	string *getCreamJobId();
	string *getCreamURL();
};

class CreamEmMad
{
    private:
	int jid;
	int connectionTimeout;
	string *info;
	string *delegationID;
	string *baseAddress;
	string *localCreamJID;
	string *certificatePath;
	map <int, CreamJob> *creamJobs;	
	
	int proxyDelegate(string *contact);
	int proxyRenew(string *contact);
	CreamJob *jobSubmit(int jid, string *contact, string *jdlFile);	
	int stagingInputFiles(CreamJob *job);
	int jobStart(CreamJob *job);	
	string *fileToString(string *jdlFileName); 
	vector <string> *getInputFiles(string *jdlString);

    public:
	CreamEmMad(char *delegation);
	int init();
	int submit(int jid, string *contact, string *jdlFile);
	int recover(int jid, string *contact);
	int cancel(int jid);
	int poll(int jid);
	int finalize();
	string *getInfo();
};

#endif /*CREAMEMMAD_H_*/

