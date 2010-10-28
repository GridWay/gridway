/* -------------------------------------------------------------------------- */
/* Copyright 2002-2010, GridWay Project Leads (GridWay.org)                  													 */
/*                                                                            																							*/
/* Licensed under the Apache License, Version 2.0 (the "License"); you may    														*/
/* not use this file except in compliance with the License. You may obtain    															*/
/* a copy of the License at                                                   																				*/
/*                                                                            																							*/
/* http://www.apache.org/licenses/LICENSE-2.0                                 																	*/
/*                                                                            																							*/
/* Unless required by applicable law or agreed to in writing, software        																*/
/* distributed under the License is distributed on an "AS IS" BASIS,          																*/
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   												*/
/* See the License for the specific language governing permissions and        															*/
/* limitations under the License.                                            												 							*/
/* -------------------------------------------------------------------------  */

package org.ggf.drmaa;
/** The information regarding a job's execution is encapsulated in the JobInfo interface. Via the JobInfo
 * interface a DRMAA application can discover information about the resource usage and exit status of 
 * a job. 
 */

public abstract interface JobInfo 
{
	/** This method gets the completed job's id. 
 	 * 
	 *  @return A {@link String} with the jobId value
 	 *
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws InternalException
 	 */
	public abstract String getJobId()
		throws DrmaaException;

	/** This method gets the completed job's resource usage data. 
 	 * 
	 *  @return A {@link java.util.Map} object with the jobId value
 	 *
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws InternalException
 	 */
	public abstract java.util.Map getResourceUsage()
		throws DrmaaException;

	/** This method returns into exited a non-zero value if stat was returned
 	 *  for a job that terminated normally. The job exit status can be retrieved
 	 *  using {@link #getExitStatus}. 
 	 *
 	 *  @return  The return is true if the job terminated abnormally,
	 *  drmaa_wifsignaled() can be used to gather more
 	 *  information. NOTE: The status code is interpreted in a bash fashion
 	 *
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws InternalException
 	 */
	public abstract boolean hasExited()
		throws DrmaaException;
	
	/** This method returns the exit code extracted from stat.
 	 *
 	 * 
 	 *  @return  The exit code extracted from stat
 	 *
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws InternalException
 	 */
	public abstract int getExitStatus()
		throws DrmaaException;
	
	/** This method evaluates into signaled a non-zero value if stat was returned
 	 *  for a job that terminated due to the receipt of a signal.
 	 *  NOTE: The status code is interpreted in a bash fashion
 	 * 
 	 *  @return  The return is true  if the job terminated on a signal
  	 *
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws InternalException
 	 */
	public abstract boolean hasSignaled()
		throws DrmaaException;
	
	/** This methods returns the signal name that
 	 *  causes the termination of the job. Only signals by POSIX are returned. 
 	 *  For non-POSIX signals, the returned name is "UNKNOWN".
 	 *
 	 *  @return  The signal name stored in a String that causes the termination of the job
 	 *
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws InternalException
 	 */
	public abstract String getTerminatingSignal()
		throws DrmaaException;
	
	/** This methods always returns "0"
 	 *
 	 * 
 	 *  @return  Always returns "0"
 	 *
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws InternalException
	 */
	public abstract boolean hasCoreDump()
		throws DrmaaException;
	
	/** This methods always returns "0"
 	 *
 	 * 
 	 *  @return  Always returns "0"
 	 *
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws InternalException
	 */
	public abstract boolean wasAborted()
		throws DrmaaException;
}
