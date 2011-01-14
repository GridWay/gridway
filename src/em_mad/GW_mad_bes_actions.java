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

import javax.xml.soap.SOAPException;
import java.net.MalformedURLException;
import javax.xml.rpc.ServiceException;
import java.rmi.RemoteException;

import org.ggf.bes.*;
import org.apache.axis.message.MessageElement;
import org.ggf.jsdl.JobDefinition_Type;
import org.w3.www._2005._08.addressing.EndpointReferenceType;
import javax.xml.namespace.QName;
import org.w3.www._2005._08.addressing.ReferenceParametersType;
import org.apache.axis.encoding.DeserializationContext;
import org.xml.sax.InputSource;
import org.apache.axis.MessageContext;
import org.apache.axis.encoding.Deserializer;
import org.apache.axis.message.EnvelopeHandler;
import org.apache.axis.message.SOAPHandler;

class Service extends Thread {
	String action;
	Integer jid;
	BESJob job;
	int status = 0;
	String info;
	ActivityStateEnumeration state;

        private MessageElement actId;
        private Calendar terminationTime = null;

	Service(String action, Integer jid, BESJob job) {
		this.action = action;
		this.jid = jid;
		this.job = job;
	}

	public void run() {
		// Perform the action
		if (action.equals("SUBMIT"))
			submit();
		//else if (action.equals("RECOVER"))
			//status = recover(jid, job, contact);
		else if (action.equals("CANCEL"))
			cancel();
		else if (action.equals("POLL"))
			poll();
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

	void submit() {

		Calendar calendar = Calendar.getInstance();
		calendar.set(Calendar.WEEK_OF_YEAR , calendar.get(Calendar.WEEK_OF_YEAR) + 1);

		try
		{ 
			setTerminationTime(calendar.getTime()); // One week
		}
		catch (Exception e)
		{
		}

		// Submit the job
		try
		{
			jobSubmit();

		}
		catch (Exception e)
		{
			info = e.getMessage().replace('\n', ' ');
			status = 1;
		}

	}

	void cancel() {

		// Cancel the job
		try
		{
			poll();
			if (info.equals("HARD_KILL")){
				status = 1;
				info = "Previously cancelled";
			}
			//If the job has not been cancelled (or even if poll() fails), it tries to cancel the job
			else{
				jobCancel();
				if (status == 0){
					poll();
					//If poll() fails, it assumes that the job has not been actually cancelled
					if (status == 1)
						info = "NOT CANCELLED" + info;
				}
			}
		}
		catch (Exception e)
		{
			info = e.getMessage().replace('\n', ' ');
			status = 1;
		}
	}

	void poll() {
		try
		{
			refreshStatus();
			if (status == 0)
				info = state.getValue().toUpperCase();
			if (info.equals("FINISHED"))
                       		info = "DONE";
			else if (info.equals("CANCELLED"))
				info = "HARD_KILL";
			else if (info.equals("RUNNING"))
				info = "ACTIVE";
		}
		catch (Exception e)
		{
			info = e.getMessage().replace('\n', ' ');
			status = 1;
		}
	}


	protected void jobSubmit() throws SOAPException, MalformedURLException,
		  ServiceException, RemoteException {

		BESServiceLocator besLocator = new BESServiceLocator();
                BESFactoryPortType stub = besLocator.getBes(new URL(job.getContact()));
		//insertWSSHeader(stub, user, passwd);

		try {
			CreateActivityType request = new CreateActivityType();
                	ActivityDocumentType activity = new ActivityDocumentType();

                	CreateActivityResponseType result = null;

			activity.setJobDefinition(job.parseJSDLFile());
                	request.setActivityDocument(activity);
                	result = stub.createActivity(request);

			if (result == null) {
                        	status = 1;
                	}

                	EndpointReferenceType activityId = result.getActivityIdentifier();

			if (activityId == null) {
                        	status = 1;
                	}
 			else{
        			this.actId = new MessageElement(new QName("http://www.w3.org/2005/08/addressing","EndpointReferenceType"),activityId);
				job.setActivityId(actId);
				info = job.extractJobID(activityId.getReferenceParameters());
        		}
		} catch (UnsupportedFeatureFaultType ex) {
       			info = ex.getMessage().replace('\n', ' ');
                        status = 1;
		} catch(Exception ex) {
			info = ex.getMessage().replace('\n', ' ');
			status = 1;
		}			
	}
  

    protected void refreshStatus() throws SOAPException, MalformedURLException,
			ServiceException, RemoteException {

		status = 0;
                EndpointReferenceType[] epr = new EndpointReferenceType[1];
	
		try{
                        epr[0] = job.parseEPR(job.getActivityId().getAsString());
                }catch(Exception ex){
			info = ex.getMessage().replace('\n', ' ');
			status = 1;
                }

                BESServiceLocator besLocator = new BESServiceLocator();
                BESFactoryPortType stub = besLocator.getBes(new URL(job.getContact()));

                GetActivityStatusesType request = new GetActivityStatusesType();
		request.setActivityIdentifier(epr);

		GetActivityStatusesResponseType response = stub
				.getActivityStatuses(request);
		
		GetActivityStatusResponseType[] activityStatusArray = response
				.getResponse();
		if (activityStatusArray == null) {
			info = "No response available";
			status = 1;
			return;
		}

	    	if (activityStatusArray[0].getActivityStatus() != null) {
                        ActivityStateEnumeration state = activityStatusArray[0].getActivityStatus().getState();
			this.state = state;
	    	}else{
			try{
                		MessageElement tmpElem = 
                    		new MessageElement(new QName("http://schemas.ggf.org/bes/2006/08/bes-factory",
                        	                         "GetActivityStatusResponseType"));
				info = tmpElem.getAsString();
			}catch(Exception ex){
                        	info = ex.getMessage().replace('\n', ' ');
                        	status = 1;
			}
			status = 1;
            	}

	    	if (activityStatusArray[0].getFault() != null) {
			status = 1;
			info = info + " " + activityStatusArray[0].getFault().getFaultstring();
	    	}
	
	
    }
    
    protected void jobCancel() 
        throws SOAPException, MalformedURLException, ServiceException, RemoteException {

	status = 0;
        EndpointReferenceType[] epr = new EndpointReferenceType[1];


        try{
        	epr[0] = job.parseEPR(job.getActivityId().getAsString());
        }catch(Exception ex){
		info = ex.getMessage().replace('\n', ' ');
		status = 1;
        }

        BESServiceLocator besLocator = new BESServiceLocator();
        BESFactoryPortType stub = besLocator.getBes(new URL(job.getContact()));

        TerminateActivitiesType request = new TerminateActivitiesType();
        request.setActivityIdentifier(epr);

        TerminateActivitiesResponseType response = stub.terminateActivities(request);

        /*TerminateActivityResponseType termActivityArray[] = response.getResponse();
	
	if (termActivityArray.isCancelled()) {
		info = "CANCELLED";
        } else {
		info = "NOT CANCELLED";
		status = 1;
                if (termActivityArray[0].getFault() != null) {
		    info = info + termActivityArray[0].getFault().getFaultstring();
                }
        }*/
    }

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

}

class BESJob {

