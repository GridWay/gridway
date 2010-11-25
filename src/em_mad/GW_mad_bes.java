/* -------------------------------------------------------------------------- */
/* Copyright 2002-2010, GridWay Project Leads (GridWay.org)                   */
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

import javax.xml.soap.SOAPException;
import java.net.MalformedURLException;
import javax.xml.rpc.ServiceException;
import java.rmi.RemoteException;

import org.ggf.bes.*;
import org.apache.axis.message.MessageElement;
import org.glite.ce.creambes.utils.XMLPrettyPrinter;
import org.ggf.jsdl.JobDefinition_Type;
import org.w3.www._2005._08.addressing.EndpointReferenceType;
import javax.xml.namespace.QName;
import org.w3.www._2005._08.addressing.ReferenceParametersType;
import org.w3.www._2005._08.addressing.AttributedURIType;
import org.apache.axis.encoding.DeserializationContext;
import org.xml.sax.InputSource;
import org.apache.axis.MessageContext;
import org.apache.axis.encoding.Deserializer;
import org.apache.axis.message.EnvelopeHandler;
import org.apache.axis.message.SOAPHandler;

class GW_mad_bes extends Thread {//implements GramJobListener {

    private Map job_pool = null; // Job pool
    private Map jid_pool = null; // JID pool

    private String portNumber;
    
    public static void main(String args[]) {
    	 //int i = 0;
         //String arg;
         //String _portNumber = "8443";
         //boolean error=false;
         GW_mad_bes gw_mad_bes;
         
         /*while ((i < args.length) && args[i].startsWith("-")) 
         {
             arg = args[i++];
             if (arg.equals("-p")) 
             {
                 if (i < args.length)
                	 _portNumber = args[i++];
                 else
                 {
                     System.err.println("-p requires a portnumber");
                     error = true;
                 }
             }
         }

        if (error == false)
        {*/
        	gw_mad_bes = new GW_mad_bes();
        	
            //gw_mad_ws.start();
            gw_mad_bes.loop();
            //gw_mad_ws.interrupt();
        //}
    }

    // Constructor
    GW_mad_bes() {
    	//this.portNumber = portNumber;
        // Create the job and JID pool
        // Warning! With JDK1.5 should be "new HashMap<Integer,GramJob>()"
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
	BESJob job = null;

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
                    System.out.println(action + " " + jid_str + " FAILURE " + info);
                }
            }

            String str_split[] = str.split(" ", 4);

            if (str_split.length != 4)
            {
                synchronized (System.out)
                {
                    System.out.println(action + " " + jid_str + " FAILURE "
                            + "Error reading line");
                }
                fin = true;
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
                    fin = true;
                }
                else if (action.equals("SUBMIT"))
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
                        if (action.equals("SUBMIT"))
                        {
			    job = new BESJob(jid_str,contact,jsdl_file);
                        }
                        
			// Add state listener
                        //job.addListener(this);

                        Service s = new Service(action, jid, job, contact, portNumber);
                        s.start();

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
		    
	            job = (BESJob) job_pool.get(jid);

                    if (job == null) 
                    {
                        synchronized(System.out)
                        {
                            System.out.println(action + " " + jid + " FAILURE Job does not exist jid");
                        }
                    }
                    else
                    {
                        Service s = new Service(action, jid, job, contact, portNumber);
                        s.start();
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
    // Method from interface GramJobListener
}

class Service extends Thread {
    String action, contact;
    Integer jid;
    BESJob job;
    int status = 0;
    String info;
    String portNumber;

    Service(String action, Integer jid, BESJob job, String contact, String portNumber) {
        this.action = action;
        this.jid = jid;
        this.job = job;
        this.contact = contact;
        this.portNumber = portNumber;
    }

    public void run() {
        // Perform the action
        if (action.equals("SUBMIT"))
            status = submit(jid, job, contact);
        //else if (action.equals("RECOVER"))
            //status = recover(jid, job, contact);
        else if (action.equals("CANCEL"))
            status = cancel(jid, job);
        else if (action.equals("POLL"))
            status = poll(jid, job);
        else
        {
            status = 1;
            info = "Not valid action";
        }

        synchronized (System.out)
        {
            if (status == 0)
                System.out.println(action + " " + jid + " SUCCESS " + info);
            else
                System.out.println(action + " " + jid + " FAILURE " + info);
        }
    }

