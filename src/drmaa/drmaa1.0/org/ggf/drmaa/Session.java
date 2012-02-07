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

package org.ggf.drmaa;

import java.util.List;


/** This is the interface to the majority of the functionality 
 *  offered by the GridWay DRMAA JAVA library.
 */

public abstract interface Session 
{
	/* -------------------------------------------------------------------------- *
	 * SECTION 1.1 public attributes for control operations                 	      *
	 * -------------------------------------------------------------------------- */

	
	/** Pre-defined timeout to be used with {@link #wait} and {@link #synchronize}
 	 *  method calls. TIMEOUT_WAIT_FOREVER, can be used to specify an 
 	 *  undetermined amount of time. 
 	 */
	public static final long TIMEOUT_WAIT_FOREVER 		= -1L;
	
	/** Pre-defined timeout to be used with {@link #wait} and {@link #synchronize}
 	 *  methods. TIMEOUT_NO_WAIT, can be used to specify no timeout 
 	 *  at all. The current GridWay-DRMAA implementation does NOT SUPPORT
 	 *  TIMEOUT_NO_WAIT. 
 	 */
	public static final long TIMEOUT_NO_WAIT 				= 0L;
	
	/* --------------------------------------------------- */
	
	/** UNDETERMINED Job state. An UNDETERMINED state can either be obtained due to a 
 	 *  communication error with the GridWay daemon, or because the job has not been
 	 *  initialized yet.
 	 */
	public static final int UNDETERMINED 						= 0x00;
	
	/** QUEUED-ACTIVE Job state. The job has been successfully submitted and it is
 	 *  pending to be scheduled. This state corresponds to the PENDING state in
 	 *  the GridWay system.
 	 */
	public static final int QUEUED_ACTIVE  						= 0x10;
	
	/** SYSTEM-ON-HOLD Job state. The GridWay system does NOT DEFINE a 
 	 *  SYSTEM-ON-HOLD state (currently), and so it will not be never returned
 	 *  by a {@link #getJobProgramStatus} method.
 	 */
	public static final int SYSTEM_ON_HOLD     				= 0x11;
	
	/** USER-ON-HOLD Job state. The job has been held by the user. This state 
 	 *  corresponds to the HOLD state in the GridWay system.
 	 */
	public static final int USER_ON_HOLD      					= 0x12;
	
	/** USER-SYSTEM-ON-HOLD Job state. The GridWay system does NOT DEFINE a 
 	 *  USER-SYSTEM-ON-HOLD state, and so it will not be never returned by a 
 	 *  {@link #getJobProgramStatus} method.
 	 */
	public static final int USER_SYSTEM_ON_HOLD 			= 0x13;
	
	/** RUNNING Job state. The job has been successfully scheduled and dispatched to 
 	 *  a remote host.Please note that once submitted, the job can be in any of 
 	 *  the execution states, namely: PROLOG (file stage-in), WRAPPER (execution), 
 	 *  EPILOG (file stage-out) or MIGRATING (to another host).
 	 */
	public static final int RUNNING									= 0x20;
	
	/** SYSTEM-SUSPENDED Job state. The GridWay system does NOT DEFINE a 
 	 *  SYSTEM-SUSPENDED state, and so it will not be never returned
 	 *  by a {@link #getJobProgramStatus} method.
 	*/
	public static final int SYSTEM_SUSPENDED     			 	= 0x21;
	
	/** USER-SUSPENDED Job state. The job has been successfully stopped.
 	 *  This state corresponds to the STOPPED state in the GridWay system.
 	 *  Once stopped, restart files (if provided by the job) have been tranferred 
 	 *  to the client.
 	 */
	public static final int USER_SUSPENDED      			  	= 0x22;
	
	/** USER-SYSTEM-SUSPENDED Job state. The GridWay system does NOT DEFINE a 
 	 *  USER-SYSTEM-SUSPENDED state, and so it will not be 
 	 *  never returned by a {@link #getJobProgramStatus} method.
 	 */
	public static final int USER_SYSTEM_SUSPENDED 			= 0x23;
	
