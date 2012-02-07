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

import java.util.*;

public class SimpleJobTemplate implements JobTemplate
{
	
	/** Attribute to refer to the command to be executed on the remote 
 	 *  host. The remoteCommand attribute MUST BE RELATIVE to the working directory 
 	 *  (workingDirectory). Architecture-dependent remoteCommand can be generated
 	 *  with GW_ARCH.
 	 */
	
	protected java.lang.String			remoteCommand = null;
	
	/** String array attribute to refer to the remoteCommand arguments.  
 	 */
	protected java.util.List				args = null;
	
	/** Integer attribute to refer to the job state at submission, the job will 
 	 *  enter either the QUEUED_ACTIVE state or HOLD state when submitted. The 
 	 *  preprocessor directives ACTIVE_STATE and 
 	 *  HOLD_STATE SHOULD be used to assign the value of this attribute. 
 	 *  The default value is ACTIVE_STATE.
 	 */
	protected int							jobSubmissionState = ACTIVE_STATE;
	
	/** String attribute to refer to the {@link #remoteCommand} environment 
 	 *  variables. 
	 */
	protected java.util.Map				jobEnvironment = null;
	

	/** String attribute to refer to the JOB working directory. The current
 	 *  GridWay DRMAA implementation will generate a job template file with
 	 *  name {@link #jobName} in the job working directory (workingDirectory). It is a
	 *  MANDATORY attribute value and MUST BE DEFINED. Plase note that ALL FILES
     *  ARE NAMED RELATIVE TO THE WORKING DIRECTORY. Also this is a LOCAL PATH NAME, 
     *  this directory will be recreated in the remote host, and it will be the 
     *  working directory of the job on the execution host.
     */ 
	protected java.lang.String			workingDirectory = null;

	/** Not relevant for the current GridWay implementation, will be ignored
 	*/
	protected java.lang.String			jobCategory = null;
	
	/** Not relevant for the current GridWay implementation, will be ignored
 	*/
	protected java.lang.String			nativeSpecification = null;
	
	/** Not relevant for the current GridWay implementation, will be ignored
 	 */
	protected java.util.Set				email = null;
	
	/** Not relevant for the current GridWay implementation, will be ignored
 	*/
	protected boolean					blockEmail = false;
	
	/** Not relevant for the current GridWay implementation, will be ignored
 	*/
	protected java.util.Date				startTime = null;
	
	
	/** String attribute to refer to the DRMAA job-name. The current GridWay
 	*  DRMAA implementation will generate a job template file with name 
 	*  jobName in the job working directory (workingDirectory). jobName is
 	*  a MANDATORY attribute value and MUST BE DEFINED. The default value is 
 	*  "job_template".
 	*/
	protected java.lang.String			jobName = null;
	
	/** String attribute to refer to standard input file for the  
 	 *  {@link #remoteCommand}. The standard input file IS RELATIVE TO THE 
 	 *  WORKING DIRECTORY.
 	 */
	protected java.lang.String			inputPath = null;
	
	/** String attribute to refer to standard output file for the  
 	 *  {@link #remoteCommand}. The standard ouput file IS RELATIVE TO THE 
 	 *  WORKING DIRECTORY.
 	 */
	protected java.lang.String			outputPath = null;
	
	/** String attribute to refer to standard error file for the  
 	 *  {@link #remoteCommand}. The standard error file IS RELATIVE TO THE 
 	 *  WORKING DIRECTORY.
 	 */
	protected java.lang.String			errorPath = null;
	
	/** Not relevant for the current GridWay implementation, will be ignored
 	*/
	protected boolean					joinFiles = false;
	
	/** Not relevant for the current GridWay implementation, will be ignored
 	*/
	protected FileTransferMode		transferFiles = null;
	
	/**Pre-defined string to represent a deadline for job execution. GridWay WILL NOT 
	  * terminate a job after the deadline, neither guarantees that the job is executed before the 
	  * deadline. A deadline is specified absolute to job submission time.
 	*/
	protected PartialTimestamp		deadlineTime = null;
	
