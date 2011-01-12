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

#include <iostream>
#include <string>

using namespace std;

int main( int argc, char *argv[] ) 
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
  int status = -1; 
  CreamEmMad *creamEmMad=NULL; 

  while (!end)
  {
	cin.getline(str,4096,'\n');
	
	paramNum = sscanf(str, "%s %s %s %[^\n]", str1, str2, str3, str4);	 

        if (paramNum != 4)
        {
            cout << "FAILURE Not all four arguments defined" << endl;
            continue;
        }

        action = new string(str1);
	jidCREAM = atoi(str2);
	contact = new string(str3);
	jdlFile = new string(str4);

        if (action->compare("INIT") == 0)
	{
            //fopen("grid-proxy-info -path");
	    if (argc == 1)
	    	creamEmMad = new CreamEmMad("GridWay", "/tmp/x509up_u500");
	    else if (argc == 2)
	    	creamEmMad = new CreamEmMad(argv[1], "/tmp/x509up_u500");	
	
            status = creamEmMad->init();
        }
	else if (action->compare("SUBMIT") == 0 && creamEmMad != NULL)
            status = creamEmMad->submit(jidCREAM, contact, jdlFile);    
        //else if (action->compare("RECOVER") == 0 && creamEmMad != NULL)
        //    status = creamEmMad->recover(jidCREAM, contact);
        else if (action->compare("CANCEL") == 0 && creamEmMad != NULL)
            status = creamEmMad->cancel(jidCREAM);
        else if (action->compare("POLL") == 0 && creamEmMad != NULL)
            status = creamEmMad->poll(jidCREAM);      
	else if (end = (action->compare("FINALIZE") == 0) && creamEmMad != NULL)
        {
     	    status = creamEmMad->finalize();
	    return 0;            
        }

        if (creamEmMad == NULL)
           cout << action->c_str() << " " << jidCREAM << " FAILURE " << "first use the command INIT" << endl;
	

        if (status != 0)
           cout << action->c_str() << " " << jidCREAM << " FAILURE " << (creamEmMad->getInfo())->c_str() << endl;
       
	delete action;
	delete contact;
	delete jdlFile;
  }

  return 0;

}


