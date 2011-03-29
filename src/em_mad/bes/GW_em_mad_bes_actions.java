/* -------------------------------------------------------------------------- */
/* Copyright 2002-2011 GridWay Team, Distributed Systems Architecture         */
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
import java.net.MalformedURLException;
import java.rmi.RemoteException;
import javax.xml.namespace.QName;
import javax.xml.soap.MessageFactory;
import javax.xml.soap.SOAPElement;
import javax.xml.soap.SOAPException;
import javax.xml.rpc.Call;
import javax.xml.rpc.Service;
import javax.xml.rpc.ServiceException;
import javax.xml.rpc.ServiceFactory;
import org.apache.xmlbeans.XmlObject;
import org.dom4j.io.DOMReader;
import org.icenigrid.gridsam.core.support.GridSAMSupport;
import org.icenigrid.schema.bes.factory.y2006.m08.CreateActivityDocument;
import org.icenigrid.schema.bes.factory.y2006.m08.CreateActivityResponseDocument;
import org.icenigrid.schema.bes.factory.y2006.m08.GetActivityStatusesDocument;
import org.icenigrid.schema.bes.factory.y2006.m08.GetActivityStatusesResponseDocument;
import org.icenigrid.schema.bes.factory.y2006.m08.TerminateActivitiesDocument;
import org.icenigrid.schema.bes.factory.y2006.m08.TerminateActivitiesResponseDocument;
import org.icenigrid.schema.jsdl.y2005.m11.JobDefinitionDocument;
import org.icenigrid.schema.jsdl.y2005.m11.JobDefinitionType;
import org.icenigrid.schema.service.gridsam.JobIdentifierDocument;
import org.w3c.dom.Document;
import org.w3.x2005.x08.addressing.EndpointReferenceDocument;
import org.w3.x2005.x08.addressing.EndpointReferenceType;


class ServiceBES extends Thread {
	private String action;
	private Integer jid;
	private String contact;
	private String jsdlFile;
	private ServiceBES job;
	private org.icenigrid.schema.bes.factory.y2006.m08.ActivityStateEnumeration.Enum state;

	int status = 0;
	String info;

        private Calendar terminationTime = null;
        private EndpointReferenceDocument xActivityIdentifier;

        public ServiceBES(String action, Integer jid, String contact, String JSDL) {
                this.action = action;
		this.jid = jid;
                this.contact = contact;
                this.jsdlFile = JSDL;
        }

        public ServiceBES(String action, Integer jid, ServiceBES job) {
                this.action = action;
                this.jid = jid;
                this.job = job;
		this.contact = job.getContact();
        }

        public ServiceBES(String action, Integer jid, String GridSAMID) {
                this.action = action;
                this.jid = jid;
		this.contact = GridSAMID;
        }

        protected void setEPR(EndpointReferenceDocument EPR){
                xActivityIdentifier = EPR;
        }

        public String getContact(){
                return contact;
        }

        public String getJSDL(){
                return jsdlFile;
        }

        public EndpointReferenceDocument getEPR(){
                return xActivityIdentifier;
        }