    int submit (Integer jid, BESJob job, String contact) {
        int status = 0;
        int index = 0;
        //EndpointReferenceType factoryEndpoint = null;

        info = "-";

        Calendar calendar = Calendar.getInstance();
        calendar.set(Calendar.WEEK_OF_YEAR , calendar.get(Calendar.WEEK_OF_YEAR) + 1);
        
	try
	{ 
        	job.setTerminationTime(calendar.getTime()); // One week
	}
	catch (Exception e)
	{
	}

        // Submit the job
        try
        {
            job.submit();
	    status = job.getStatus();
	    info = job.getInfo();
            //info = job.getHandle();
        }
        catch (Exception e)
        {
            info = e.getMessage().replace('\n', ' ');
            status = 1;
        }

        return status;
    }
    
    int cancel (Integer jid, BESJob job) {
        int status = 0;

        info = "-";

        // Cancel the job
        try
        {
            job.cancel();
            status = job.getStatus();
            info = job.getInfo();
        }
        catch (Exception e)
        {
            info = e.getMessage().replace('\n', ' ');
            status = 1;
        }

        return status;
    }

    int poll(Integer jid, BESJob job) {
        int status = 0;
        int index = 0;
	String state;

        // Get job status
        try
        {
            job.refreshStatus();
            status = job.getStatus();
            info = job.getInfo();

	    ActivityStateEnumeration jobState = job.getState();
	    state = jobState.getValue().toUpperCase();
	    info = info + state;
            /* (jobState == ActivityStateEnumeration.Unsubmitted)
                info = "PENDING";*/
        }
        catch (Exception e)
        {
            info = e.getMessage().replace('\n', ' ');
            status = 1;
        }

        return status;
    }
}

class BESJob {

    private String hostPort;
    private String jsdlFileName;
    private String jobId;

    private int status = 0;
    private String info;
    private ActivityStateEnumeration state;

    private Calendar terminationTime = null;
    private Vector listeners;

    public BESJob(String jobId,
	       String hostPort,
	       String jsdlFileName) {
        this.jobId = jobId;
	this.hostPort = hostPort;
	this.jsdlFileName = jsdlFileName;
    }

    public int getStatus(){
	return this.status;
    }

    public String getInfo() {
	return this.info;
    }

    public ActivityStateEnumeration getState() {
	return this.state;
    }

    protected void submit() throws SOAPException, MalformedURLException,
			ServiceException, RemoteException {

		CreamBesServiceLocator besLocator = new CreamBesServiceLocator();
		BESFactoryPortType stub = besLocator.getCreamBes(new URL(hostPort));

		//WSSBuilder.insertWSSHeader(stub, user, passwd, vomsEpr);

		//for (int i = 0; jsdlFileName != null && i < jsdlFileName.length; i++) {
			try {
				//submitSingleJob( stub, jsdlFileName );
				CreateActivityType request = new CreateActivityType();
                		ActivityDocumentType activity = new ActivityDocumentType();

                		CreateActivityResponseType result = null;

                		activity.setJobDefinition(parseJSDLFile(jsdlFileName));
                		request.setActivityDocument(activity);
                		result = stub.createActivity(request);
                		if (result == null) {
                        		status = 1;
                		}

                		EndpointReferenceType activityId = result.getActivityIdentifier();
                		if (activityId == null) {
                        		status = 1;
                		}
	
			} catch (UnsupportedFeatureFaultType ex) {
				/*System.err.println("UnsupportedFeatureFault raised:");
				System.err.println("Detail Message: " + ex.getMessage());
				int featureSize = (ex.getFeature() == null ? 0
						: ex.getFeature().length);
				int messageSize = (ex.get_any() == null ? 0 : ex.get_any().length);
				// Prints all the feature and message elements
				for (int j = 0; j < featureSize || j < messageSize; ++j) {
					if (ex.getFeature() != null && j < ex.getFeature().length) {
						System.err.println("Feature: " + ex.getFeature(j));
					}
					if (ex.get_any() != null && j < ex.get_any().length) {
						MessageElement msg = ex.get_any()[j];
						System.err.println("MessageElement: " + XMLPrettyPrinter.prettyPrint(msg));
					}*/
                                info = ex.getMessage().replace('\n', ' ');
                                status = 1;
			} catch(Exception ex) {
				//System.err.println("Failed due to exception: "+ex);				
				info = ex.getMessage().replace('\n', ' ');
				status = 1;
			}			
		//}
	}
    
