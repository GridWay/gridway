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
/* -------------------------------------------------------------------------  */

package org.ggf.drmaa;
/** The information regarding a job's execution is encapsulated in the JobInfo class. Via the JobInfo
 * class a DRMAA application can discover information about the resource usage and exit status of 
 * a job. 
 */

public abstract class JobInfo implements java.io.Serializable
{

	/** This attribute stores the unique name offered by GridWay
	*/
	protected String 		jobId;
	
	/** This attribute stores the status of the Job
	*/
	protected int			status;
	/** This attribute stores the Job resource usage
	*/
	protected java.util.Map		resourceUsage;
	
	protected JobInfo(String jobName, int statusCode, java.util.Map resourceUsage)
	{
		this.jobId 		= new String(jobName);
		this.status     	= statusCode;
		this.resourceUsage 	= resourceUsage;
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
	
	/** This method returns into exited a non-zero value if stat was returned
 	 *  for a job that terminated normally. The job exit status can be retrieved
 	 *  using {@link #getExitStatus}. 
 	 *
 	 *  @return  The return is true if the job terminated abnormally,
	 *  drmaa_wifsignaled() can be used to gather more
 	 *  information. NOTE: The status code is interpreted in a bash fashion
 	 *
 	 */
	public abstract boolean ifExited();
	
	/** This method returns the exit code extracted from stat.
 	 *
 	 * 
 	 *  @return  The exit code extracted from stat
 	 */
	public abstract int getExitStatus();
	
	/** This method evaluates into signaled a non-zero value if stat was returned
 	 *  for a job that terminated due to the receipt of a signal.
 	 *  NOTE: The status code is interpreted in a bash fashion
 	 * 
 	 *  @return signaled non-zero if the job terminated on a signal
 	 */
	public abstract boolean ifSignaled();
	
	/** This methods returns the signal name that
 	 *  causes the termination of the job. Only signals by POSIX are returned. 
 	 *  For non-POSIX signals, the returned name is "UNKNOWN".
 	 *
 	 *  @return  The signal name stored in a String that causes the termination of the job
 	 */
	public abstract String getTerminatingSignal();
	
	/** This methods always returns "0"
 	 *
 	 * 
 	 *  @return  Always returns "0"
	 */
	public abstract boolean coreDump();
	
	/** This methods always returns "0"
 	 *
 	 * 
 	 *  @return  Always returns "0"
	 */
	public abstract boolean ifAborted();
}
