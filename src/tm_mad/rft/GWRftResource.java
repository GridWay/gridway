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

import java.util.List;

import org.apache.axis.message.addressing.EndpointReferenceType;
import org.globus.rft.generated.DeleteType;
import org.globus.rft.generated.RFTFaultResourcePropertyType;
import org.globus.rft.generated.ReliableFileTransferPortType;
import org.globus.rft.generated.RequestStatusType;
import org.globus.rft.generated.RequestStatusTypeEnumeration;
import org.globus.rft.generated.TransferType;
import org.globus.transfer.reliable.service.RFTConstants;
import org.globus.wsrf.NotifyCallback;
import org.globus.wsrf.core.notification.ResourcePropertyValueChangeNotificationElementType;
import org.oasis.wsrf.faults.BaseFaultType;
import org.oasis.wsrf.properties.ResourcePropertyValueChangeNotificationType;

/**
 * This class represents a transfer/deletion ordered by GWRftProcessor. <br/>
 * It is notified when a change of state is produce in each transfer.
 * 
 */
public class GWRftResource implements NotifyCallback {

	private ReliableFileTransferPortType rft;
	
	/**
	 * This is the command producing this transfer/deletion. >br/>
	 * For instance if we order "MKDIR 3 - - gsiftp://domain.com/tmp/yourdir -", its value is "MKDIR 3 -"  
	 */
	private String action;
	
	/**
	 * This signal says if the user has been notified of the end of this transfer/deletion 
	 * (for instance, it has been printed by standard output the message "MKDIR 3 - SUCCESS -")
	 */
	private boolean done = false;

	private TransferType transfer;
	
	private DeleteType deletion;

	public GWRftResource (){
		super();
	}

	public ReliableFileTransferPortType getRft() {
		return rft;
	}

	public void setRft(ReliableFileTransferPortType rft) {
		this.rft = rft;
	}

	public String getAction() {
		return action;
	}

	public void setAction(String action) {
		this.action = action;
	}

	public boolean isDone() {
		return done;
	}

	public void setDone(boolean done) {
		this.done = done;
	}

	public TransferType getTransfer() {
		return transfer;
	}

	public void setTransfer(TransferType transfer) {
		this.transfer = transfer;
	}

	public DeleteType getDeletion() {
		return deletion;
	}

	public void setDeletion(DeleteType deletion) {
		this.deletion = deletion;
	}

    
    /**
     * Receives the notification of a change of state (for instance, from Pending->Active or Active->Done)
     * 
     * @param topicPath
     * @param producer
     * @param message
     */
    public void deliver(List topicPath, EndpointReferenceType producer,
            Object message) {

        ResourcePropertyValueChangeNotificationType changeMessage =
            ((ResourcePropertyValueChangeNotificationElementType) message)
                .getResourcePropertyValueChangeNotification();
        BaseFaultType fault = null;
        try {

            if (changeMessage != null) {

            	RequestStatusType requestStatus = (RequestStatusType) changeMessage
                        .getNewValue().get_any()[0].getValueAsType(
                        RFTConstants.REQUEST_STATUS_RESOURCE,
                		RequestStatusType.class);

            	RFTFaultResourcePropertyType faultRP = requestStatus.getFault();
                if(faultRP != null) {
                    fault = getFaultFromRP(faultRP);
                }
                if (fault != null) {
                	synchronized(this){
            			this.setDone(true);
                    	System.out.println(this.getAction() + "FAILURE " + fault.getDescription(0));
                	}
                } else {
	            	synchronized(this) {
	            		if (!this.isDone() && requestStatus.getRequestStatus().equals(RequestStatusTypeEnumeration.Done)){
	            			this.setDone(true);
	            			System.out.println(this.getAction() + "SUCCESS -");
	            		}
	            	}
                }

            }
        } catch (Exception e) {
        	//TODO This is not currently shown. Perhaps it can be useful to write it in a log file
        	//            System.err.println(e.getMessage());
        }
    }

	private BaseFaultType getFaultFromRP(RFTFaultResourcePropertyType faultRP) {
		if (faultRP.getRftAuthenticationFaultType() != null) {
			return faultRP.getRftAuthenticationFaultType();
		} else if (faultRP.getRftAuthorizationFaultType() != null) {
			return faultRP.getRftAuthorizationFaultType();
		} else if (faultRP.getRftDatabaseFaultType() != null) {
			return faultRP.getRftDatabaseFaultType();
		} else if (faultRP.getRftRepeatedlyStartedFaultType() != null) {
			return faultRP.getRftRepeatedlyStartedFaultType();
		} else if (faultRP.getRftTransferFaultType() != null) {
			return faultRP.getRftTransferFaultType();
		} else if (faultRP.getTransferTransientFaultType() != null) {
			return faultRP.getTransferTransientFaultType();
		} else {
			return null;
		}
	}

    
}