    /*protected void submitSingleJob(BESFactoryPortType stub, String file)
			throws RemoteException, NotAcceptingNewActivitiesFaultType,
			UnsupportedFeatureFaultType, InvalidRequestMessageFaultType,
			NotAuthorizedFaultType, ServiceException {
		CreateActivityType request = new CreateActivityType();
		ActivityDocumentType activity = new ActivityDocumentType();

		CreateActivityResponseType result = null;

		//activity.setJobDefinition(BESClient.parseJSDLFile(file));
                activity.setJobDefinition(parseJSDLFile(file));
		request.setActivityDocument(activity);
		result = stub.createActivity(request);

		if (result == null) {
			System.err.println("CreateActivityResponse not found");
			return;
		}

		EndpointReferenceType activityId = result.getActivityIdentifier();
		if (activityId == null) {
			System.err.println("ActivityIdentifier not found");
			return;
		}else{
            MessageElement tmpElem = new MessageElement(new QName("http://www.w3.org/2005/08/addressing", 
                                                                  "EndpointReferenceType"),
                                                        activityId);
            System.out.println("\n---------------------------- raw epr begin ----------------------------");
            System.out.println(tmpElem);
            System.out.println("----------------------------- raw epr end -----------------------------\n");
        }
		if (activityId.getAddress() != null) {
			System.out.println("Activity Identifier address: "
					+ activityId.getAddress());
		}
		if (activityId.getReferenceParameters() != null) {
			System.out.println("Activity Identifier Reference Parameters: "
					//+ BESClient.extractJobID(activityId
                                        + extractJobID(activityId
							.getReferenceParameters()));
		}
		if (activityId.getMetadata() != null) {
			System.out.println("Activity Identifier Metadata: "
					+ activityId.getMetadata().toString());
		}
	}*/    
    
    
    protected void refreshStatus() throws SOAPException, MalformedURLException,
			ServiceException, RemoteException {

		//EndpointReferenceType[] epr = new EndpointReferenceType[jobId.length];
                EndpointReferenceType epr;

		//for (int i = 0; i < jobId.length; i++) {
            if( jobId.trim().startsWith("https") ){
                AttributedURIType address = new AttributedURIType(hostPort);

                MessageElement[] elem = new MessageElement[1];
                elem[0] = new MessageElement(new QName("http://glite.org/2007/02/ce/cream/types", "id"),
                                             jobId);
                ReferenceParametersType refParam = new ReferenceParametersType(elem);

                epr = new EndpointReferenceType();
                epr.setAddress(address);
                epr.setReferenceParameters(refParam);
            }else{
                /*
                  jobId is the raw xml of the EPR
                */
                //epr = BESClient.parseEPR(jobId);
	        epr = parseEPR(jobId);
            }
		//}

		CreamBesServiceLocator besLocator = new CreamBesServiceLocator();
		BESFactoryPortType stub = besLocator.getCreamBes(new URL(hostPort));

		//WSSBuilder.insertWSSHeader(stub, user, passwd, vomsEpr);

		GetActivityStatusesType request = new GetActivityStatusesType();
		request.setActivityIdentifier(0,epr);

		GetActivityStatusesResponseType response = stub
				.getActivityStatuses(request);

		GetActivityStatusResponseType activityStatusArray = response
				.getResponse(0);

		if (activityStatusArray == null) {
			//System.out.println("No response available");
			status = 1;
			return;
		}

		//System.out.println("Retrieved " + activityStatusArray.length
				//+ " activity");

		//for (int i = 0; i < activityStatusArray.length; i++) {

			/*System.out.println("activity epr = "
					+ activityStatusArray.getActivityIdentifier()
							.getAddress());*/
            /*ReferenceParametersType param = activityStatusArray
					.getActivityIdentifier().getReferenceParameters();
            String tmpJobID = extractJobID(param);
            if( tmpJobID!=null && tmpJobID.length()>0 ){
                System.out.println("          jobId = " + tmpJobID);
            }*/

	    if (activityStatusArray.getActivityStatus() != null) {
			/*System.out
					.println("          activity status = "
							+ activityStatusArray.getActivityStatus()
									.getState());*/
			ActivityStateEnumeration state = activityStatusArray.getActivityStatus().getState();
	    }else{
                /*MessageElement tmpElem = 
                    new MessageElement(new QName("http://schemas.ggf.org/bes/2006/08/bes-factory",
                                                 "GetActivityStatusResponseType"));
                System.out.println("\n---------------------------- raw status begin ----------------------------");
                System.out.println(tmpElem);
                System.out.println("----------------------------- raw status end -----------------------------\n");*/
		status = 1;
            }

	    if (activityStatusArray.getFault() != null) {
			/*System.out.println("          activity fault = "
					+ activityStatusArray.getFault().getFaultstring());*/
		status = 1;
		info = activityStatusArray.getFault().getFaultstring();
	    }
		//}
	}
    
    
    protected void cancel() 
        throws SOAPException, MalformedURLException, ServiceException, RemoteException {

        EndpointReferenceType epr;

        //for (int i = 0; i < jobId.length; i++) {
            if( jobId.trim().startsWith("https") ){
                AttributedURIType address = new AttributedURIType(hostPort);

                MessageElement[] elem = new MessageElement[1];
                elem[0] = new MessageElement(new QName("http://glite.org/2007/02/ce/cream/types", "id"),jobId);
                ReferenceParametersType refParam = new ReferenceParametersType(elem);

                epr = new EndpointReferenceType();
                epr.setAddress(address);
                epr.setReferenceParameters(refParam);
            }else{
                /*
                  jobId is the raw xml of the EPR
                */
                epr = parseEPR(jobId);
            }
        //}

        CreamBesServiceLocator besLocator = new CreamBesServiceLocator();
        BESFactoryPortType stub = besLocator.getCreamBes(new URL(hostPort));

        //WSSBuilder.insertWSSHeader(stub, user, passwd, vomsEpr);

        TerminateActivitiesType request = new TerminateActivitiesType();
        request.setActivityIdentifier(0,epr);

        TerminateActivitiesResponseType response = stub.terminateActivities(request);

        TerminateActivityResponseType termActivityArray = response.getResponse(0);

        //System.out.println("result: retrieved " + termActivityArray.length + " activity");

        //for (int i = 0; i < termActivityArray.length; i++) {
            //System.out.println("activity epr = " + termActivityArray.getActivityIdentifier().getAddress().toString());
            //if (termActivityArray[i].isTerminated()) {
	    if (termActivityArray.isCancelled()) {
                //System.out.println("activity status = cancelled");
		info = "CANCELLED";
            } else {
                //System.out.println("activity status = not cancelled");
		info = "NOT CANCELLED";
		status = 1;
                if (termActivityArray.getFault() != null) {
                    //System.out.println("activity fault = " + termActivityArray.getFault().getFaultstring());
		    info = info + termActivityArray.getFault().getFaultstring();
                }
            }
        //}
    }