	/** DONE Job state. Job has been completely executed and output files are 
 	 *  available at the client. This state corresponds to the ZOMBIE state in the 
 	 *  GridWay system. {@link #wait} and {@link #synchronize} calls on the job will
 	 *  return immediately. Also rusage information is available. 
 	 */
	public static final int DONE  					                	= 0x30;
	
	/** FAILED Job state. Job execution has failed, and the "on_failure" policy is
 	 *  to hold it on FAILED state. This state corresponds to the FAILED state in 
 	 *  the GridWay system.
 	 */
	public static final int FAILED      					          	= 0x40;
	
	
	/* --------------------------------------------------- */

		
	/** SUSPEND signal. A job will be stopped, and restart files transferred back
	 *  to the client. These files if provided by the running job will be used on
 	 *  RESUME to re-start execution.
 	*/
	public static final int SUSPEND  							 		= 0;
	
	/** RESUME signal. A previously stopped job will be resumed.
 	 *  If re-start files are provided the job will used them to re-start execution
 	 *  from the last checkpointing context.
 	 */
	public static final int RESUME    								= 1;
	
	/** HOLD signal. A job can be held if it is in the QUEUED_ACTIVE state, and on 
 	 *  SUCESS will enter the USER_ON_HOLD state.
 	 */
	public static final int HOLD 							     		= 2;
	
	/** RELEASE signal. Release a previously held job, only jobs in the USER_ON_HOLD 
 	 *  state cen be released. On SUCCESS the job will enter the QUEUED_ACTIVE
 	 *  state.
 	 */
	public static final int RELEASE   									= 3;
	
	/** TERMINATE signal. The job will be killed, it execution can be synchronized
 	 *  through the {@link #wait} and {@link #synchronize} methods. However, job 
 	 *  rusage information will not be available and these methods will return 
 	 *  DRMAA_ERRNO_NO_RUSAGE.
 	 */
	public static final int TERMINATE 								= 4;
	
	            /* --------------------------------------------------- */

	/** Pre-defined string used to refer to ANY job submitted during a DRMAA 
 	 *  session. Please note that "disposed" jobs will be removed from the job-list
 	 *  associated to the DRMAA session.
 	 */
	public static final java.lang.String JOB_IDS_SESSION_ANY = "DRMAA_JOB_IDS_SESSION_ANY";
	
	/** Pre-defined string used to refer to ALL the jobs submitted during a DRMAA 
 	 *  session. Please note that "disposed" jobs will be removed from the job-list
 	 *  associated to the DRMAA session.
 	 */
	public static final java.lang.String JOB_IDS_SESSION_ALL = "DRMAA_JOB_IDS_SESSION_ALL";
	
	/* -------------------------------------------------------------------------- *
	 * SECTION 2 public methods of the main class
	 * -------------------------------------------------------------------------- */

	/** Initialize DRMAA API library and create a new DRMAA Session. init()
 	 *  method MUST BE called once per DRMAA program BEFORE any DRMAA related
 	 *  methods are used.
 	 *
 	 *  @param contactString is an implementation dependent string which may 
 	 *  be used to specify which DRM system to use. The current GridWay DRMAA 
 	 *  implementation contact MUST be NULL or "localhost".
 	 *  
 	 *  @throws DrmsInitException
	 *  @throws InvalidContactStringException 
	 *  @throws AlreadyActiveSessionException
	 *  @throws DefaultContactStringException
	 *  @throws NoDefaultContactStringSelectedException  
	 *  @throws java.lang.OutOfemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public void init(java.lang.String contactString)
		throws DrmaaException;
	
	/** Disengage from DRMAA library. This routine ends this DRMAA Session, 
 	 *  but does not effect any jobs (i.e. queued and running jobs remain 
 	 *  queued and running).
 	 *
 	 *  @throws DrmsExitException
	 *  @throws NoActiveSessionException  
 	 *  @throws java.lang.OutOfemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
	 */  
	public void exit()
		throws DrmaaException;

