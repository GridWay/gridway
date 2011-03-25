/* -------------------------------------------------------------------------- */
/* Copyright 2002-2010 GridWay Team, Distributed Systems Architecture         */
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

class GW_em_mad_bes extends Thread {

	private Map job_pool = null; // Job pool
	private Map jid_pool = null; // JID pool

	private String host;

	public static void main(String args[]) {
		GW_em_mad_bes gw_em_mad_bes;

		gw_em_mad_bes = new GW_em_mad_bes();
		gw_em_mad_bes.loop();
	}

	// Constructor
	GW_em_mad_bes() {
		// Create the job and JID pool
		job_pool = Collections.synchronizedMap(new HashMap());
		jid_pool = Collections.synchronizedMap(new HashMap());
	}

	void loop() {
		String str = null;
		String action = null;
		String jid_str = null;
		String contact;
		String jsdl_file = null;
		boolean fin = false;
		Integer jid = null;
		ServiceBES job = null;

		BufferedReader in = new BufferedReader(new InputStreamReader(System.in));
		while (!fin) {
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
                 		
				// Perform the action
				if (action.equals("INIT"))
					init();
				else if (action.equals("FINALIZE"))
				{
					finalize_mad();
					fin = true;
				}
				else if (action.equals("SUBMIT") || action.equals("RECOVER"))
				{
					try
					{
						jid = new Integer(jid_str);
					}
					catch (NumberFormatException e)
					{
						String info = e.getMessage().replace('\n', ' ');

						synchronized (System.out)
						{
							System.out.println(action + " " + jid_str + " FAILURE "
									+ info);
						}
					}

					// Create a job
					try
					{	
						if (action.equals("SUBMIT")){
							host = str_split[2];
							if (str_split[2].indexOf('/') != -1)
                                        			host = str_split[2].substring(0,str_split[2].indexOf('/'));
                                                        contact = "http://" + host + ":8080/gridsam/services/bes";
                                			jsdl_file = str_split[3];
							job = new ServiceBES(action, jid, contact, jsdl_file);
						}
						else
						{
							contact = str_split[2];
							job = new ServiceBES(action, jid, contact);
						}
						job.start();
	
						// Add job and jid to the pools
						job_pool.put(jid, job);
						jid_pool.put(job, jid);
					}
					catch (Exception e)
					{
						String info = e.getMessage().replace('\n', ' ');

						synchronized (System.out)
						{
							System.out.println(action + " " + jid_str + " FAILURE "
									+ info);
						}
					}
				}
				else if (action.equals("CANCEL") || action.equals("POLL"))
				{
					try
					{
						jid = new Integer(jid_str);
					}
					catch (NumberFormatException e)
					{
						String info = e.getMessage().replace('\n', ' ');

						synchronized(System.out)
						{
							System.out.println(action + " " + jid_str + " FAILURE "
									+ info);
						}
					}

					job = (ServiceBES) job_pool.get(jid);

					if (job == null) 
					{
						synchronized(System.out)
						{
							System.out.println(action + " " + jid + " FAILURE Job does not exist jid");
						}
					}
					else
					{
						job = new ServiceBES(action, jid, job);
						job.start();
					}
				}
				else
				{
					synchronized(System.out)
					{
						System.out.println(action + " " + jid_str
								+ " FAILURE Not valid action");
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

}
