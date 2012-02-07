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

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Vector;

import javax.xml.rpc.Stub;

import org.apache.axis.message.addressing.Address;
import org.apache.axis.message.addressing.EndpointReferenceType;
import org.globus.gsi.GSIConstants;
import org.globus.rft.generated.DeleteOptionsType;
import org.globus.rft.generated.DeleteRequestType;
import org.globus.rft.generated.DeleteType;
import org.globus.rft.generated.RFTOptionsType;
import org.globus.rft.generated.ReliableFileTransferPortType;
import org.globus.rft.generated.Start;
import org.globus.rft.generated.StartOutputType;
import org.globus.rft.generated.TransferRequestType;
import org.globus.rft.generated.TransferType;
import org.globus.transfer.reliable.client.BaseRFTClient;
import org.globus.transfer.reliable.service.RFTConstants;
import org.globus.wsrf.NotificationConsumerManager;
import org.globus.wsrf.WSNConstants;
import org.globus.wsrf.container.ServiceContainer;
import org.globus.wsrf.impl.security.authorization.Authorization;
import org.globus.wsrf.impl.security.authorization.HostAuthorization;
import org.globus.wsrf.impl.security.authorization.ResourcePDPConfig;
import org.globus.wsrf.impl.security.authorization.ServiceAuthorizationChain;
import org.globus.wsrf.impl.security.descriptor.GSISecureConvAuthMethod;
import org.globus.wsrf.impl.security.descriptor.GSISecureMsgAuthMethod;
import org.globus.wsrf.impl.security.descriptor.GSITransportAuthMethod;
import org.globus.wsrf.impl.security.descriptor.ResourceSecurityDescriptor;
import org.globus.wsrf.security.Constants;
import org.oasis.wsn.Subscribe;
import org.oasis.wsn.TopicExpressionType;
import org.oasis.wsrf.lifetime.SetTerminationTime;
import org.oasis.wsrf.lifetime.SetTerminationTimeResponse;

public class GWRftClient extends BaseRFTClient{

	public static String SERVICE_URL_ROOT = "/wsrf/services/";

	public static List gw_pool = Collections.synchronizedList(new ArrayList());
	
	static EndpointReferenceType credEPR = null;

    static NotificationConsumerManager consumer = null;

	public static String HOST = GWRftUtil.getLocalHostName(); 

	/**
	 * Action INIT
	 * @param xfr_id
	 */
	public static boolean gw_init(int xfr_id) throws Exception {
		if (credEPR != null) {
			return true;
		}

		if (PORT == null) {
            if (authType.equals(GSIConstants.GSI_TRANSPORT)) {
                PORT = "8443";
            } else {
                PORT ="8080";
            }
        }
        if (authType.equals(GSIConstants.GSI_TRANSPORT)) {
            PROTOCOL = "https";
        }
//        String rftFactoryAddress = 
//            PROTOCOL + "://"+ HOST+ ":" + PORT + SERVICE_URL_ROOT 
//            + "ReliableFileTransferFactoryService";
//        String rftServiceAddress = PROTOCOL+ "://" + HOST + ":" + PORT + 
//            SERVICE_URL_ROOT + "ReliableFileTransferService";
        
        //
        credEPR = delegateCredential(HOST, PORT);
		
		return false;
	}
	
	
	
	/**
	 * Action START
	 * 
	 * @param xfr_id
	 */
	
	public static boolean gw_add_xfr(int xfr_id){
		for (int i = 0; i < xfr_id + 1; i++) {
			gw_pool.add(i, new ArrayList());
		}
		
		return false;
	}

	/**
	 * Action END
	 * 
	 * @param xfr_id
	 */
	public static boolean gw_del_xfr(int xfr_id){
		List list = (List) gw_pool.get(xfr_id); 
		list.removeAll(gw_pool);
		gw_pool.set(xfr_id, null);
		
		return false;
	}
	
	/**
	 * Action RMDIR
	 * @param resource
	 * @param src_url
	 */
	public static boolean gw_transfer_rmdir(GWRftResource resource, String src_url) throws Exception {
        
		DeleteRequestType deleteRequestType = getDeleteRequest(src_url, credEPR);

        String rftFactoryAddress = 
            PROTOCOL + "://"+ HOST+ ":" + PORT + SERVICE_URL_ROOT 
            + "ReliableFileTransferFactoryService";
        String rftServiceAddress = PROTOCOL+ "://" + HOST + ":" + PORT + 
            SERVICE_URL_ROOT + "ReliableFileTransferService";

        EndpointReferenceType rftepr = createRFT(rftFactoryAddress, 
                deleteRequestType);

		
        rftepr.setAddress(new Address(rftServiceAddress));
        ReliableFileTransferPortType rft = rftLocator
                .getReliableFileTransferPortTypePort(rftepr);
        //For secure notifications
        setSecurity((Stub)rft);
        subscribe(resource, rft);
        //End subscription code
        Calendar termTime = Calendar.getInstance();
        termTime.add(Calendar.MINUTE, TERM_TIME);
        SetTerminationTime reqTermTime = new SetTerminationTime();
        reqTermTime.setRequestedTerminationTime(termTime);
        SetTerminationTimeResponse termRes = rft
                .setTerminationTime(reqTermTime);
        StartOutputType startresp = rft.start(new Start());
                
		return false;
		
	}

