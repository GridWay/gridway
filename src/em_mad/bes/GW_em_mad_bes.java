/* -------------------------------------------------------------------------- */
/* Copyright 2002-2012 GridWay Team, Distributed Systems Architecture         */
/* Group, Universidad Complutense de Madrid                                   */
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
import java.util.*;
import javax.xml.rpc.ServiceException;

class GW_em_mad_bes extends Thread {

	static String usage =
                "USAGE\n GW_em_mad_bes [-h] [-t] \n\n" +
		"SYNOPSIS\n" +
		"  Execution driver to interface with BES. It is not intended to be used from CLI.\n\n" +
		"OPTIONS\n" +
		"  -h    print this help" +
                "  -t    target BES implementation (gridsam | unicore)";
	static String susage =
		"usage: GW_em_mad_bes [-h] [-t]";

    	private Map job_pool = null; // Job pool
    	private Map jid_pool = null; // JID pool

    	private String host;
    	private Calendar terminationTime = null;
        private String target;
	
    	public static void main(String args[]) {
        	GW_em_mad_bes gw_em_mad_bes;
    		int i = 0;
		String arg;
                String target = "gridsam";

         	while (i < args.length && args[i].startsWith("-")) 
         	{
             		arg = args[i++];

             		if (arg.equals("-h")) 
             		{
				System.out.println(usage);
                        	System.exit(0);
			}
                        if (arg.equals("-t"))
                        {
                                if (i < args.length)
                                    target = args[i++];
                                else
                                {
                                    System.err.println("-t requires a target BES implementation (gridsam | unicore)");
                                    System.out.println(susage);
                                    System.exit(1);
                                }
                        }
			else
			{
                        	System.err.println("error: invalid option \'" + arg + "\'\n");
                        	System.out.println(susage);
                        	System.exit(1);
			}
		}

        	gw_em_mad_bes = new GW_em_mad_bes(target);
        	gw_em_mad_bes.loop();
    	}

	GW_em_mad_bes(String target) {
		// Create the job and JID pool
		job_pool = Collections.synchronizedMap(new HashMap());
                this.target = target;
	}

	void loop() {
		String str = null;
		String action = null;
		String jid_str = null;
		String contact;
		String jsdl_file = null;
		boolean end = false;
		Integer jid = null;

		BufferedReader in = new BufferedReader(new InputStreamReader(System.in));
		while (!end) {
			// Read a line a parse it
			try
			{
				str = in.readLine();
			}
			catch (IOException e)
			{
				String info = e.getMessage().replace('\n', ' ');

				synchronized (System.out)
				{
                                        System.out.println(" - - FAILURE " + info);
				}
			}

			String str_split[] = str.split(" ", 4);

			if (str_split.length != 4)
			{
				synchronized (System.out)
				{
                                        System.out.println(" - - FAILURE Error reading line");
				}
			}
			else
			{
				action = str_split[0].toUpperCase();
				jid_str = str_split[1];
	 			contact = str_split[2];
				jsdl_file = str_split[3];
                 	
				// Perform the action
				if (action.equals("INIT"))
					init();
				else if (action.equals("FINALIZE"))
				{
					finalize_mad();
					end = true;
				}
				else if (action.equals("SUBMIT"))
				{
                                        try
                                        {
                                                jid = new Integer(jid_str);
					 	if (str_split[2].indexOf('/') != -1)
                                               		host = str_split[2].substring(0,str_split[2].indexOf('/'));
                                                if (target.equals("unicore"))
                                                {
                                                    String gateway = "";
                                                    if (str_split[2].indexOf('/') != str_split[2].lastIndexOf('/'))
                                                        gateway = str_split[2].substring(str_split[2].indexOf('/'),str_split[2].lastIndexOf('/'));
                                                    contact = "https://" + host + ":8080/" + gateway + "/services/BESFactory?res=default_bes_factory";
                                                }
                                                else // Assuming GridSAM
                                                        contact = "https://" + host + ":8443/gridsam/services/bes?wsdl";
                                                submit(jid, contact, jsdl_file);
                                        }
                                        catch (NumberFormatException e)
                                        {
                                                String info = e.getMessage().replace('\n', ' ');

                                                synchronized (System.out)
                                                {
                                                        System.out.println(action + " " + jid_str + " FAILURE " + info);
                                                }       
                                        }
				}
                                else if (action.equals("RECOVER"))
				{
                                        try
                                        {
                                                jid = new Integer(jid_str);
						recover(jid, contact);
                                        }
                                        catch (NumberFormatException e)
                                        {
                                                String info = e.getMessage().replace('\n', ' ');

                                                synchronized (System.out)
                                                {
                                                        System.out.println(action + " " + jid_str + " FAILURE " + info);
                                                }       
                                        }
				}
				else if (action.equals("CANCEL"))
				{
                                        try
                                        {
                                                jid = new Integer(jid_str);
						cancel(jid);
                                        }
                                        catch (NumberFormatException e)
                                        {
                                                String info = e.getMessage().replace('\n', ' ');

                                                synchronized (System.out)
                                                {
                                                        System.out.println(action + " " + jid_str + " FAILURE " + info);
                                                }       
                                        }
				}
				else if (action.equals("POLL"))
				{
                                        try
                                        {
                                                jid = new Integer(jid_str);
						poll(jid);
                                        }
                                        catch (NumberFormatException e)
                                        {
                                                String info = e.getMessage().replace('\n', ' ');

                                                synchronized (System.out)
                                                {
                                                        System.out.println(action + " " + jid_str + " FAILURE " + info);
                                                }       
                                        }

				}
				else
				{
					synchronized(System.out)
					{
						System.out.println(action + " " + jid_str + " FAILURE Not valid action");
					}
				}
			}
		}
	}


