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

#include "gw_em_mad_cream.h"

CreamJob::CreamJob(int gridwayID, string *creamID, string *contact)
{
  this->gridwayID = gridwayID;
  this->creamID   = new string(*creamID);
  this->contact   = new string(*contact);
}

void CreamJob::setGridWayID(int gridwayID)
{
  this->gridwayID = gridwayID;
}

void CreamJob::setCreamID(string *creamID)
{
  this->creamID = new string(*creamID);
}

void CreamJob::setContact(string *contact)
{
  this->contact = new string(*contact);
}

void CreamJob::setInputFiles(vector<string> *inputFiles)
{
  this->inputFiles = inputFiles;
}

void CreamJob::setIsbUploadUrl(string isbUploadUrl)
{
  this->isbUploadUrl = isbUploadUrl;
}
 
int CreamJob::getGridWayID()
{
  return this->gridwayID;
}

string *CreamJob::getCreamID()
{
  return this->creamID;
}

string *CreamJob::getContact()
{
  return this->contact;
}

vector <string> *CreamJob::getInputFiles()
{
  return this->inputFiles;
}

string  CreamJob::getIsbUploadUrl()
{
  return this->isbUploadUrl;
}
	
CreamEmMad::CreamEmMad(char *delegation)
{
    this->delegationID = new string(delegation);	
    this->info = NULL;
    this->baseAddress = NULL;
    this->localCreamJID = NULL;
    this->creamJobs = new map <int, CreamJob> ();
}

int CreamEmMad::init()
{
    FILE *file;
    char path[512], *pline;

    file = popen("grid-proxy-info -path", "r");

    if (file == NULL)
    {
        cout << "INIT - FAILURE Error getting proxy path" << endl;
        return -1;
    }

    while( fgets(path, sizeof(path), file) != NULL )
    {
        // Keep looping even if we just expect one line
    }

    pclose(file);

    if (path == NULL)
    {
        cout << "INIT - FAILURE Error reading proxy path" << endl;
        return -1;
    }

    pline =  strchr(path, '\n');
    if (pline != NULL)
        *pline = '\0';

    this->certificatePath = new string(path);

    cout << "INIT - SUCCESS -" << endl;
    
    return 0;
}

int CreamEmMad::submit(int jid, string *contact, string *jdlFile)
{
    // Delegation should be once per CE!!!
    if (this->proxyDelegate(contact) != 0)
      return -1;
      
    CreamJob *job = this->jobSubmit(jid, contact, jdlFile);

    if (job == NULL)
        return -1;

    // Make staging with GASS!!! And register job with autostart!!!
    /*if (this->stagingInputFiles(job) != 0)
        return -1;

    if (this->jobStart(job) != 0)
        return -1;*/
    contact = job->getContact();
    
    cout << "SUBMIT " << jid << " SUCCESS " << contact->substr(0, contact->find("/ce-cream")) << "/" << *(job->getCreamID()) << endl;  
    
    return 0;
}

int CreamEmMad::poll(int jid)
{
    string status;
 
    map<int,CreamJob>::iterator it = this->creamJobs->find(jid);
 
    if (it == creamJobs->end())
    {
        this->info = new string ("The job ID do not exist");
        return -1;
    }

    CreamJob creamJob = it->second;
   
    API::JobIdWrapper job1(*(creamJob.getCreamID()), *(creamJob.getContact()), vector<API::JobPropertyWrapper>());

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
        this->info = new string("Error creating Cream client");
        return -1;
    }

    string serviceAddress = *(creamJob.getContact());

    try 
    {
        creamClient->setCredential(this->certificatePath->c_str());
        creamClient->execute(serviceAddress);
    }
    catch(exception& ex) 
    {
        this->info = new string(ex.what());
        delete creamClient;
        return -1;
    }

    map<string, boost::tuple<API::JobStatusWrapper::RESULT, API::JobStatusWrapper, std::string> >::const_iterator jobIt = Sresult.begin();

    while( jobIt != Sresult.end() ) 
    {
        if( jobIt->second.get<0>() == API::JobStatusWrapper::OK ) 
	{
	    status = jobIt->second.get<1>().getStatusName();
            if (status.compare("IDLE") == 0)
                status = "ACTIVE IDLE";
            else if (status.compare("RUNNING") == 0)
                status = "ACTIVE RUNNING";
            else if (status.compare("REALLY-RUNNING") == 0)
	        status = "ACTIVE RRUNNING";
            else if (status.compare("DONE-OK") == 0)
                status = "DONE";
    
            cout << "POLL " << jid << " SUCCESS " << status << endl;
        }
        else 
        {
            this->info = new string(jobIt->second.get<2>());
            delete creamClient;
            return -1;    
	}
     	jobIt++;
   }
 
   delete creamClient;

   return 0;	
}

