/* -------------------------------------------------------------------------- */
/* Copyright 2002-2011, GridWay Project Leads (GridWay.org)                   */
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

/*
import java.util.*;
import java.net.URL;

*/

class Gw_im_mad_mds4 extends Thread 
{

    private String  portNumber;
    private String  serverName;
    private String  hostList;

    // Entry point - main procedure
    
    public static void main(String args[]) 
    {
    	int i = 0;
        String arg;
        String _portNumber = "8443";
 	  	String _serverName = "";
 	 	String _hostList = "";
        boolean error=true;
        Gw_im_mad_mds4 gw_im_mad_mds4;
        
        while (i < args.length && args[i].startsWith("-")) 
        {
            arg = args[i++];
            
            if (arg.equals("-p")) 
            {
                if (i < args.length)
			 	{
               	 _portNumber = args[i++];
			 	}
                else
                {
                    System.err.println("-p requires a portnumber");
                }
            }
            if (arg.equals("-s")) 
            {
            	if (i < args.length)
				{
              	    _serverName = args[i++];
					error=false;	
				}
                else
                {
                    System.err.println("-s requires a server hostname");
                }
            }
            if (arg.equals("-l")) 
            {
                if (i < args.length)
			 	{
               		_hostList = args[i++];
					error=false;	
		 		}
                else
                {
                    System.err.println("-l requires the path to a host list");
                }
            }
        }


        if (!error)
        {
            gw_im_mad_mds4 = new Gw_im_mad_mds4(_portNumber, _serverName, _hostList);    	
            gw_im_mad_mds4.loop();
        }
		else
		{
			System.out.println("java Gw_im_mad_mds4 [-s SERVER] [-l HOSTLIST] [-p PORTNUMBER]\n");
		}
    }

    // Constructor
    Gw_im_mad_mds4(String portNumber,String serverName,String hostList) 
    {
    	this.portNumber = portNumber;
    	this.serverName = serverName;
    	this.hostList   = hostList;
    }

    void loop() 
    {
        String str    = null;
        String action  = null;
        String host;
        String args_str;
        String hid_str = null;
		String hostToMonitor;
        boolean fin = false;

        BufferedReader in = new BufferedReader(new InputStreamReader(System.in));

        while (!fin) 
	    {
            // Read a line a parse it
            try
            {
                str = in.readLine();
            }
            catch (IOException e)
            {
                String message = e.getMessage().replace('\n', ' ');

                synchronized (System.out)
                {
                    System.out.println(action + " " + hid_str + " FAILURE " + message);
                }
            }

            String str_split[] = str.split(" ", 4);

            if (str_split.length != 4)
            {
                synchronized (System.out)
                {
                    System.out.println("- - FAILURE Unknown command");
                }
            }
            else
            {
                action        = str_split[0].toUpperCase();
                hid_str       = str_split[1];
                hostToMonitor = str_split[2];
				args_str      = str_split[3];
             	
				// Perform the action
                if (action.equals("INIT"))
                    init();
                else if (action.equals("FINALIZE"))
                {
                    finalize_mad();
                    fin = true;
                }
                else if (action.equals("DISCOVER") || action.equals("MONITOR"))
                {
 					ServiceIM s = new ServiceIM(action,hostList,serverName,portNumber,hid_str,hostToMonitor);
                    s.start();
				}
            }
        }
    }

    void init() 
    {
        // Nothing to do here
        synchronized(System.out)
        {
            System.out.println("INIT - SUCCESS -");
        }
    }

    void finalize_mad() 
    {
        // Nothing to do here
        synchronized(System.out)
        {
            System.out.println("FINALIZE - SUCCESS -");
        }
    }
}