    /**
     * Set the termination time of a job.
     * @param date The date/time desired for termination of this job service.
     *             If the value is null then the termination time will be set
     *             to now.
     */
    public synchronized void setTerminationTime(
        Date date) {

        if (this.terminationTime == null) {
            this.terminationTime = Calendar.getInstance();
        }

        if (date != null) {
            this.terminationTime.setTime(date);
        } else {
            this.terminationTime.setTime(Calendar.getInstance().getTime());
        }
    }


    public static JobDefinition_Type parseJSDLFile(String fileName) throws ServiceException {

        FileInputStream in = null;
        try{
            in = new FileInputStream(fileName);

            DeserializationContext deserCtx = new DeserializationContext(new InputSource(in),
                                                                         MessageContext.getCurrentContext(),"");
            Deserializer deserializer = deserCtx.getDeserializerForClass(JobDefinition_Type.class);
            deserCtx.pushElementHandler(new EnvelopeHandler((SOAPHandler)deserializer));

            deserCtx.parse();

            return (JobDefinition_Type)deserializer.getValue();
        }catch(Exception ex){
            throw new ServiceException(ex.getMessage());
        }finally{
            if( in!=null ){
                try{
                    in.close();
                }catch(Exception ex){}
            }
        }
    }

    public static EndpointReferenceType parseEPR(String epr) throws ServiceException {
        try{
            DeserializationContext deserCtx = new DeserializationContext(new InputSource(new StringReader(epr)),
                                                                         MessageContext.getCurrentContext(),"");
            Deserializer deserializer = deserCtx.getDeserializerForClass(EndpointReferenceType.class);
            deserCtx.pushElementHandler(new EnvelopeHandler((SOAPHandler)deserializer));

            deserCtx.parse();

            return (EndpointReferenceType)deserializer.getValue();
        }catch(Exception ex){
            throw new ServiceException(ex.getMessage());
        }
    }
    
}