int CreamEmMad::cancel(int jid)
{
   map<int,CreamJob>::iterator it = this->creamJobs->find(jid);
   
   if (it == creamJobs->end())
   {
	this->info = new string ("The job ID  not exists");
	return -1;
   }
  
   CreamJob creamJob = it->second;
   
   API::JobIdWrapper job1(*(creamJob.getCreamID()), *(creamJob.getContact()), vector<API::JobPropertyWrapper>());

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
	this->info = new string("Error creating Cream client to cancel job");
	return -1;
   }

   string serviceAddress = *(creamJob.getContact());

   try 
   {
    	creamClient->setCredential(this->certificatePath->c_str());
    	creamClient->execute(serviceAddress);
   } 
   catch(exception& ex) 
   {
	this->info = new string(ex.what());
    	delete creamClient;
    	return -1;
   }

   cout << "CANCEL " << jid << " SUCCESS" << endl;  
   delete creamClient;

   return 0;
}
	

int CreamEmMad::finalize()
{
    cout << "FINALIZE SUCCESS - -" << endl;
    return 0;
}

string *CreamEmMad::getInfo()
{
    return this->info;
}		

int CreamEmMad::proxyDelegate(string *contact)
{
    API::AbsCreamProxy 	*creamClient; 
    string	    	*serviceAddress;

    creamClient = API::CreamProxyFactory::make_CreamProxy_ProxyRenew(*(this->delegationID), this->connectionTimeout);
    //creamClient = API::CreamProxyFactory::make_CreamProxyDelegate(*(this->delegationID), this->connectionTimeout);

    if (creamClient == NULL)
    {
        this->info = new string ("Error creating Cream proxy");
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
        this->info = new string(ex.what());
        delete creamClient;
        return -1;
    }

    delete creamClient;
 
    return 0;
}

CreamJob *CreamEmMad::jobSubmit(int jid,string *contact,string *jdlFile)
{
    string *JDL = this->fileToString(jdlFile);
    string *jidCREAM = new string("GridWayJob");	
    stringstream jidString;
    size_t found;
    string *serviceAddress;	
    string *creamURL;
    string *creamJID;
    CreamJob *creamJob;	
    vector<string> *inputFiles;

    if (JDL == NULL)
        return NULL;
    else
        inputFiles=this->getInputFiles(JDL);
	
    jidString << jid;

    *jidCREAM += jidString.str();

    API::JobDescriptionWrapper jd(*JDL, *(this->delegationID), "", "", true, jidCREAM->c_str());

    API::AbsCreamProxy::RegisterArrayRequest reqs;
    API::AbsCreamProxy::RegisterArrayResult resp;

    reqs.push_back( &jd );
     
    API::AbsCreamProxy* creamClient = API::CreamProxyFactory::make_CreamProxyRegister( &reqs, &resp, connectionTimeout );
     
    if (creamClient == NULL)
    {
        this->info = new string("Error creating Cream client");
        return NULL;
    }

    if (contact == NULL || (contact->size() == 1 && (found = contact->find("-")) != string::npos))
        serviceAddress = new string(*(this->baseAddress)+":8443/ce-cream/services/CREAM2");
    else if ((found = contact->find("https")) == string::npos)
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
        this->info = new string(ex.what());
        delete creamClient;
        return NULL;
    }

    boost::tuple<bool, API::JobIdWrapper, string> registrationResponse = resp[jidCREAM->c_str()];
       
    if(registrationResponse.get<0>()!=API::JobIdWrapper::OK) 
    {
        this->info = new string(registrationResponse.get<2>());
        delete creamClient;
        return NULL;
    }  

    creamURL  = new string(registrationResponse.get<1>().getCreamURL());
    creamJID  = new string(registrationResponse.get<1>().getCreamJobID());
    creamJob  = new CreamJob(jid, creamJID, creamURL);

    map< string, string > properties;

    registrationResponse.get<1>().getProperties( properties );
    string ISB_upload_url = properties["CREAMInputSandboxURI"];
    creamJob->setIsbUploadUrl(ISB_upload_url);
    creamJob->setInputFiles(inputFiles);  
  
    this->creamJobs->insert(pair<int, CreamJob>(jid, *creamJob));

    delete creamClient;
    
    return creamJob;
}