	/** Not relevant for the current GridWay implementation, will be ignored
 	*/
	protected long 						hardWallclockTimeLimit = 0;
	
	/** Not relevant for the current GridWay implementation, will be ignored
 	*/
	protected long 						softWallclockTimeLimit = 0;
	
	/** Not relevant for the current GridWay implementation, will be ignored
 	*/
	protected long 						hardRunDurationLimit = 0;
	
	/** Not relevant for the current GridWay implementation, will be ignored
 	*/
	protected long 						softRunDurationLimit = 0;	
	

	/* Specific attributes*/
	
	protected java.util.Set 				attributeNames = null ;
	
	private java.util.HashMap			jobTemplateString = null;
	
	/** The SimpleJobTemplate() constructor creates a new instance of a SimpleJobTemplate. In most cases, a
	 * DRMAA implementation will require that JobTemplates be created through the
	 * {@link Session#createJobTemplate} method, however. In those cases, passing a JobTemplate created
	 * through the JobTemplate() constructor to the {@link Session#deleteJobTemplate}, {@link Session#runJob},
	 * or {@link Session#runBulkJobs} methods will result in an {@link InvalidJobTemplateException} being thrown.
	 */
	public SimpleJobTemplate()
	{
		this.jobName = new String("SimpleJobTemplate");		
		
		this.attributeNames = new java.util.HashSet();
		
		attributeNames.add("remoteCommand");
		attributeNames.add("args");
		attributeNames.add("jobSubmissionState");
		attributeNames.add("jobEnvironment");
		attributeNames.add("workingDirectory");	
		attributeNames.add("jobName");
		attributeNames.add("inputPath");		
		attributeNames.add("outputPath");		
		attributeNames.add("errorPath");						
		attributeNames.add("deadlineTime");
	}
		
	/** This method set the attribute {@link #remoteCommand}. 
 	 * 
	 *  @param command A command to set
 	 *
	 *  @throws InvalidAttributeValueException
	 *  @throws ConflictingAttributeValuesException
	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */ 	
	public  void setRemoteCommand(java.lang.String command)
		throws DrmaaException
	{
		if (command == null)
			throw new InvalidAttributeValueException("The command attribute is NULL");	
		else
			this.remoteCommand = new String(command);
	}

	/** This method set the attribute {@link #args}. 
 	 * 
	 *  @param args The attributes to set
 	 *
	 *  @throws InvalidAttributeValueException
	 *  @throws ConflictingAttributeValuesException
	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */ 
	public  void setArgs(java.util.List args)
		throws DrmaaException
	{
		if (args == null)
			throw new InvalidAttributeValueException("The args attribute is NULL");	
		else
			this.args = new ArrayList<String>((ArrayList<String>) args);
	}
	