	/**
	 * Action MKDIR
	 * @param xfr
	 * @param src_url
	 */
	public static boolean gw_transfer_mkdir(GWRftResource xfr, String src_url) throws Exception {
		//in order to create a directory, we must first create the dir locally 
		//and then, copy this directory to the final destination
		String dirName = "";
		
		
		// TODO this temporary directory should be deleted at the end, now it is only deleted 
		//      when the program ends
		try{
			File tempDir = File.createTempFile("tempdir", null);
			dirName += tempDir.getCanonicalPath();
			tempDir.delete();
			tempDir = new File(dirName);
			if (!tempDir.exists()){
				tempDir.deleteOnExit();
				tempDir.mkdirs();
			}
			
			//dirName += tempDir.getCanonicalPath();
			dirName = "file://" + dirName;
			dirName = GWRftUtil.correctDirectoryURL(dirName);
	    	
	    	src_url = GWRftUtil.correctDirectoryURL(src_url);
	    	
		}catch (IOException ioe) {
			//ioe.printStackTrace(System.err);
			//System.err.println(ioe.getMessage());
			return true;
		}
		
		return gw_transfer_url_to_url(xfr, 0, dirName, src_url, '-');
		
	}

	

	/**
	 * Action CP
	 * Copy a file (a dir is also possible) from the source url to the destination url
	 * The protocols file://, gsiftp:// are supported <br/>
	 * When we use modex with the value 'X', and we are copying a local file (using file://)
	 * we chage the execute permission of this file 
	 * 
	 * @param xfr
	 * @param cp_xfr_id
	 * @param src_url
	 * @param dst_url
	 * @param modex
	 */
	public static boolean gw_transfer_url_to_url(
			GWRftResource xfr, int cp_xfr_id, String src_url, String dst_url, char modex) throws Exception {
        
		// We must test the value of modex, to see if we must set the execution flag
		if (modex == 'X' && src_url.substring(0,7).equals("file://")) {
			Process proc = Runtime.getRuntime().exec("chmod a+x " + src_url.substring(7));
			int exitCode = proc.waitFor();
			
			if (exitCode != 0) {
				// if we cannot chage the permissions of the directory, this is considered as a failure
				return true;
			}
			
		}
		
		src_url = GWRftUtil.correctURL(src_url);
		dst_url = GWRftUtil.correctURL(dst_url);
		
		TransferRequestType transferType = getTransfer(src_url, dst_url, credEPR);
        
        String rftFactoryAddress = 
            PROTOCOL + "://"+ HOST+ ":" + PORT + SERVICE_URL_ROOT 
            + "ReliableFileTransferFactoryService";
        String rftServiceAddress = PROTOCOL+ "://" + HOST + ":" + PORT + 
            SERVICE_URL_ROOT + "ReliableFileTransferService";

        
        EndpointReferenceType rftepr = createRFT(rftFactoryAddress, 
                transferType);

        rftepr.setAddress(new Address(rftServiceAddress));
        ReliableFileTransferPortType rft = rftLocator
                .getReliableFileTransferPortTypePort(rftepr);
        setSecurity((Stub)rft);

        //For secure notifications
        subscribe(xfr, rft);
        //End subscription code
        Calendar termTime = Calendar.getInstance();
        termTime.add(Calendar.MINUTE, TERM_TIME);
        SetTerminationTime reqTermTime = new SetTerminationTime();
        reqTermTime.setRequestedTerminationTime(termTime);
        SetTerminationTimeResponse termRes = rft
                .setTerminationTime(reqTermTime);
        StartOutputType startresp = rft.start(new Start());
        
		return false;
		
	}
	
	/**
	 * Deletes a dir (src_url), it is used by the method gw_transfer_rmdir
	 * @param src_url
	 * @param epr
	 */
    private static DeleteRequestType getDeleteRequest(String src_url, EndpointReferenceType epr) {

		src_url = GWRftUtil.correctDirectoryURL(src_url);
    	
    	
    	DeleteRequestType deleteRequest = new DeleteRequestType();
	    DeleteType deleteArray[] = new DeleteType[1];
	    
	    deleteArray[0] = new DeleteType();
	    deleteArray[0].setFile(src_url);

	    //transferCount = (requestData.size() - 1) / 2;
	    DeleteOptionsType deleteOptions = new DeleteOptionsType();

	    //Subject name (defaults to host subject)
	    //deleteOptions.setSubjectName("SUBJECT-NAME");
	    deleteOptions.setSubjectName(null);
	    deleteRequest.setDeleteOptions(deleteOptions);
	    deleteRequest.setDeletion(deleteArray);
	    deleteRequest.setTransferCredentialEndpoint(epr);
	    //deleteRequest.setConcurrency(new Integer(1));
	    //deleteRequest.setMaxAttempts(new Integer(10));
	    return deleteRequest;
    }