        private String hostPort;
        private String jsdlFileName;
        private String jobId;

	private MessageElement actId;

        public BESJob(String jobId,
                        String hostPort,
                        String jsdlFileName) {
                this.jobId = jobId;
                this.hostPort = hostPort;
                this.jsdlFileName = jsdlFileName;
        }

	protected void setActivityId(MessageElement actId){
		this.actId = actId;
	}

	public MessageElement getActivityId(){
		return actId;
	}

	public String getContact(){
		return hostPort;
	}

        public static String extractJobID(ReferenceParametersType params){

                int i;
                String id = new String();
                id = "";

                if( params==null )
                        return null;

                //Change for other platforms
                QName jobIDQN = new QName("http://www.icenigrid.org/service/gridsam","JobIdentifier");

                MessageElement[] msgElem = params.get_any();

                for(int k=0; k< msgElem.length; k++){
                        if( msgElem[k].getQName().equals(jobIDQN) ){
                                for(i=0; i<msgElem[k].getElementsByTagName("ID").getLength(); i++)
                                        id = id + msgElem[0].getElementsByTagName("ID").item(i).getChildNodes().item(0) + " ";
                        }
                }
                return id;
    }

    public JobDefinition_Type parseJSDLFile() throws ServiceException {
	return this.parseJSDLFile(jsdlFileName);
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

class BESServiceLocator extends org.apache.axis.client.Service { 

    public BESServiceLocator() {
    }

    // The WSDD service name defaults to the port name.
    private java.lang.String BESWSDDServiceName = "bes";

    public java.lang.String getBESWSDDServiceName() {
        return BESWSDDServiceName;
    }

    public org.ggf.bes.BESFactoryPortType getBes(java.net.URL portAddress) throws javax.xml.rpc.ServiceException {
        try {
            org.ggf.bes.BESFactoryBindingStub _stub = new org.ggf.bes.BESFactoryBindingStub(portAddress, this);
            _stub.setPortName(getBESWSDDServiceName());
            return _stub;
        }
        catch (org.apache.axis.AxisFault e) {
            return null;
        }
    }
}