	/** This method allocates a new job template. This template is used to 
 	 *  describe the job to be submitted. This is accomplished by setting the 
 	 *  desired scalar and vector attributes to their appropriate values.
 	 * 
 	 *  @return A reference to a {@link JobTemplate} Object.
 	 *  The DRMAA API runtime library will allocate memory for the new template.
 	 *  This memory MUST be freed with a subsequent call to 
 	 *  {@link #deleteJobTemplate} method.
 	 *
 	 *  @throws NoActiveSessionException
 	 *  @throws DrmCommunicationException
 	 *  @throws java.lang.OutOfemoryError
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public JobTemplate createJobTemplate()
		throws DrmaaException;

	/** This method deallocates a job template.
 	 * 
 	 *  @param jt Rerence to a {@link JobTemplate} object. 
 	 *  The {@link JobTemplate} object jt MUST BE previously allocated with a 
 	 *  {@link #createJobTemplate()} method.
 	 * 
 	 *  @throws NoActiveSessionException
 	 *  @throws InvalidJobTemplateException
 	 *  @throws DrmCommunicationException
 	 *  @throws java.lang.OutOfemoryError
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public void deleteJobTemplate(JobTemplate jt)
		throws DrmaaException;
	
	/** This method submits a single job with the attributes defined in the job 
 	 *  template.
 	 *    
 	 *  @param jobTemplate Reference to a {@link JobTemplate} object. 
 	 *  The {@link JobTemplate} jobTemplate MUST BE previously allocated with a 
 	 *  {@link #createJobTemplate()} method call. Job template values MUST be
 	 *  previously defined with set methods.
	 *
	 *  @return An unique {@link String} identification as provided by the GridWay
 	 *  system.
 	 *
	 *  @throws TryLaterException
	 *  @throws DeniedByDrmException 
	 *  @throws InvalidJobTemplateException
	 *  @throws NoActiveSessionException
	 *  @throws DrmCommunicationException
	 *  @throws java.lang.OutOfemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */ 
	public java.lang.String runJob(JobTemplate jobTemplate)
		throws DrmaaException;
	
	/** Submits a set of parametric jobs tha can be run concurrently. 
 	 *  For each parametric job the same template is used, and so must be properly 
 	 *  set. Each job is identical except of it's index: </p>
	 *  
 	 *      <ul><li> The first parametric job has an index equal to beginIndex.</li>
 	 *      <t><li> The next job has an index equal to beginIndex + step, and so on.</li>
 	 *      <t><li> The last job has an index equal to beginIndex + n *step, where n is equal to
 	 *      				(endIndex-beginIndex)/step.
 	 *  These values can be used as arguments for each task and to generate input/output
 	 *  filenames.</li></ul></p>
 	 * 
 	 *  GridWay will internally rescale the start-end range to 0-total_tasks
 	 *  The coherence of start, end and incr values are not
 	 *  check by runJob(). Their coherence SHOULD be guarantee by the calling
 	 *  program.
 	 *
 	 *  @param jobTemplate Reference to a {@link JobTemplate} structure. 
 	 *  The {@link JobTemplate} jobTemplate MUST BE previously allocated with a 
 	 *  {@link #createJobTemplate()} method call. Job template values MUST be
 	 *  previously defined with sets methods calls.
 	 *
	 *  @param beginIndex index associated to the first job. The smallest start value is 0
	 *
	 *  @param endIndex index associated to the last job.	
 	 * 
	 *  @param step increment used to obtain the total number of job. GridWay
   	 *  will internally used incr=1
 	 *
	 *  @throws TryLaterException
	 *  @throws DeniedByDrmException 
	 *  @throws InvalidJobTemplateException
	 *  @throws NoActiveSessionException
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
     *  @throws java.lang.OutOfemoryError
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */   
	public  java.util.List runBulkJobs(JobTemplate jobTemplate, int beginIndex, int endIndex, int step)
		throws DrmaaException;
	

