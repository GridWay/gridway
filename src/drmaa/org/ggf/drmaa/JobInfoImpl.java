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

/** This class implements the abstract methods of the {@link JobInfo} class.
 *  The sets methods are not commented because they are just used internally.
 *  <br>
 *  The normal user of this API <b><i>SHOULD NEVER</i></b> use these methods.
 */

public class JobInfoImpl extends JobInfo
{
	private boolean ifExited = false;
	private boolean ifSignaled = false;
	private boolean coreDump = false;
	private boolean ifAborted = false;
	private String	signal    = null;

	public JobInfoImpl(String jobName, int statusCode, java.util.Map resourceUsage)
	{
		super(jobName, statusCode, resourceUsage);
	}

	public boolean ifExited()
	{
		return ifExited;
	}
	
	public int getExitStatus()
	{
		if (this.ifExited())
		   return this.status;
		else
		   return 0;
	}
	
	public boolean ifSignaled()
	{
		return ifSignaled;
	}
	
	public String getTerminatingSignal()
	{
		if (this.ifSignaled())
		    return this.signal;
		else
		   return null;
	}
	
	public boolean coreDump()
	{
		return coreDump;
	}
	
	public boolean ifAborted()
	{
		return ifAborted;
	}

	/**The normal user of this API <b><i>SHOULD NEVER</i></b> use this method.
	  */
	public void setIfExited(boolean ifExited)
	{
		this.ifExited = ifExited;
	}
	
	/**The normal user of this API <b><i>SHOULD NEVER</i></b> use this method.
	  */
	public void setIfSignaled(boolean ifSignaled)
	{
		this.ifSignaled = ifSignaled;
	}
	
	/**The normal user of this API <b><i>SHOULD NEVER</i></b> use this method.
	  */
	public void setTerminationSignal(String signal)
	{
		this.signal = new String(signal);
	}
	
	/**The normal user of this API <b><i>SHOULD NEVER</i></b> use this method.
	  */
	public void setCoreDump(boolean coreDump)
	{
		this.coreDump = coreDump;
	}
	
	/**The normal user of this API <b><i>SHOULD NEVER</i></b> use this method.
	  */
	public void setIfAborted(boolean ifAborted)
	{
		this.ifAborted = ifAborted;
	}
}
