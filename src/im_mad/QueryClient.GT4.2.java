/* -------------------------------------------------------------------------- */
/* Copyright 2002-2006 GridWay Team, Distributed Systems Architecture         */
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

/**
 * QueryClient queries an Index Service (MDS4) and calls a parser for the
 * resulting XML doc
 */

import java.util.List;

import org.oasis.wsrf.properties.WSResourcePropertiesServiceAddressingLocator;
import org.oasis.wsrf.properties.QueryResourceProperties_Element;
import org.oasis.wsrf.properties.QueryResourcePropertiesResponse;
import org.oasis.wsrf.properties.QueryExpressionType;
import org.oasis.wsrf.properties.QueryResourceProperties_PortType;

//import org.apache.commons.cli.ParseException;
//import org.apache.commons.cli.CommandLine;
import org.globus.axis.message.addressing.Address;
import org.globus.axis.message.addressing.EndpointReferenceType;
import org.apache.axis.types.URI.MalformedURIException;

import org.globus.wsrf.WSRFConstants;
import org.globus.wsrf.utils.AnyHelper;
import org.globus.wsrf.utils.FaultHelper;
import org.globus.wsrf.client.Query;
import org.globus.wsrf.impl.security.authentication.Constants;
import org.globus.wsrf.impl.security.authorization.NoAuthorization;
import org.globus.wsrf.impl.security.authentication.Constants;
import org.globus.wsrf.impl.security.authentication.encryption.EncryptionCredentials;

import org.globus.axis.gsi.GSIConstants;
import org.globus.axis.util.Util;

import org.globus.gsi.CertUtil;

import java.security.cert.X509Certificate;
import javax.security.auth.Subject;
import javax.xml.rpc.Stub;

public class QueryClient
{
	String serverName;
	String portNumber;
    protected EndpointReferenceType endpoint;

    static 
	{
        Util.registerTransport();
    }
	
	QueryClient(String serverName,String portNumber)
	{
		this.serverName = serverName;
		this.portNumber = portNumber;
	}
	
	// Dicovery	
	String retrieveHosts()
	{
		
		/***************************************************************************************
		 * First: Contact the Index Service and retrieve the XML doc
		 ****************************************************************************************/	
		
		String dialect = WSRFConstants.XPATH_1_DIALECT;
	    String expression = "//*[local-name()=\"MemberServiceEPR\"]/*[local-name()=\"Address\" "+
							"and substring-after(text(),\"/wsrf/services/\")=\"DefaultIndexService\"]";		
		String contactString;
		String result = "FAILED";
		
		contactString = "https://" + serverName + ":" + portNumber + "/wsrf/services/DefaultIndexService";
		
		this.endpoint = new EndpointReferenceType();
		
		try
		{
        	this.endpoint.setAddress(new Address(contactString));		
		}
		catch(MalformedURIException mfue)
		{
			result = "FAILED Malformed URI Exception. Error message:" + mfue.getMessage().replace('\n', ' ');
			return result;			
		}
	
		WSResourcePropertiesServiceAddressingLocator locator =
			new WSResourcePropertiesServiceAddressingLocator();

		try 
		{
			QueryExpressionType query = new QueryExpressionType();
			query.setDialect(dialect);
			query.setValue(expression);

			QueryResourceProperties_PortType port = locator.getQueryResourcePropertiesPort(this.endpoint);
			setOptions((Stub)port);

			QueryResourceProperties_Element request	= new QueryResourceProperties_Element();
			request.setQueryExpression(query);

			QueryResourcePropertiesResponse response = port.queryResourceProperties(request);

			if (response != null && response.get_any() != null &&
				response.get_any().length != 0)
			{
				result = AnyHelper.toSingleString(response);
			}           

		} 
		catch(Exception e) 
		{
			result = "FAILED " + e.getMessage().replace('\n', ' ');
			return result;
		}
		
		/***************************************************************************************
		 * Second: Parse the XML doc and retrieve the relevant information
		 ****************************************************************************************/
		
		Mds4QueryParser mqp = new Mds4QueryParser();
		return mqp.parseXMLString("<root>"+result+"</root>", "");
	}	
	
	// Monitoring
	String retrieveAttributes()
	{
		
		/***************************************************************************************
		 * First: Contact the Index Service and retrieve the XML doc
		 ****************************************************************************************/
		
		
		String dialect = WSRFConstants.XPATH_1_DIALECT;
	    String expression = "/*/*/*[local-name()=\"MemberServiceEPR\"]/*[local-name()=\"Address\" and" + 		
					  " substring-after(text(),\"/wsrf/services/\")=\"ManagedJobFactoryService\"]/../..";	
		String contactString ;
		String result = "FAILED";
		
		contactString = "https://" + serverName + ":" + portNumber + "/wsrf/services/DefaultIndexService";
		
		this.endpoint = new EndpointReferenceType();
		
		try
		{
        	this.endpoint.setAddress(new Address(contactString));		
		}
		catch(MalformedURIException mfue)
		{
			result = "FAILED Malformed URI Exception. Error message:" + mfue.getMessage().replace('\n', ' ');
			return result;			
		}				
		
		WSResourcePropertiesServiceAddressingLocator locator =
			new WSResourcePropertiesServiceAddressingLocator();

		try 
		{
			QueryExpressionType query = new QueryExpressionType();
			query.setDialect(dialect);
			query.setValue(expression);

			QueryResourceProperties_PortType port = locator.getQueryResourcePropertiesPort(this.endpoint);
			setOptions((Stub)port);

			QueryResourceProperties_Element request	= new QueryResourceProperties_Element();
			request.setQueryExpression(query);

			QueryResourcePropertiesResponse response = port.queryResourceProperties(request);

			if (response != null && response.get_any() != null &&
				response.get_any().length != 0)
			{
				result = AnyHelper.toSingleString(response);
			}            

		} 
		catch(Exception e) 
		{
			result = "FAILED " + e.getMessage().replace('\n', ' ');
			return result;
		}
		
		/***************************************************************************************
		 * Second: Parse the XML doc and retrieve the relevant information
		 ****************************************************************************************/
		
		Mds4QueryParser mqp = new Mds4QueryParser();
		return mqp.parseXMLString("<root>"+result+"</root>", serverName);
	}	
	
	// Set connection parameters
	public void setOptions(Stub stub) throws Exception 
	{
        if (this.endpoint.getAddress().getScheme().equals("https")) 
		{
                stub._setProperty(GSIConstants.GSI_TRANSPORT,
                                  Constants.SIGNATURE);
        }
        stub._setProperty(Constants.AUTHORIZATION, NoAuthorization.getInstance());
        stub._setProperty(Constants.GSI_ANONYMOUS, Boolean.TRUE);
    }
	
}