	/** This method allows the job specified by jobid to be controlled according 
 	 *  to a given action. Possible action to be performed over a given job are:
 	 *  <ul>
 	 *  <li> {@link #SUSPEND} A job will be stopped, and restart files 
 	 *  tranferred back to the client. These files if provided by the running job 
 	 *  will be used on RESUME to re-start execution.</li>
 	 *
 	 * <li> {@link #RESUME}  A previously stopped job will be resumed.
 	 *  If re-start files are provided the job will used them to re-start execution,
 	 *  from the last checkpointing context.</li>
 	 *
 	 * <li> {@link #TERMINATE} The job will be killed, it execution can be 
 	 *  synchronized through the drmaa_wait and drmaa_synchronize method calls. 
 	 *  However, job rusage information will not be available.</li>
 	 * 
 	 * <li> {@link #HOLD} The job will be held, it execution will not 
 	 *  start until it is released. Only jobs in the QUEUED_ACTIVE state can be held.
 	 * 	
 	 * <li> {@link #RELEASE} The job will be released and scheduled, only jobs
 	 *  in the USER_ON_HOLD state can be released.
 	 * </ul> 
	 *
 	 *  @param jobName String with the job unique identification as 
 	 *  provided by the GridWay system. The jobid SHOULD be obtained from a 
 	 *  {@link #runJob} or {@link #runBulkJobs} methods calls.
 	 *
 	 *  @param operation The action to be performed over the job whose value 
 	 *  may be one of the following: {@link #SUSPEND}, {@link #RESUME},
 	 *  {@link #TERMINATE}, {@link #HOLD} or {@link #RELEASE}. 
 	 *
 	 *  @throws NoActiveSessionException
	 *  @throws ResumeInconsistentStateException
	 *  @throws SuspendInconsistentStateException
	 *  @throws HoldInconsistentStateException     
	 *  @throws ReleaseInconsistentStateException     
	 *  @throws InvalidJobException     
	 *  @throws java.lang.OutOfemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */ 
	public void control(java.lang.String jobName, int operation)
		throws DrmaaException;

	/** This method blocks until all jobs specified by jobList have completed 
 	 *  execution or fail.
 	 *
 	 *  @param jobList  A NULL terminated list of jobid strings.
 	 *  The jobids SHOULD be obtained from a {@link #runJob} or 
 	 *  {@link #runBulkJobs} methods calls. The pre-defined value 
 	 *  {@link #JOB_IDS_SESSION_ALL} can be used to synchronize all jobs submitted
 	 *  during the DRMAA session.  Please note that "disposed" jobs will be removed 
 	 *  from the job-list associated to the DRMAA session.
 	 *
 	 *  @param timeout The current GridWay DRMAA implementation 
 	 *  only supports {@link #TIMEOUT_WAIT_FOREVER}, it 
 	 *  specifies an undetermined amount of time. 
 	 *
 	 *  @param dispose If dispose is equal to 1 the jobid will be killed, and
 	 *  its resources freed in the GridWay system. Therefore subsequent calls on 
 	 *  this job will fail. However, if dispose is equal to 0 the job remains in
 	 *  {@link #DONE} state in the GridWay system and its rusage statistics can be 
 	 *  obtained with {@link #wait} method call. Also these jobid will not make
 	 *  subsequent calls synchronize method call to fail.
 	 *
	 *  @throws NoActiveSessionException
	 *  @throws ExitTimeoutException
	 *  @throws InvalidJobException
	 *  @throws java.lang.OutOfemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public void synchronize(java.util.List  jobList, long timeout, boolean dispose)
		throws DrmaaException,  java.lang.RuntimeException;
		
	/** This method waits for a given job to either finish executing or fail. 
 	 *  If successfully waited, the jobs rusage information has been reaped, 
 	 *  and further calls to wait with this job_id will throw
 	 *  {@link InvalidJobException}.
 	 *
 	 *  @param jobName String with the job unique identification as 
 	 *  provided by the GridWay system. The jobid SHOULD be obtained from a 
 	 *  {@link #runJob} or {@link #runBulkJobs} methods calls. 
 	 *  {@link #JOB_IDS_SESSION_ANY} can be used to wait on any job submitted
 	 *  during the DRMAA session. Please note that "disposed" jobs will be removed 
 	 *  from the job-list associated to the DRMAA session.
 	 *
	 *  @param timeout The current GridWay DRMAA implementation 
 	 *  only supports {@link #TIMEOUT_WAIT_FOREVER}. {@link #TIMEOUT_WAIT_FOREVER}, 
 	 *  specifies an undetermined amount of time. 
 	 *
	 *  @return {@link JobInfo} object that stores the exit status and the resource usage of the job_out. 
 	 *	 
	 *  @throws NoActiveSessionException 
	 *  @throws ExitTimeoutException
	 *  @throws InvalidJobException
	 *  @throws java.lang.OutOfemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
	 */
	public JobInfo wait(java.lang.String  jobName, long timeout)
	throws DrmaaException;
	