	public void run() {
		// Perform the action
		if (action.equals("SUBMIT"))
			submit();
		else if (action.equals("RECOVER"))
			recover();
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

        void recover() {

                // Cancel the job
                try
                {
                        jobRecover();
                        if (status == 0){
                                info = state.toString().toUpperCase();
                        	if (info.equals("FINISHED"))
                                	info = "DONE";
                        	else if (info.equals("CANCELLED"))
                                	info = "DONE";
                       	 	else if (info.equals("RUNNING"))
                                	info = "ACTIVE";
			}
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
			jobCancel();                              
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
                                info = state.toString().toUpperCase();
			if (info.equals("FINISHED"))
                       		info = "DONE";
			else if (info.equals("CANCELLED"))
				info = "DONE";
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

		status = 0;

		try { 
                        JobDefinitionDocument xJSDL = JobDefinitionDocument.Factory.parse(this.getJSDL());
                	CreateActivityDocument xCreateActivityDocument = CreateActivityDocument.Factory.newInstance();
                	xCreateActivityDocument.addNewCreateActivity().addNewActivityDocument().setJobDefinition(xJSDL.getJobDefinition());

			CreateActivityResponseDocument xResult;
        		try {
            			SOAPElement xResponse = invokeRemoteOperation( "BESFactoryPort/CreateActivity", xCreateActivityDocument);
            			xResult = CreateActivityResponseDocument.Factory.parse(xResponse);
        		} catch ( Throwable exc ) {
            			if ( exc instanceof RemoteException ) {
                			throw ((RemoteException)exc);
            			}
            			throw new ServiceException( exc );
        		}

                	EndpointReferenceDocument xActivityIdentifier = EndpointReferenceDocument.Factory.newInstance();
                	xActivityIdentifier.setEndpointReference( xResult.getCreateActivityResponse().getActivityIdentifier() );

			if (xResult == null) {
                	       	status = 1;
                	}
			else {
				//Get GridSAM JobID
				String id = xActivityIdentifier.getEndpointReference().getDomNode().getOwnerDocument().
						getElementsByTagName("ID").item(0).getChildNodes().item(0).getNodeValue();
				info = contact + "/" + id;
	                        this.setEPR(xActivityIdentifier);
        		}
		} catch(Exception ex) {
			info = ex.getMessage().replace('\n', ' ');
			status = 1;
		}			
	}
  
    protected void jobRecover() throws SOAPException, MalformedURLException,
                        ServiceException, RemoteException {

	status = 0;
	String serviceURL;
	String jobID;

	serviceURL = contact.substring(0,contact.indexOf("/urn:gridsam:"));
	jobID = contact.substring(contact.indexOf("urn:gridsam:"),contact.length());
	this.contact = serviceURL;
        EndpointReferenceDocument xActivityEPR = EndpointReferenceDocument.Factory.newInstance();

        if (jobID != null && jobID.startsWith("urn:gridsam:")) {
		JobIdentifierDocument xJobIdentifier = JobIdentifierDocument.Factory.newInstance();
                xJobIdentifier.addNewJobIdentifier().setID(jobID);
                EndpointReferenceType xActivityIdentifier = EndpointReferenceType.Factory.newInstance();
                xActivityIdentifier.addNewAddress().setStringValue(serviceURL);
                xActivityIdentifier.addNewReferenceParameters().set(xJobIdentifier);
		xActivityEPR.addNewEndpointReference();
		xActivityEPR.setEndpointReference(xActivityIdentifier);
	}
	else {
		status = 1;
		return;
	}
        setEPR(xActivityEPR);
        this.contact = serviceURL;

	//refreshStatus
        GetActivityStatusesDocument xActivities = GetActivityStatusesDocument.Factory.newInstance();
        try {
                EndpointReferenceDocument xActivityIdentifier = EndpointReferenceDocument.Factory.parse( (new DOMReader()).read( 
				xActivityEPR.getEndpointReference().getDomNode().getOwnerDocument() ).asXML() );
                xActivities.addNewGetActivityStatuses().addNewActivityIdentifier().set( xActivityIdentifier.getEndpointReference() );
        } catch ( Throwable exc ){
                info = exc.getMessage().replace('\n', ' ');
                status = 1;
        }

        GetActivityStatusesResponseDocument xResult;
        try {
                SOAPElement xResponse = invokeRemoteOperation( "BESFactoryPort/GetActivityStatuses", xActivities);
                xResult = GetActivityStatusesResponseDocument.Factory.parse(xResponse);
        } catch ( Throwable exc ) {
                if ( exc instanceof RemoteException ) {
                        throw ((RemoteException)exc);
                }
                throw new ServiceException( exc );
        }
        if (xResult == null){
                info = "No response available";
                status = 1;
                return;
        }
	if (xResult.getGetActivityStatusesResponse().getResponseArray(0).getActivityStatus() != null)
               	this.state = xResult.getGetActivityStatusesResponse().getResponseArray(0).getActivityStatus().getState();
	else{
		info = "Bad contact";
		status = 1;
	}
    }

    protected void refreshStatus() throws SOAPException, MalformedURLException, ServiceException, RemoteException { 
	status = 0;

        String xActivityEPR = (new DOMReader()).read( this.job.getEPR().getEndpointReference().getDomNode().getOwnerDocument() ).asXML();
	GetActivityStatusesDocument xActivities = GetActivityStatusesDocument.Factory.newInstance();
        try {
		EndpointReferenceDocument xActivityIdentifier = EndpointReferenceDocument.Factory.parse( xActivityEPR );
                xActivities.addNewGetActivityStatuses().addNewActivityIdentifier().set( xActivityIdentifier.getEndpointReference() );
        } catch ( Throwable exc ){
		info = exc.getMessage().replace('\n', ' ');
                status = 1;        
        }

	GetActivityStatusesResponseDocument xResult;
       	try {
         	SOAPElement xResponse = invokeRemoteOperation( "BESFactoryPort/GetActivityStatuses", xActivities);
        	xResult = GetActivityStatusesResponseDocument.Factory.parse(xResponse);
       	} catch ( Throwable exc ) {
       		if ( exc instanceof RemoteException ) {
               		throw ((RemoteException)exc);
       		}
       		throw new ServiceException( exc );
       	}

	if (xResult == null){
		info = "No response available";
		status = 1;
		return;
	}
        if (xResult.getGetActivityStatusesResponse().getResponseArray(0).getActivityStatus() != null)
                this.state = xResult.getGetActivityStatusesResponse().getResponseArray(0).getActivityStatus().getState();
        else{
                status = 1;
        }
    }
    
    protected void jobCancel() 
        throws SOAPException, MalformedURLException, ServiceException, RemoteException {

	status = 0;

        String xActivityEPR = (new DOMReader()).read( this.job.getEPR().getEndpointReference().getDomNode().getOwnerDocument() ).asXML();
        TerminateActivitiesDocument xActivities = TerminateActivitiesDocument.Factory.newInstance();
	try {
        	EndpointReferenceDocument xActivityIdentifier = EndpointReferenceDocument.Factory.parse( xActivityEPR );
                xActivities.addNewTerminateActivities().addNewActivityIdentifier().set( xActivityIdentifier.getEndpointReference() );
      	} catch ( Throwable exc ){
                info = exc.getMessage().replace('\n', ' ');
                status = 1;
	}

	TerminateActivitiesResponseDocument xResult;
	try {
        	SOAPElement xResponse = invokeRemoteOperation( "BESFactoryPort/TerminateActivities", xActivities);
            	xResult = TerminateActivitiesResponseDocument.Factory.parse(xResponse);
        } catch ( Throwable exc ) {
        	if ( exc instanceof RemoteException ) {
                	throw ((RemoteException)exc);
        	}
        	throw new ServiceException( exc );
        }

        if (xResult == null){
		info = "Not cancelled";
                status = 1;
	}
        if (xResult.getTerminateActivitiesResponse().getResponseArray(0).getTerminated())
		info = "Cancelled";
    }

    public synchronized void setTerminationTime(Date date) {

        if (this.terminationTime == null) {
            this.terminationTime = Calendar.getInstance();
        }

        if (date != null) {
            this.terminationTime.setTime(date);
        } else {
            this.terminationTime.setTime(Calendar.getInstance().getTime());
        }
    }


    private SOAPElement invokeRemoteOperation( String pPortOperation, XmlObject pOutput ) throws Throwable {

        Service xService = null;
        Call xCall = null;
            
	xService = ServiceFactory.newInstance().createService(getClass().getClassLoader().getResource(
                "org/icenigrid/gridsam/resource/schema/wsdl/bes.wsdl"),
        new QName(GridSAMSupport.BES_NAMESPACE_STRING, "BasicExecutionService"));

        xCall = xService.createCall( new QName( GridSAMSupport.BES_FACTORY_NAMESPACE_STRING, pPortOperation.split("/")[0] ), pPortOperation.split("/")[1] );
        xCall.setProperty( Call.SOAPACTION_USE_PROPERTY, Boolean.TRUE );
        xCall.setProperty( Call.SOAPACTION_URI_PROPERTY, GridSAMSupport.BES_FACTORY_NAMESPACE_STRING + "/" + pPortOperation );

        xCall.setTargetEndpointAddress(this.getContact());

        SOAPElement xRequest = MessageFactory.newInstance().createMessage().getSOAPBody().addDocument((Document)pOutput.newDomNode());

        Object xRet = xCall.invoke(new Object[] { xRequest });

        SOAPElement xResponse = null;
        if (xRet instanceof List) {
            xResponse = (SOAPElement) ((List) xRet).get(0);
        }
 
        return xResponse;
    }
}