	/** This method set the attribute {@link #jobSubmissionState}. 
 	 * 
	 *  @param state The state to set
 	 *
 	 *  @throws InvalidAttributeValueException
	 *  @throws ConflictingAttributeValuesException
	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public  void setJobSubmissionState(int state)
		throws DrmaaException
	{
		this.jobSubmissionState = state;
	}
	
	/** This method set the attribute {@link #jobEnvironment}. 
 	 * 
	 *  @param env The jobEnvironment to set
 	 *
	 *  @throws InvalidAttributeValueException
	 *  @throws ConflictingAttributeValuesException
	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public  void setJobEnvironment(java.util.Map env)
		throws DrmaaException
	{
		if (env == null)
			throw new InvalidAttributeValueException("The env attribute is NULL");	
		else
		{
			this.jobEnvironment = new Properties();
			
			Enumeration e = ((Properties) env).propertyNames();
			
			while (e.hasMoreElements())
			{
				String name  = (String) e.nextElement();
				String value = ((Properties) env).getProperty(name);
					
				((Properties) this.jobEnvironment).setProperty(name, value);
			}
		}
			
	}
	
	/** This method set the attribute {@link #workingDirectory}. 
 	 * 
	 *  @param wd The working directoy to set
 	 *
	 *  @throws InvalidAttributeValueException
	 *  @throws ConflictingAttributeValuesException
	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public  void setWorkingDirectory(String wd)
		throws DrmaaException
	{
		if (wd == null)
			throw new InvalidAttributeValueException("The workingDirectory attribute is NULL");	
		else
			this.workingDirectory = new String(wd);
	}

	/** This method set the attribute {@link #jobCategory}. 
 	 * 
	 *  @param category The category to set
 	 *
	 *  @throws InvalidAttributeValueException
	 *  @throws ConflictingAttributeValuesException
	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public  void setJobCategory(String category)
		throws DrmaaException
	{
		if (category == null)
			throw new InvalidAttributeValueException("The category attribute is NULL");	
		else
			this.jobCategory = new String(category);
	}
	
	/** This method set the attribute {@link #nativeSpecification}. 
 	 * 
	 *  @param spec The native specification to set
 	 *
	 *  @throws InvalidAttributeValueException
	 *  @throws ConflictingAttributeValuesException
	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public  void setNativeSpecification(String spec)
		throws DrmaaException
	{
		if (spec == null)
			throw new InvalidAttributeValueException("The nativeSpecification attribute is NULL");	
		else
			this.nativeSpecification = new String(spec);
	}

	
	/** This method set the attribute {@link #email}. 
 	 * 
	 *  @param email The email to set
 	 *
	 *  @throws InvalidAttributeValueException
	 *  @throws ConflictingAttributeValuesException
	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public  void setEmail(Set email)
		throws DrmaaException
	{
		if (email == null)
			throw new InvalidAttributeValueException("The email attribute is NULL");	
		else
			this.email = new HashSet<String>((HashSet<String>)email);
	}		

	
	/** This method set the attribute {@link #blockEmail}. 
 	 * 
	 *  @param blockEmail The blockEmail to set
 	 *
	 *  @throws InvalidAttributeValueException
	 *  @throws ConflictingAttributeValuesException
	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public  void setBlockEmail(boolean blockEmail)
		throws DrmaaException
	{
		this.blockEmail = blockEmail;
	}
	
	/** This method set the attribute {@link #startTime}. 
 	 * 
	 *  @param startTime The startTime to set
 	 *
	 *  @throws InvalidAttributeValueException
	 *  @throws ConflictingAttributeValuesException
	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public  void setStartTime(Date startTime)
		throws DrmaaException
	{
		if (startTime == null)
			throw new InvalidAttributeValueException("The startTime attribute is NULL");	
		else
			this.startTime = startTime;
	}
	
	/** This method set the attribute {@link #jobName}. 
 	 * 
	 *  @param name The Job name to set
 	 *
	 *  @throws InvalidAttributeValueException
	 *  @throws ConflictingAttributeValuesException
	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public  void setJobName(String name)
		throws DrmaaException
	{
		if (name == null)
			throw new InvalidAttributeValueException("The JobTemplate name attribute is NULL");	
		else
			this.jobName = new String(name);	
	}		
	
	/** This method set the attribute {@link #inputPath}. 
 	 * 
	 *  @param inputPath The input path to set
 	 *
	 *  @throws InvalidAttributeValueException
	 *  @throws ConflictingAttributeValuesException
	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public  void setInputPath(String inputPath)
		throws DrmaaException
	{
		if (inputPath == null)
			throw new InvalidAttributeValueException("The inputPath attribute is NULL");	
		else
			this.inputPath = new String(inputPath);		
	}		
	
	/** This method set the attribute {@link #outputPath}. 
 	 * 
	 *  @param outputPath The output path to set
 	 *
	 *  @throws InvalidAttributeValueException
	 *  @throws ConflictingAttributeValuesException
	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public  void setOutputPath(String outputPath)
		throws DrmaaException
	{
		if (outputPath == null)
			throw new InvalidAttributeValueException("The outputPath attribute is NULL");	
		else
			this.outputPath = new String(outputPath);		
	}
	
	/** This method set the attribute {@link #errorPath}. 
 	 * 
	 *  @param errorPath The error path to set
 	 *
	 *  @throws InvalidAttributeValueException
	 *  @throws ConflictingAttributeValuesException
	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public  void setErrorPath(String errorPath)
		throws DrmaaException
	{
		if (errorPath == null)
			throw new InvalidAttributeValueException("The errorPath attribute is NULL");	
		else
			this.errorPath = new String(errorPath);		
	}
	
	/** This method set the attribute {@link #joinFiles}. 
 	 * 
	 *  @param joinFiles The joinFiles to set
 	 *
	 *  @throws InvalidAttributeValueException
	 *  @throws ConflictingAttributeValuesException
	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public  void setJoinFiles(boolean joinFiles)
		throws DrmaaException
	{
		this.joinFiles = joinFiles;		
	}
	
	/** This method set the attribute {@link #transferFiles}. 
 	 * 
	 *  @param mode The transfer mode to set
 	 *
	 *  @throws InvalidAttributeValueException
	 *  @throws ConflictingAttributeValuesException
	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public  void setTransferFiles(FileTransferMode mode)
		throws DrmaaException
	{
		if (mode == null)
			throw new InvalidAttributeValueException("The fileTransferMode attribute is NULL");	
		else
			this.transferFiles = mode;		
	}
	
	/** This method set the attribute {@link #deadlineTime}. 
 	 * 
	 *  @param deadline The deadline to set
 	 *
	 *  @throws InvalidAttributeValueException
	 *  @throws ConflictingAttributeValuesException
	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public  void setDeadlineTime(PartialTimestamp deadline)
		throws DrmaaException
	{
		if (deadline == null)
			throw new InvalidAttributeValueException("The deadline attribute is NULL");	
		else
			this.deadlineTime = deadline;		
	}
	
	/** This method set the attribute {@link #hardWallclockTimeLimit}. 
 	 * 
	 *  @param limit The hard wall clock time limit to set
 	 *
	 *  @throws InvalidAttributeValueException
	 *  @throws ConflictingAttributeValuesException
	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public  void setHardWallclockTimeLimit(long limit)
		throws DrmaaException
	{
		this.hardWallclockTimeLimit = limit;		
	}
	
	/** This method set the attribute {@link #softWallclockTimeLimit}. 
 	 * 
	 *  @param limit The soft wall clock time timit to set
 	 *
	 *  @throws InvalidAttributeValueException
	 *  @throws ConflictingAttributeValuesException
	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public  void setSoftWallclockTimeLimit(long limit)
		throws DrmaaException
	{
		this.softWallclockTimeLimit = limit;		
	}
	
	/** This method set the attribute {@link #hardRunDurationLimit}. 
 	 * 
	 *  @param limit The hard run duration limit to set
 	 *
	 *  @throws InvalidAttributeValueException
	 *  @throws ConflictingAttributeValuesException
	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public  void setHardRunDurationLimit(long limit)
		throws DrmaaException
	{
		this.hardRunDurationLimit = limit;		
	}
	
	/** This method set the attribute {@link #softRunDurationLimit}. 
 	 * 
	 *  @param limit The soft run duration limit to set
 	 *
	 *  @throws InvalidAttributeValueException
	 *  @throws ConflictingAttributeValuesException
	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public  void setSoftRunDurationLimit(long limit)
		throws DrmaaException
	{
		this.softRunDurationLimit = limit;
	}
	
	/** This method get the attribute {@link #remoteCommand}. 
 	 * 
	 *  @return A {@link String} with the remoteCommand value
 	 *
 	 */
	public  String getRemoteCommand()		
	{
		return this.remoteCommand;
	}

