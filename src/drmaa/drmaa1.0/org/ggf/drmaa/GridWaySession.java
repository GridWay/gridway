/* -------------------------------------------------------------------------- */
/* Copyright 2002-2010, GridWay Project Leads (GridWay.org)                   														*/
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
/* limitations under the License.                                             																				*/
/* -------------------------------------------------------------------------- */

package org.ggf.drmaa;

/** This is the class to implement the Session interface.
 */
public class GridWaySession implements Session 
{
	/* -------------------------------------------------------------------------- *
 	 * Public attributes used for DRMAA GridWay implementation		              *
 	 * -------------------------------------------------------------------------- */

	/** Pre-defined string to refer to the number of total tasks in a bulk job.
 	 *  GW_TOTAL_TASKS will be available during job execution 
 	 *  and can be used as an ARGUMENT for the REMOTE COMMAND.
 	 *  This attribute name should be used ONLY in conjuntion with a 
 	 *  drmaa_run_bulk_jobs() function call.
 	 */
	public static final String GW_TOTAL_TASKS		="${TOTAL_TASKS}";

	/** Pre-defined string to refer to the job unique identification as provided by
 	 *  the GridWay system. GW_JOB_ID will be available during job execution 
 	 *  and can be used as an ARGUMENT for the REMOTE COMMAND. It is also usefull 
 	 *  to generate output filenames, since it is available in the main DRMAA
 	 *  program as returned by {@link #runBulkJobs} and {@link #runJob} method calls.
 	 */
	public static final String GW_JOB_ID				="${JOB_ID}";

	/** Pre-defined string to refer to the task unique identification as provided by
 	 *  the GridWay system. GW_TASK_ID will be available during job execution 
 	 *  and can be used as an ARGUMENT for the REMOTE COMMAND. It is also usefull 
 	 *  to generate output filenames of bluk jobs. GW_TASK_ID ALWAYS ranges
 	 *  from 0 to {@link #GW_TOTAL_TASKS} -1. Please note that this attribute name
 	 *  should be used ONLY in conjuntion with a {@link #runBulkJobs}
 	 *  method call.
 	 */
	public static final String GW_TASK_ID				="${TASK_ID}";

	/** Pre-defined string to refer to the remote host architecture as returned by
 	 *  the resource selector module. DRMAA_GW_ARCH will be available during job 
 	 *  execution and can be used to generate architecture-dependent REMOTE 
 	 *  COMMAND executables.
 	 */
	public static final String GW_ARCH					="${ARCH}";
	
	
	/** Pre-defined string to define single (one process) jobs.
	 */
	public static final String GW_TYPE_SINGLE		="single";
	
	/** Pre-defined string to define MPI (Message Passing Interface) jobs.
	*/
	public static final String GW_TYPE_MPI			="mpi";
	
	/* -------------------------------------------------------------------------- *
 	 * Private attributes used for DRMAA implementation		              *
 	 * -------------------------------------------------------------------------- */

	private DrmaaJNI	drmaaImpl;
	private String		contactString = null;
	private Version		version;
	private String		drmsInfo;
	private String		drmaaImplementation;
	
	public GridWaySession()
	{
		drmaaImpl 					= new DrmaaJNI();
		version   						= new Version(1, 0);
		drmsInfo  						= new String(drmaaImpl.getDrmsInfo());
		drmaaImplementation 	= new String(drmaaImpl.getDrmaaImplementation());
	}
	
	public void init(java.lang.String contactString)
	throws DrmaaException
	{	
		if (contactString!=null)
			this.contactString = new String(contactString);
				
		drmaaImpl.init(this.contactString); 
	};

	public  void exit()
	throws DrmaaException
	{		
		drmaaImpl.exit();	
	};

	public JobTemplate createJobTemplate()
	throws DrmaaException
	{
		return drmaaImpl.createJobTemplate();
	};

	public void deleteJobTemplate(JobTemplate jt)
	throws DrmaaException
	{
		drmaaImpl.deleteJobTemplate(jt);
	};

	public java.lang.String runJob(JobTemplate jobTemplate)
	throws DrmaaException
	{
		return drmaaImpl.runJob((GridWayJobTemplate) jobTemplate);
	};

	public java.util.List runBulkJobs(JobTemplate jobTemplate, int beginIndex, int endIndex, int step)
	throws DrmaaException
	{
		return drmaaImpl.runBulkJobs((GridWayJobTemplate) jobTemplate, beginIndex, endIndex, step);
	}

	public void control(java.lang.String jobName, int operation)
	throws DrmaaException
	{
		drmaaImpl.control(jobName, operation);	
	}

	public void synchronize(java.util.List  jobList, long timeout, boolean dispose)
	throws DrmaaException
	{
	   	drmaaImpl.synchronize(jobList, timeout, dispose);
	};

	public JobInfo wait(java.lang.String  jobName, long timeout)
	throws DrmaaException
	{
		return drmaaImpl.wait(jobName, timeout);
	};

	public int getJobStatus(java.lang.String jobName)
	throws DrmaaException
	{
		return drmaaImpl.getJobStatus(jobName);
	};
	
	public java.lang.String getContact()
	{
		return this.contactString;
	};
	
	public Version getVersion()
	{
		this.version = this.drmaaImpl.getVersion();
		return this.version;
	};
	
	
	public java.lang.String getDrmsInfo()
	{
		return this.drmsInfo;
	};
	
	public java.lang.String getDrmaaImplementation()
	{
		return this.drmaaImplementation;
	};

}