string *CreamEmMad::fileToString(string *jdlFileName)
{
   string 	*jdlString = new string("");
   ifstream	*jdlFile = new ifstream(jdlFileName->c_str());
   char		str[4096];

   if (!jdlFile->is_open())
   {
	this->info = new string("Error open the JDL file: " + *jdlFileName );
	return NULL;
   }
    
   while (jdlFile->getline(str,40096,'\n'))
   {
	string *aux = new string(str);
	(*jdlString) += *aux ;       	   
   }   	
  
   return jdlString;
}

vector<string> *CreamEmMad::getInputFiles(string *jdlString)
{
    size_t   begin;
    size_t   end;
    string   subString; 
    string   inputFile;		
    vector<string> *inputFiles = new vector<string>();

    begin = jdlString->find("InputSandbox", 0);

    if (begin != string::npos)	
    {
	subString=jdlString->substr(begin, jdlString->size());

    	begin = subString.find("\"",0);
    	end = subString.find(",",0);
	
        if (end == string::npos)
	    end = subString.find("}",0);
	    
	while (begin!=string::npos && end!=string::npos)
	{
   	    inputFile=subString.substr(begin+1, end-begin-2);
	    inputFiles->push_back(inputFile);	

	    subString = subString.substr(end+1);
	    begin = subString.find("\"",0);
    	    end = subString.find(",",0);
            
	    if (end == string::npos)
	    	end = subString.find("}",0);
	}
    }
   
    return inputFiles;
}

int CreamEmMad::stagingInputFiles(CreamJob *job)
{
    int status;
    string *destination;
    string *source;
    vector<string> inputFiles = *(job->getInputFiles());

    for (int i=0; i<(int) inputFiles.size(); i++)
    {	
    	if (fork() == 0)
	{
	    source = new string("file://" + inputFiles[i]);
	    destination = new string(job->getIsbUploadUrl() + "/"  + basename(inputFiles[i].c_str()));
	    
	    execlp ("globus-url-copy", "globus-url-copy", source->c_str(), destination->c_str(), NULL);
      	    this->info = new string("Error executing glubus-url-copy");
	    return -1;
    	}
	else
	    wait(&status);
    }

    return 0; 	       
}	

int CreamEmMad::jobStart(CreamJob *job) 
{
   string creamURL = *(job->getContact());
   string localCreamJID1 = *(job->getCreamID());
   API::JobIdWrapper job1(localCreamJID1, creamURL, vector<API::JobPropertyWrapper>() );
   	
   vector< API::JobIdWrapper > JobVector;
   JobVector.push_back( job1 );

   int fromDate = -1;
   int toDate   = -1;
   vector<string> statusVec;

   API::JobFilterWrapper jfw( JobVector, statusVec, fromDate, toDate, *(this->delegationID), "" );

   int connection_timeout = 30; // seconds

   API::ResultWrapper result;

   API::AbsCreamProxy* creamClient = API::CreamProxyFactory::make_CreamProxyStart( &jfw, &result, connection_timeout );
  
   if (creamClient == NULL)
   {
	this->info = new string("Error creating Cream client");
	return -1;
   }

   try 
   {
    	creamClient->setCredential( this->certificatePath->c_str() );
    	creamClient->execute(*(job->getContact()));   
   } 
   catch(exception& ex) 
   {
	this->info = new string(ex.what());
    	delete creamClient;
    	return -1;
   }
 
   delete creamClient;

   return 0;
}
