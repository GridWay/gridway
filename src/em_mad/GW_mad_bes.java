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
import java.net.URL;

import org.glite.ce.creambes.client.CreateActivityClient;
import org.glite.ce.creambes.client.GetActivityStatusClient;
import org.glite.ce.creambes.client.TerminateActivitiesClient;

class GW_mad_bes extends Thread {//implements GramJobListener {

    //private Map job_pool = null; // Job pool
    //private Map jid_pool = null; // JID pool
//    private Long last_goodtill = 0L;
    private String action;
    private String[] arguments;

    //private String portNumber;
    
    public static void main(String args[]) {
    	 int i = 0;
         //String _portNumber = "8443";
         //boolean error=false;
         GW_mad_bes gw_mad_bes;
	 String[] arg = null;
         
	 if (args.length>2)
	 {
	     if (args[0].equals("--op") && (args[1].equals("SUBMIT") || args[1].equals("POLL") || args[1].equals("CANCEL")))
		{
			arg = new String[args.length-2];
	                System.arraycopy(args,2,arg,0,arg.length);

			gw_mad_bes = new GW_mad_bes(args[1],arg);
			gw_mad_bes.loop();
		}
		else
		{
			printUsage();
	     	}
	 }
	 else
	 {
         	printUsage();
	 }
    }

    // Constructor
    GW_mad_bes(String action, String[] arguments) {
    	
	this.action = action;
	this.arguments = arguments;
    	//this.portNumber = portNumber;
        // Create the job and JID pool
        // Warning! With JDK1.5 should be "new HashMap<Integer,GramJob>()"
        //job_pool = Collections.synchronizedMap(new HashMap());
        //jid_pool = Collections.synchronizedMap(new HashMap());
    }

    void loop() {


   	// Perform the action
        /*if (action.equals("INIT"))
        	init();
        else if (action.equals("FINALIZE"))
        {
        	finalize_mad();
                fin = true;
        }
        if (action.equals("SUBMIT")) 
        {
        	// Create a job
               	try
               	{
        		CreateActivityClient jobCA = null; 
                        Service s = new Service(action, jobCA, arguments);
                        s.start();
                }
               	catch (Exception e)
                {
                	String info = e.getMessage().replace('\n', ' ');

                        synchronized (System.out)
                        {
                            System.out.println(action + " " +// jid_str + 
				    " FAILURE "
                                    + info);
                        }
                    }
        }
        else if (action.equals("CANCEL"))
        {
		try
		{
			GetActivityStatusClient jobGAS = null;
                        Service s = new Service(action, jobGAS, arguments);
                        s.start();
		}
		catch (Exception e)
                {
                        String info = e.getMessage().replace('\n', ' ');

                        synchronized (System.out)
                        {
                            System.out.println(action + " " +// jid_str + 
                                    " FAILURE "
                                    + info);
                        }
                }
        }
	else if (action.equals("POLL"))
	{
		try
		{
			TerminateActivitiesClient jobTA = null;
			Service s = new Service(action, jobTA, arguments);
                        s.start();
		}
		catch (Exception e)
                {
                        String info = e.getMessage().replace('\n', ' ');

                        synchronized (System.out)
                        {
                           	System.out.println(action + " " +// jid_str + 
                                    	" FAILURE "
                                    	+ info);
                        }
                }
	}
        else
        {
                synchronized(System.out)
                {
                        System.out.println(action + " " +// jid_str
                                " FAILURE Not valid action");
                }
        }
    }

    /*void init() {
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
    }*/


    private static void printUsage() {

        System.err.println("Usage:");
        System.err
        	.println("GW_mad_bes --op SUBMIT | POLL | CANCEL <arguments>");
        System.err
                .println("This method: 1) submits the job described by <jsdl-filename> to a BES service, 2) returns the status of one or more activities or 3) is used to terminate one or more activities");
        System.err
              	.println("Each operation requires a set of specific parameters (<arguments>):");
	System.err
		.println("	-r|--resource <endpoint> [-u|--user <user>] [-p|--passwd <pass>] [--voms <service URI>] [==help] <jsdl-filename> ... for SUBMIT operation");
	System.err
		.println("	-r|--resource <endpoint> [-h|--help] [-u|--user <user>] [-p|--passwd <pass>] [--voms <service URI>] <activity> ... for POLL and CANCEL operations");
    }
}

class Service extends Thread {
    String action, contact;
    CreateActivityClient jobCA;
    GetActivityStatusClient jobGAS;
    TerminateActivitiesClient jobTA;
    int status = 0;
    String info;
    //String portNumber;
    String arguments[];

    Service(String action, CreateActivityClient job, String[] arguments) {
        this.action = action;
        //this.jid = jid;
        this.jobCA = job;
        //this.contact = contact;
        //this.portNumber = portNumber;
	this.arguments = arguments;
    }

    Service(String action, GetActivityStatusClient job, String[] arguments) {
        this.action = action;
        //this.jid = jid;
        this.jobGAS = job;
        //this.contact = contact;
        //this.portNumber = portNumber;
	this.arguments = arguments;
    }

    Service(String action, TerminateActivitiesClient job, String[] arguments) {
        this.action = action;
        //this.jid = jid;
        this.jobTA = job;
        //this.contact = contact;
        //this.portNumber = portNumber;
	this.arguments = arguments;
    }

    public void run() {
        // Perform the action
        if (action.equals("SUBMIT"))
            status = submit(jobCA);
        /*else if (action.equals("RECOVER"))
            status = recover(jid, job, contact);*/
        else if (action.equals("CANCEL"))
            status = cancel(jobTA);
        else if (action.equals("POLL"))
            status = poll(jobGAS);
        else
        {
            status = 1;
            info = "Not valid action";
        }

        synchronized (System.out)
        {
            if (status == 0)
                System.out.println(action + " " +// jid 
			" SUCCESS " + info);
            else
                System.out.println(action + " " +// jid + 
			" FAILURE " + info);
        }
    }

    int submit (CreateActivityClient job) {
        int status = 0;
        int index = 0;

        info = "-";

        // Submit the job
        try
        {
            job.main(arguments);
        }
        catch (Exception e)
        {
            info = e.getMessage().replace('\n', ' ');
            status = 1;
        }

        return status;
    }
    
    int cancel (TerminateActivitiesClient job) {
        int status = 0;

        info = "-";

        // Cancel the job
        try
        {
            job.main(arguments);
        }
        catch (Exception e)
        {
            info = e.getMessage().replace('\n', ' ');
            status = 1;
        }

        return status;
    }

    int poll(GetActivityStatusClient job) {
        int status = 0;
        int index = 0;

        // Get job status
        try
        {
	    job.main(arguments);
        }
        catch (Exception e)
        {
            info = e.getMessage().replace('\n', ' ');
            status = 1;
        }

        return status;
    }
}
