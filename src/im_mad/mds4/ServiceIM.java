/* -------------------------------------------------------------------------- */
/* Copyright 2002-2012, GridWay Project Leads (GridWay.org)                   */
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


import java.io.*;


public class ServiceIM extends Thread 
{
    String  action;
	String  hostList;
	String  serverName;
    String  portNumber;
    String  hid_str;
	String  hostToMonitor;

    boolean flagSucccess = true;
    String info;


    ServiceIM(String action, String hostList, String serverName, String portNumber, String hid_str,String hostToMonitor) 
	{
        this.action        = action;
		this.hostList      = hostList;
	    this.serverName    = serverName;
        this.portNumber    = portNumber;
        this.hid_str       = hid_str;
		this.hostToMonitor = hostToMonitor;
    }

    public void run() 
	{
        // Perform the action
        if (action.equals("DISCOVER"))
		{
            discover();
		}
        else if (action.equals("MONITOR"))
		{
            monitor();
		}
        else
        {
	        synchronized (System.out)
	        {
	                System.out.println(action + " " + hid_str + " FAILURE Not valid action");
	        }
        }

    }

 
    void discover() 
    {
		// We always include the server used for discovery as another resource
		String  info        = serverName+" ";
		boolean flagSuccess = true;
				 
		// Static discovery
		if(!hostList.equals(""))
		{
		    try 
		    {
				BufferedReader in = new BufferedReader(new FileReader(System.getProperty("GW_LOCATION")+"/"+hostList));
				String str;
				while ((str = in.readLine()) != null) 
				{
			        info = info + (str.split(" "))[0] + " ";
				}
				in.close();
		    } 
			catch (IOException e) 
			{
				info = e.getMessage().replace('\n', ' ');
				flagSuccess = false;
			}
		}
		
		// Dynamic discovery
		if( (!serverName.equals("")) && flagSuccess)
		{
			QueryClient qc = new QueryClient(serverName,portNumber);
			String discoverResult = qc.retrieveHosts();
			
			if(!discoverResult.startsWith("FAIL"))
			{
				info = info + discoverResult;
			}
			else
			{
				flagSuccess = false;
				info = discoverResult;
			}
	    }
	
	    synchronized(System.out)
        {
            if(flagSuccess)
			{
				System.out.println("DISCOVER - SUCCESS " + info);	
			}
			else
			{
				System.out.println("DISCOVER - FAILURE " + info);
			}
			
        }
    }

    void monitor() 
    {
		// Info is going to contain the host attributes for hostToMonitor 
		String info = "";
		boolean flagSuccess = true;
			 
		// Static monitoring
		if(!hostList.equals(""))
		{
		    try 
		    {
				String hostAttrFileName = "";

				BufferedReader in = new BufferedReader(new FileReader(System.getProperty("GW_LOCATION")+"/"+hostList));
				String str;
				String[] splitLine;
				
				while ((str = in.readLine()) != null) 
				{
					splitLine = str.split(" ");
			        if ( splitLine[0].startsWith(hostToMonitor) && splitLine.length > 1 )
					{
						hostAttrFileName = splitLine[1];
					}
				}
				in.close();
				
				// Build the pathname for the host attributes file
				//String gwLocation = hostAttrFileName.substring(0,hostList.indexOf("/etc")-1);
				hostAttrFileName = System.getProperty("GW_LOCATION") + "/" + hostAttrFileName;
				// Now that we have the host attributes filename, extract the static info from it
				in = new BufferedReader(new FileReader(hostAttrFileName));
				while ((str = in.readLine()) != null) 
				{
			        info = info + str;
				}
				in.close();
							
		    } 
			catch (IOException e) 
			{
				// Do nothing, dynamic monitoring may be more lucky
				info = "";
			}
		
		}
		
		// Dynamic monitoring

		QueryClient qc = new QueryClient(hostToMonitor,portNumber);
		String discoverResult = qc.retrieveAttributes();

		if(!discoverResult.startsWith("FAIL"))
		{
			info = discoverResult + info;
		}
		else
		{
			flagSuccess = false;
			info = discoverResult;
		}
					

	
	    synchronized(System.out)
        {
            if(flagSuccess)
			{
				System.out.println("MONITOR " + hid_str + " SUCCESS " + info);	
			}
			else
			{
				System.out.println("MONITOR " + hid_str + " FAILURE " + info);
			}
			
        }
    }

}