	void init() {
		// Nothing to do here
		synchronized(System.out)
		{
			System.out.println("INIT - SUCCESS -");
		}
	}

	void finalize_mad() {
		// Nothing to do here
		synchronized(System.out)
		{
			System.out.println("FINALIZE - SUCCESS -");
		}
	}

        void submit(int jid, String contact, String jsdl_file) {

		String info = null;
		int status = 0;

                // Submit the job
                try
                {
                        if (target.equals("unicore"))
                        {
                            String jsdl_username = add_username(jid, jsdl_file);
                            if (jsdl_username != null){
                                ServiceBES job = new ServiceBES(jid, contact, jsdl_username);
                                status = job.submit(target);
                                info = job.getInformation();
                                // Add job and jid to the pools
                                synchronized (this)
                                {
                                    job_pool.put(jid, job);
                                }
                                job = null;
                            }

                        }
                        else if (target.equals("gridsam"))
                        {
			    String jsdl_myproxy = add_myproxy(jid, jsdl_file);
			    if (jsdl_myproxy != null){
                                ServiceBES job = new ServiceBES(jid, contact, jsdl_myproxy);
                                status = job.submit(target);
			        info = job.getInformation();
                                // Add job and jid to the pools
			        synchronized (this) 
			        {
	                            job_pool.put(jid, job);
				}
                                job = null;
                            }
			}
			else
				return;
                }
                catch (Exception e)
                {
                        info = e.getMessage().replace('\n', ' ');
                        status = -1;
                }
                synchronized (System.out)
                {
                        if (status == 0)
                                System.out.println("SUBMIT" + " " + jid + " SUCCESS " + info);
                        else
                                System.out.println("SUBMIT" + " " + jid + " FAILURE " + info);
                }
        }

        void recover(int jid, String contact) {

                String info;
                int status;
                // Recover the job
                try
                {
			ServiceBES job = new ServiceBES(jid, contact, null);
                        status = job.recover();
			info = job.getInformation();
                        // Add job and jid to the pools
			synchronized (this)
			{
                        	job_pool.put(jid, job);
			}
                        job = null;
                }
                catch (Exception e)
                {
                        info = e.getMessage().replace('\n', ' ');
                        status = -1;
                }
                synchronized (System.out)
                {
                        if (status == 0)
                                System.out.println("RECOVER" + " " + jid + " SUCCESS " + info);
                        else
                                System.out.println("RECOVER" + " " + jid + " FAILURE " + info);
                }
        }

        void cancel(int jid) {

                String info = null;
                int status = 0;
                // Cancel the job
                try
                {
                  	ServiceBES job = (ServiceBES) job_pool.get(jid);
                        if (job == null)
                   	{
                                status = -1;
                                info = "Job does not exist jid";
                   	}
                   	else
                        {
				status = job.cancel();
				info = job.getInformation();
				if (status == 0)
				{
					sleep(20);
					pollAfterCancel(jid);
				}
                        }
                        job = null;
                }
                catch (Exception e)
                {
                        info = e.getMessage().replace('\n', ' ');
                        status = -1;
                }
                synchronized (System.out)
                {
                        if (status == 0)
                               System.out.println("CANCEL" + " " + jid + " SUCCESS " + info);
                        else
                               System.out.println("CANCEL" + " " + jid + " FAILURE " + info);
                }

        }