	/** This method get the attribute {@link #args}. 
 	 * 
	 *  @return A {@link List} object with the args value
 	 *
	 *  @throws InvalidAttributeValueException
	 *  @throws ConflictingAttributeValuesException
	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public  java.util.List getArgs()
	{
		return this.args;
	}
	
	/** This method get the attribute {@link #jobSubmissionState}. 
 	 * 
	 *  @return A int with the jobSubmissionState value
 	 *
	 *  @throws InvalidAttributeValueException
	 *  @throws ConflictingAttributeValuesException
	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public  int getJobSubmissionState()
	{
		return this.jobSubmissionState;
	}
	
	/** This method get the attribute {@link #jobEnvironment}. 
 	 * 
	 *  @return A {@link Map} object with the jobEnvironment value
 	 *
	 *  @throws InvalidAttributeValueException
	 *  @throws ConflictingAttributeValuesException
	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public  java.util.Map getJobEnvironment()
	{
		return this.jobEnvironment;
	}

	/** This method get the attribute {@link #workingDirectory}. 
 	 * 
	 *  @return A {@link String} with the workingDirectory value
 	 *
	 *  @throws InvalidAttributeValueException
	 *  @throws ConflictingAttributeValuesException
	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException 	 
 	 */
	public  String getWorkingDirectory()
	{
		return this.workingDirectory;		
	}

