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


import java.net.InetAddress;
import java.net.UnknownHostException;

public class GWRftUtil {

	private static String localHostName;
    
	/**
	 * Gets the host name of this local machine
	 * @return this local host name
	 */
    public static String getLocalHostName() {
    	if (localHostName==null){
    		try {
 		       InetAddress addr = InetAddress.getLocalHost();
 		       localHostName = addr.getHostName();
 		     } catch (UnknownHostException e) {
 		     }
    	}
    	return localHostName;
    }
	
	/**
	 * Transform the url of type "file://whateveritis" into "gsiftp://localhostname/whateveritis"
	 * @param url
	 * @return
	 */
	public static String correctURL(String url) {
		if (url.substring(0,7).equals("file://")) {
			url = "gsiftp://" + localHostName + url.substring(7);
		}
		
		return url;
	}

	/**
	 * Transform the url of type "file://whateveritis" into "gsiftp://localhostname/whateveritis",
	 * considering that this is a directory. So if it is not ended by a "/", it is just added at the end of the string
	 * @param url
	 * @return
	 */
	public static String correctDirectoryURL(String url) {
		url = correctURL(url);
		
		if ( url.charAt(url.length() - 1) != '/' )
    		url = url.concat("/");
		return url;
	}
	
	
}