        void pollAfterCancel(int jid) {

                String info = null;
                int status = 0;
                try
                {
                        ServiceBES job = (ServiceBES) job_pool.get(jid);
                        if (job == null)
				return;
                        else
                        {
                                status = job.poll();
                                info = job.getInformation();
                                if (info.equals("DONE") || info.equals("FAILED"))
                                {
                                    synchronized (this)
                                    {
                                        job_pool.remove(jid);
                                    }
                                }
                        }
                        job = null;
                }
                catch (Exception e)
                {
                        info = e.getMessage().replace('\n', ' ');
                        status = -1;
                }
                synchronized (System.out)
                {
                        if (info == "DONE")
                                System.out.println("POLL" + " " + jid + " SUCCESS " + info);
                }
        }

        void poll(int jid) {

                String info = null;
                int status = 0;
                try
                {
                        ServiceBES job = (ServiceBES) job_pool.get(jid);
                        if (job == null)
                        {	
				status = -1;
				info = "Job does not exist jid";
                        }
                        else
                        {
				status = job.poll();
				info = job.getInformation();
                                if (info.equals("DONE") || info.equals("FAILED"))
                                {
                                    synchronized (this)
                                    {
                                        job_pool.remove(jid);
                                    }
                                }
                        }
                        job = null;
                }
                catch (Exception e)
                {
                        info = e.getMessage().replace('\n', ' ');
                        status = -1;
                }
                synchronized (System.out)
                {
                        if (status == 0)
                                System.out.println("POLL" + " " + jid + " SUCCESS " + info);
                        else
                                System.out.println("POLL" + " " + jid + " FAILURE " + info);
                }
        }

	String add_myproxy(int jid, String jsdl_filename) {
		String myproxy_filename;
		String myproxy_server = null;
		String user = null;
    		String passwd = null;
		String myproxy_buffer;
  		StringBuffer sb = new StringBuffer();
		String line;
		String jid_str = String.valueOf(jid);

		myproxy_filename = System.getenv("HOME") + "/.myproxy";
        	try
        	{
                	BufferedReader in = new BufferedReader(new FileReader(myproxy_filename));
			myproxy_server = in.readLine();
			user = in.readLine();
			passwd = in.readLine();
			in.close();
        	} catch (IOException e){
                	String info = e.getMessage().replace('\n', ' ');
                      	synchronized (System.out)
                        {
                        	System.out.println("SUBMIT" + " " + jid_str + " FAILURE " + info);
                        }
			return null;
		}

    		myproxy_buffer = " <MyProxy xmlns=\"urn:gridsam:myproxy\">\n" +
            		"  <ProxyServer>" + myproxy_server + "</ProxyServer>\n" +
            		"  <ProxyServerPort>7512</ProxyServerPort>\n" +
            		"  <ProxyServerUserName>" + user + "</ProxyServerUserName>\n" +
            		"  <ProxyServerPassPhrase>" + passwd + "</ProxyServerPassPhrase>\n" +
            		" </MyProxy>\n" + 
		 	"</jsdl:JobDefinition>\n";
		try
		{
			BufferedReader in = new BufferedReader(new FileReader(jsdl_filename));
			while((line = in.readLine())!=null){
				if(line.indexOf("</jsdl:JobDefinition>")!= -1){
					sb.append(myproxy_buffer);
				}
				else {
					sb.append(line);
                  			sb.append(System.getProperty("line.separator"));
				}
			}
			in.close();
		} catch (IOException e){
                        String info = e.getMessage().replace('\n', ' ');
                        synchronized (System.out)
                        {
                                System.out.println("SUBMIT" + " " + jid_str + " FAILURE " + info);
                        }
			return null;
		} 
		return sb.toString();
	}

        String add_username(int jid, String jsdl_filename) {
                String username_buffer;
                StringBuffer sb = new StringBuffer();
                String line;
                String jid_str = String.valueOf(jid);

                username_buffer = "   <ac:Credential xmlns:ac=\"http://schemas.ogf.org/hpcp/2007/11/ac\">\n" +
                                  "    <wsse:UsernameToken xmlns:wsse=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd\">\n" +
                                  "     <wsse:Username>***</wsse:Username>\n" +
                                  "     <wsse:Password>***</wsse:Password>\n" +
                                  "    </wsse:UsernameToken>\n" +
                                  "   </ac:Credential>\n";
                try
                {
                        BufferedReader in = new BufferedReader(new FileReader(jsdl_filename));
                        while((line = in.readLine())!=null){
                                if((line.indexOf("</jsdl:Target>")!= -1) || (line.indexOf("</jsdl:Source>")!= -1)){
                                        sb.append(line);
                                        sb.append(username_buffer);
                                }
                                else {
                                        sb.append(line);
                                        sb.append(System.getProperty("line.separator"));
                                }
                        }
                        in.close();
                } catch (IOException e){
                        String info = e.getMessage().replace('\n', ' ');
                        synchronized (System.out)
                        {
                                System.out.println("SUBMIT" + " " + jid_str + " FAILURE " + info);
                        }
                        return null;
                }
                return sb.toString();
        }
}
