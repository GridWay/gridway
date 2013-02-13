/* -------------------------------------------------------------------------- */
/* Copyright 2002-2013, GridWay Project Leads (GridWay.org)                   */
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

package org.ggf.drmaa;

import java.util.*;

/** This class implements the abstract methods of the {@link org.ggf.drmaa.JobInfo} class.
 *  The sets methods are not commented because they are just used internally.
 *  <br>
 *  The normal user of this API <b><i>SHOULD NEVER</i></b> use these methods.
 */

public class GridWayJobInfo implements JobInfo
{
	/** This attribute stores the unique name offered by GridWay
	*/
	private String 	jobId							= null;
	
	/** This attribute stores the status of the Job
	*/
	private int	status;
	/** This attribute stores the Job resource usage
	*/
	private java.util.Map	resourceUsage	= null;
	
	private boolean hasExited 				= false;
	
	private boolean hasSignaled 				= false;
	
	private boolean hasCoreDump 			= false;
	
	private boolean wasAborted 				= false;
	
	private String	signal    					= null;

	
	GridWayJobInfo(String jobName, int statusCode, java.util.Map resourceUsage)
	{
		this.jobId 					= new String(jobName);
		this.status     			= statusCode;
		this.resourceUsage 	= new Hashtable(resourceUsage);
	}
	
	/** This method returns the completed job's id.*/
	public String getJobId()
	{
	   return this.jobId;
	}
	
	/** This method returns the completed job's resource usage data.*/
	public java.util.Map getResourceUsage()
	{
	  return this.resourceUsage;
	}
	
	public boolean hasExited()
	{
		return hasExited;
	}
	
	public int getExitStatus()
	{
		if (this.hasExited())
		   return this.status;
		else
		   return 0;
	}
	
	public boolean hasSignaled()
	{
		return hasSignaled;
	}
	
	public String getTerminatingSignal()
	{
		if (this.hasSignaled())
		    return this.signal;
		else
		   return null;
	}
	
	public boolean hasCoreDump()
	{
		return this.hasCoreDump;
	}
	
	public boolean wasAborted()
	{
		return this.wasAborted;
	}

	/**The normal user of this API <b><i>SHOULD NEVER</i></b> use this method.
	  */
	public void setHasExited(boolean hasExited)
	{
		this.hasExited = hasExited;
	}
	
	/**The normal user of this API <b><i>SHOULD NEVER</i></b> use this method.
	  */
	public void setHasSignaled(boolean hasSignaled)
	{
		this.hasSignaled = hasSignaled;
	}
	
	/**The normal user of this API <b><i>SHOULD NEVER</i></b> use this method.
	  */
	public void setTerminationSignal(String signal)
	{
		this.signal = new String(signal);
	}
	
	/**The normal user of this API <b><i>SHOULD NEVER</i></b> use this method.
	  */
	public void setHasCoreDump(boolean hasCoreDump)
	{
		this.hasCoreDump = hasCoreDump;
	}
	
	/**The normal user of this API <b><i>SHOULD NEVER</i></b> use this method.
	  */
	public void setWasAborted(boolean wasAborted)
	{
		this.wasAborted = wasAborted;
	}	
}
