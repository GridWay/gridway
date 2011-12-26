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
	private org.icenigrid.schema.bes.factory.y2006.m08.ActivityStateEnumeration.Enum state;

	int status = 0;
	String info;

        private EndpointReferenceDocument xActivityIdentifier;

        public ServiceBES(Integer jid, String contact, String JSDL) {
		this.jid = jid;
                this.contact = contact;
                this.jsdlFile = JSDL;
        }

	public String getInformation(){
		return info;
	}


	protected int submit() throws SOAPException, MalformedURLException,
		  ServiceException, RemoteException {

		try { 
                        JobDefinitionDocument xJSDL = JobDefinitionDocument.Factory.parse(this.jsdlFile);
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
				//this.info = "";
				return -1;
                	}
			else {
				//Get GridSAM JobID
				String id = xActivityIdentifier.getEndpointReference().getDomNode().getOwnerDocument().
						getElementsByTagName("ID").item(0).getChildNodes().item(0).getNodeValue();
				this.info = contact + "/" + id;
				this.xActivityIdentifier = xActivityIdentifier; 
        		}
		} catch(Exception ex) {
			this.info = ex.getMessage().replace('\n', ' ');
			return -1;
		}		
		return 0;	
	}
  
   	protected int recover() throws SOAPException, MalformedURLException,
                        ServiceException, RemoteException {

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
			//this.info = "";
			return -1;
		}
		this.xActivityIdentifier = xActivityEPR;
        	this.contact = serviceURL;

		//refreshStatus
        	GetActivityStatusesDocument xActivities = GetActivityStatusesDocument.Factory.newInstance();
        	try {
                	EndpointReferenceDocument xActivityIdentifier = EndpointReferenceDocument.Factory.parse( (new DOMReader()).read( 
				xActivityEPR.getEndpointReference().getDomNode().getOwnerDocument() ).asXML() );
                	xActivities.addNewGetActivityStatuses().addNewActivityIdentifier().set( xActivityIdentifier.getEndpointReference() );
        	} catch ( Throwable exc ){
                	this.info = exc.getMessage().replace('\n', ' ');
			return -1;
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
                	this.info = "No response available";
			return -1;
        	}
		if (xResult.getGetActivityStatusesResponse().getResponseArray(0).getActivityStatus() != null){
               		this.state = xResult.getGetActivityStatusesResponse().getResponseArray(0).getActivityStatus().getState();
			this.info = state.toString().toUpperCase();
			if (info.equals("FINISHED"))
                		this.info = "DONE";
               		else if (info.equals("CANCELLED"))
                       		this.info = "DONE";
             		else if (info.equals("RUNNING"))
                      		this.info = "ACTIVE";
		}
		else{
			this.info = "Bad contact";
			return -1;
		}
		return 0;
    	}

    	protected int poll() throws SOAPException, MalformedURLException, ServiceException, RemoteException { 

        	String xActivityEPR = (new DOMReader()).read( this.xActivityIdentifier.getEndpointReference().getDomNode().getOwnerDocument() ).asXML();
		GetActivityStatusesDocument xActivities = GetActivityStatusesDocument.Factory.newInstance();
        	try {
			EndpointReferenceDocument xActivityIdentifier = EndpointReferenceDocument.Factory.parse( xActivityEPR );
                	xActivities.addNewGetActivityStatuses().addNewActivityIdentifier().set( xActivityIdentifier.getEndpointReference() );
        	} catch ( Throwable exc ){
			this.info = exc.getMessage().replace('\n', ' ');
			return -1;
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
			this.info = "No response available";
			return -1;
		}
        	if (xResult.getGetActivityStatusesResponse().getResponseArray(0).getActivityStatus() != null){
                	this.state = xResult.getGetActivityStatusesResponse().getResponseArray(0).getActivityStatus().getState();
			this.info = state.toString().toUpperCase();
                	if (info.equals("FINISHED"))
                 		this.info = "DONE";
               		else if (info.equals("CANCELLED"))
                  		this.info = "DONE";
              		else if (info.equals("RUNNING"))
                   		this.info = "ACTIVE";
		}
        	else{
			//this.info = "";
			return -1;
        	}
		return 0;
    	}
    
    	protected int cancel() 
        	throws SOAPException, MalformedURLException, ServiceException, RemoteException {

        	String xActivityEPR = (new DOMReader()).read( this.xActivityIdentifier.getEndpointReference().getDomNode().getOwnerDocument() ).asXML();
        	TerminateActivitiesDocument xActivities = TerminateActivitiesDocument.Factory.newInstance();
		try {
        		EndpointReferenceDocument xActivityIdentifier = EndpointReferenceDocument.Factory.parse( xActivityEPR );
                	xActivities.addNewTerminateActivities().addNewActivityIdentifier().set( xActivityIdentifier.getEndpointReference() );
	      	} catch ( Throwable exc ){
        	        this.info = exc.getMessage().replace('\n', ' ');
			return -1;
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
			this.info = "Not cancelled";
			return -1;
		}
        	if (xResult.getTerminateActivitiesResponse().getResponseArray(0).getTerminated())
			this.info = "CANCELLED";
		return 0;
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

        	xCall.setTargetEndpointAddress(this.contact);

        	SOAPElement xRequest = MessageFactory.newInstance().createMessage().getSOAPBody().addDocument((Document)pOutput.newDomNode());

        	Object xRet = xCall.invoke(new Object[] { xRequest });

        	SOAPElement xResponse = null;
        	if (xRet instanceof List) {
           		xResponse = (SOAPElement) ((List) xRet).get(0);
        	}
 
        	return xResponse;
    }
}