	/** Obtains the status of a given job.
 	 * 
 	 *  @param jobName String with the job unique identification as 
 	 *  provided by the GridWay system. The jobid SHOULD be obtained from a 
 	 *  {@link #runJob} or {@link #runBulkJobs} method calls.
 	 *
 	 *  @return The actual state of the job can be one
 	 *  of the following:<p>
	 *  <ul>
 	 *      <li> {@link #UNDETERMINED}: An UNDETERMINED state can either obtained due
 	 *  to a  communication error with the GridWay daemon, or because the job has
 	 *  not been initialized yet.</li>
 	 *
 	 *      <li> {@link #QUEUED_ACTIVE}: The job has been successfully submitted and 
 	 *  it is pending to be scheduled.</li>
 	 *
 	 *      <li> {@link #RUNNING}: The job has been successfully submitted to a 
 	 * remote host. Please note that once submitted, the job can be in any of the 
 	 * execution stages, namely: prolog (file stage-in), wrapper (execution), 
 	 * epilog (file stage-out) or migrating (to another host).</li>
 	 *
 	 *      <li> {@link #USER_ON_HOLD}: The job has been held by the user</li>
 	 *
 	 *      <li> {@link #DONE}: Job has been completely executed and output files are 
 	 *  available at the client. drmaa_wait() and drmaa_synchronize() calls on the 
 	 *  job will return immediately. Also rusage information is available. </li>
 	 *
 	 *      <li> {@link #DONE}: Job has been completely executed and output files are 
 	 *  available at the client. drmaa_wait() and drmaa_synchronize() calls on the 
 	 *  job will return immediately. Also rusage information is available. </li>
 	 *
 	 *      <li> {@link #FAILED}: Job execution has failed, and the "on_failure" policy
 	 *  is to hold it on FAILED state.</li></ul>
 	 *
 	 *  The GridWay DRMAA implementation does not define the following
 	 *  actions: {@link #SYSTEM_ON_HOLD}, {@link #USER_SYSTEM_ON_HOLD}, 
 	 *  {@link #SYSTEM_SUSPENDED} and {@link #USER_SYSTEM_SUSPENDED}.
 	 *
 	 *  @throws NoActiveSessionException
	 *  @throws InvalidJobException
	 *  @throws java.lang.OutOfemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 *  
 	 */      
	public int getJobStatus(java.lang.String jobName)
	throws DrmaaException;
	
	/** If called before {@link #init}, this method returns a string containing a 
 	 *  comma-delimited list of default contact hosts for the GridWay daemon. If called 
 	 *  after {@link #init}, this funtion returns the contact string (hostname) 
 	 *  where GridWay is running. The client library has been initialized by contacting this
 	 *  host.
 	 * 
 	 *  @return  The contact string
 	 *
 	 *  @throws java.lang.OutOfemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public  java.lang.String getContact();
	
	/** This method sets major and minor to the major and minor versions of the DRMAA C
 	 *  binding specification implemented by the DRMAA implementation. Current implementation
 	 *  is 1.0
 	 *
 	 * 
 	 *  @return  Returns the version number as a Version object 
 	 *
 	 *  @throws java.lang.OutOfemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public Version getVersion();
	
	 
	/** This method always returns "GridWay" in drm_system, the only DRM system supported
 	 *  by the GridWay DRMAA implementation
 	 * 
 	 *  @return Always "GridWay"  
 	 */
	public java.lang.String getDrmsInfo();
	
	/** This method returns the DRMAA implementation. It always returns 
 	 *  "DRMAA for GridWay M.m" where
 	 *  M is the GridWay major version number and m is the minor version number  
 	 * 
 	 *  @return Always "DRMAA for GridWay M.m" 
 	 *
 	 *  @throws java.lang.OutOfemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public java.lang.String getDrmaaImplementation();	
}