	/** This method get the attribute {@link #jobCategory}. 
 	 * 
	 *  @return A {@link String} with the jobCategory value
 	 *
	 *  @throws InvalidAttributeValueException
	 *  @throws ConflictingAttributeValuesException
	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public  String getJobCategory()
	{
		return this.jobCategory;		
	}
	
	/** This method get the attribute {@link #nativeSpecification}. 
 	 * 
	 *  @return A {@link String} with the nativeSpecification value
 	 *
	 *  @throws InvalidAttributeValueException
	 *  @throws ConflictingAttributeValuesException
	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public  String getNativeSpecification()
	{
		return this.nativeSpecification;		
	}
	
	/** This method get the attribute {@link #email}. 
 	 * 
	 *  @return A {@link Set} object with the email value
	 *
	 *  @throws InvalidAttributeValueException
	 *  @throws ConflictingAttributeValuesException
	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public  Set getEmail()
	{
		return this.email;
	}		

	/** This method get the attribute {@link #blockEmail}. 
 	 * 
	 *  @return A boolean with the blockEmail value
 	 *
	 *  @throws InvalidAttributeValueException
	 *  @throws ConflictingAttributeValuesException
	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public  boolean getBlockEmail()
	{
		return this.blockEmail;
	}
	
	/** This method get the attribute {@link #startTime}. 
 	 * 
	 *  @return A Date object  with the startTime value
 	 *
	 *  @throws InvalidAttributeValueException
	 *  @throws ConflictingAttributeValuesException
	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
 	 */
	public  Date getStartTime()
	{
		return this.startTime;
	}
	