    /**
     * Gets the transfer attribute of the ReliableFileTransferClient class,
     * used by the method gw_transfer_url_to_url
     * @param path
     * @param epr
     * @return The transfer value
     */
    private static TransferRequestType getTransfer(
    		String src_url, String dst_url, EndpointReferenceType epr) {

        TransferType[] transfers1 = new TransferType[1];
        RFTOptionsType multirftOptions = new RFTOptionsType();
        multirftOptions.setBinary(Boolean.valueOf(true)); //true is the default value
        multirftOptions.setBlockSize(new Integer(16000)); //16000 is the default value
        multirftOptions.setTcpBufferSize(new Integer(16000)); //16000 is the default value
        multirftOptions.setNotpt(new Boolean(false)); //false is the default value
        multirftOptions.setParallelStreams(new Integer(1));
        multirftOptions.setDcau(new Boolean(true)); //Data Channel Authentication for FTP transfers
        int concurrency = 1; //number of files to transfer at any given point, 1 is the default
        
        //This is used for Authorization purposes (in the source/destination gridftp server)
//        String sourceSubjectName = (String) requestData.elementAt(i++);
//        if (sourceSubjectName != null) {
//            multirftOptions.setSourceSubjectName(sourceSubjectName);
//        }
//        String destinationSubjectName = (String) requestData.elementAt(i++);
//        if (destinationSubjectName != null) {
//            multirftOptions.setDestinationSubjectName(destinationSubjectName);
//        }
        boolean allOrNone = false;
        int maxAttempts = 10;

            transfers1[0] = new TransferType();
            transfers1[0].setSourceUrl(src_url);
            transfers1[0]
                    .setDestinationUrl(dst_url);

            //TODO We should used a suitable ID value
            //transfers1[0].setTransferId(new Integer(1));

        TransferRequestType transferRequest = new TransferRequestType();
        transferRequest.setTransfer(transfers1);
        transferRequest.setRftOptions(multirftOptions);
        transferRequest.setConcurrency(new Integer(concurrency));
        transferRequest.setAllOrNone(new Boolean(allOrNone));
        transferRequest.setMaxAttempts(new Integer(maxAttempts));
        transferRequest.setTransferCredentialEndpoint(epr);
      
        return transferRequest;
    }


    public static void subscribe(GWRftResource resource, ReliableFileTransferPortType rft)
			throws Exception {
		Subscribe request = new Subscribe();
		request.setUseNotify(Boolean.TRUE);
		if (PROTOCOL.equals("http")) {
			consumer = NotificationConsumerManager.getInstance();
		} else if (PROTOCOL.equals("https")) {
			Map properties = new HashMap();
			properties.put(ServiceContainer.CLASS,
					"org.globus.wsrf.container.GSIServiceContainer");
			consumer = NotificationConsumerManager.getInstance(properties);
		}
		consumer.startListening();
		EndpointReferenceType consumerEPR = null;
		ResourceSecurityDescriptor resDesc = new ResourceSecurityDescriptor();
		Vector authMethod = new Vector();
		if (AUTHZ.equalsIgnoreCase("host")) {
			ResourcePDPConfig pdpConfig = new ResourcePDPConfig("host");
			pdpConfig.setProperty(Authorization.HOST_PREFIX,
					HostAuthorization.URL_PROPERTY, endpoint);
			ServiceAuthorizationChain authz = new ServiceAuthorizationChain();
			authz.initialize(pdpConfig, "chainName", "someId");
			resDesc.setAuthzChain(authz);
		} else if (AUTHZ.equalsIgnoreCase("self")) {
			resDesc.setAuthz("self");
		}
		if (PROTOCOL.equals("http")) {
			if (authType.equals(Constants.GSI_SEC_MSG)) {
				authMethod.add(GSISecureMsgAuthMethod.BOTH);
			} else if (authType.equals(Constants.GSI_SEC_CONV)) {
				authMethod.add(GSISecureConvAuthMethod.BOTH);
			}
		} else if (PROTOCOL.equals("https")) {
			authMethod.add(GSITransportAuthMethod.BOTH);
		}
		resDesc.setAuthMethods(authMethod);
		consumerEPR = consumer.createNotificationConsumer(resource,
				resDesc);
		request.setConsumerReference(consumerEPR);
		TopicExpressionType topicExpression = new TopicExpressionType();
		topicExpression.setDialect(WSNConstants.SIMPLE_TOPIC_DIALECT);
		//this is the type of events we want to subscribe to
		topicExpression.setValue(RFTConstants.REQUEST_STATUS_RESOURCE);
		request.setTopicExpression(topicExpression);

		rft.subscribe(request);
	}
	
}