	/** This method get the attribute {@link #jobName}. 
 	 * 
	 *  @return A {@link String} with the jobName value
 	 *
	 *  @throws InvalidAttributeValueException
	 *  @throws ConflictingAttributeValuesException
	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public  String getJobName()
	{
		return this.jobName;
	}		
	
	/** This method get the attribute {@link #inputPath}. 
 	 * 
	 *  @return A {@link String} with the inputPath value
 	 *
	 *  @throws InvalidAttributeValueException
	 *  @throws ConflictingAttributeValuesException
	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public  String getInputPath()
	{
		return this.inputPath;
	}		
	
	/** This method get the attribute {@link #outputPath}. 
 	 * 
	 *  @return A {@link String} with the outputPath value
 	 *
	 *  @throws InvalidAttributeValueException
	 *  @throws ConflictingAttributeValuesException
	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public  String getOutputPath()
	{
		return this.outputPath;
	}
	
	/** This method get the attribute {@link #errorPath}. 
 	 * 
	 *  @return A {@link String} with the errorPath value
 	 *
	 *  @throws InvalidAttributeValueException
	 *  @throws ConflictingAttributeValuesException
	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public  String getErrorPath()
	{
		return this.errorPath;		
	}
	
	/** This method get the attribute {@link #joinFiles}. 
 	 * 
	 *  @return A boolean with the joinFiles value
 	 *
	 *  @throws InvalidAttributeValueException
	 *  @throws ConflictingAttributeValuesException
	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public  boolean getJoinFiles()
	{
		return this.joinFiles;		
	}
	
	/** This method get the attribute {@link #transferFiles}. 
 	 * 
	 *  @return A FileTransferMode object with the transferFiles value
 	 *
	 *  @throws InvalidAttributeValueException
	 *  @throws ConflictingAttributeValuesException
	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public  FileTransferMode getTransferFiles()
	{
		return this.transferFiles;		
	}
	
	/** This method get the attribute {@link #deadlineTime}. 
 	 * 
	 *  @return A PartialTimestamp object with the deadlineTime value
 	 *
	 *  @throws InvalidAttributeValueException
	 *  @throws ConflictingAttributeValuesException
	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public  PartialTimestamp getDeadlineTime()
	{
		return this.deadlineTime;
	}
	
	/** This method get the attribute {@link #hardWallclockTimeLimit}. 
 	 * 
 	 *
	 *  @throws InvalidAttributeValueException
	 *  @throws ConflictingAttributeValuesException
	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public  long getHardWallclockTimeLimit()
	{
		return this.hardWallclockTimeLimit;
	}
	
	/** This method get the attribute {@link #softWallclockTimeLimit}. 
 	 * 
	 *  @return A long with the softWallclockTimeLimit value
 	 *
	 *  @throws InvalidAttributeValueException
	 *  @throws ConflictingAttributeValuesException
	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public  long getSoftWallclockTimeLimit()
	{
		return this.softWallclockTimeLimit;
	}
	
	/** This method get the attribute {@link #hardRunDurationLimit}. 
 	 * 
	 *  @return A long with the hardRunDurationLimit value
 	 *
	 *  @throws InvalidAttributeValueException
	 *  @throws ConflictingAttributeValuesException
	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public  long getHardRunDurationLimit()
	{
		return this.hardRunDurationLimit;
	}
	
	/** This method get the attribute {@link #softRunDurationLimit}. 
 	 * 
	 *  @return A long with the softRunDurationLimit value
 	 *
	 *  @throws InvalidAttributeValueException
	 *  @throws ConflictingAttributeValuesException
	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public  long getSoftRunDurationLimit()
	{
		return this.softRunDurationLimit;
	}
	
	/** This method return the list of supported property names.
 	 *  The required Gridway property names are:
	 * <ul>
	 * <li> {@link #remoteCommand}, {@link #args}, {@link #jobSubmissionState}</li>
	 * <li> {@link #jobEnvironment}, {@link #workingDirectory}, {@link #jobName}, {@link #inputPath}</li>		
	 * <li> {@link #outputPath}, {@link #errorPath}</li>		
	 * </ul>
	 * 
	 *  The optional Gridway property names (implemented in {@link GridWayJobTemplate}) are:
	 * <ul>
	 * <li> inputFiles, outputFiles, restartFiles</li>
	 * <li> rescheduleOnFailure, numberOfRetries, rank</li>
	 * <li> requirements</li>
	 * </ul>
 	 */
	public java.util.Set getAttributeNames()
	{	
		return attributeNames;
	}
	
	/** This method returns a string representation of the job template instance
	 * 
	 */
	public java.lang.String toString()
	{
		String result;
		
		result = "Name: " + this.jobName + "\n";
		result += "Remote Command: " + this.remoteCommand + "\n";
		result += "Arguments: " + this.args + "\n";
		result += "Job Submission State: " + this.jobSubmissionState + "\n";
		result += "Environment: " + this.jobEnvironment + "\n";	
		result += "Input Path: " + this.inputPath + "\n";
		result += "Output Path: " + this.outputPath + "\n";
		result += "Error Path: " + this.errorPath + "\n";
		result += "Deadline Time: " + this.deadlineTime + "\n";
		
		return result;
	}
	
	/** This method marks the job template to indicate that its properties have been modified. 
	 * Not relevant for the current GridWay implementation, will be ignored.
 	 * 
 	 */
	public void modified()
	{		
		return;
	}
	
	/** This method return the list of the optional Gridway property names 
	 * (implemented in {@link GridWayJobTemplate}):
 	 * 
	 * <ul>
	 * <li> inputFiles, outputFiles, restartFiles</li>
	 * <li> rescheduleOnFailure, numberOfRetries, rank</li>
	 * <li> requirements</li>
	 * </ul>
 	 */
	protected  java.util.Set getOptionalAttributeNames()
	{		
		return null;
	}
}
