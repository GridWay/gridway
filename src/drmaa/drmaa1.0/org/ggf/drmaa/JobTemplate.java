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
import java.util.*;


/** DRMAA application uses the JobTemplate  interface, in order to define the attributes associated with a job. 
 * JobTemplates are created via the active {@link Session} implementation. A DRMAA application
 * gets a JobTemplate from the active {@link Session}, specifies in the JobTemplate any required job
 * parameters, and the passes the JobTemplate back to the Session when requesting that a job be
 * executed. When finished, the DRMAA application should call the
 * {@link Session#deleteJobTemplate} method to allow the underlying implementation to free any
 * resources bound to the JobTemplate object. 
 */

public abstract interface  JobTemplate 
{
	
	/** Pre-defined string to refer to the HOLD state on submission. 
 	 *  Use this preprocessor directive to assign the value of the 
 	 *  drmaa_js_state attribute through the {@link #setJobSubmissionState} method call.
 	 */ 
	public static final int 	HOLD_STATE						= 0;//"drmaa_hold";
	
	/** Pre-defined string to refer to the ACTIVE state on submission. Use this
 	 *  preprocessor directive to assign the value of the drmaa_js_state attribute
 	 *  through the {@link #setJobSubmissionState} method call.
 	 */
	public static final int 	ACTIVE_STATE					= 1;//"drmaa_active";
	
	/** Pre-defined string to refer the user's home directory.
 	*/ 
	public static final String 	HOME_DIRECTORY			= "$drmaa_hd_ph$";
	
	/** Pre-defined string constant to represent the current working
 	 *  directory when building paths for the input, output, and error path attribute values.
 	 *  Plase note that ALL FILES MUST BE NAMED RELATIVE TO THE WORKING DIRECTORY. 
 	 */
	public static final String 	WORKING_DIRECTORY		= "$drmaa_wd_ph$";
	
	/** Pre-defined string to be used in parametric jobs (bulk jobs). 
 	 *  PARAMETRIC_INDEX will be available during job execution and can be
 	 *  used as an ARGUMENT for the REMOTE COMMAND, or to generate output filenames.
 	 *  Please note that this attribute name should be used ONLY  in conjuntion 
 	 *  with a {@link Session#runBulkJobs} method call. Use DRMAA_GW_JOB_ID for "stand-alone" jobs.
 	 */ 
	public static final String	PARAMETRIC_INDEX	= "$drmaa_incr_ph$";
	

	/** This method set the attribute {@link SimpleJobTemplate#remoteCommand}. 
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
		throws DrmaaException;

	/** This method set the attribute {@link SimpleJobTemplate#args}. 
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
		throws DrmaaException;
	
	/** This method set the attribute {@link SimpleJobTemplate#jobSubmissionState}. 
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
		throws DrmaaException;	
	
	/** This method set the attribute {@link SimpleJobTemplate#jobEnvironment}. 
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
		throws DrmaaException;
	
	/** This method set the attribute {@link SimpleJobTemplate#workingDirectory}. 
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
		throws DrmaaException;

	/** This method set the attribute {@link SimpleJobTemplate#jobCategory}. 
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
		throws DrmaaException;
	
	/** This method set the attribute {@link SimpleJobTemplate#nativeSpecification}. 
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
		throws DrmaaException;

	
	/** This method set the attribute {@link SimpleJobTemplate#email}. 
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
	public  void setEmail(java.util.Set email)
		throws DrmaaException;
	
	/** This method set the attribute {@link SimpleJobTemplate#blockEmail}. 
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
		throws DrmaaException;
	
	/** This method set the attribute {@link SimpleJobTemplate#startTime}. 
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
		throws DrmaaException;
	
	/** This method set the attribute {@link SimpleJobTemplate#jobName}. 
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
		throws DrmaaException;
	
	/** This method set the attribute {@link SimpleJobTemplate#inputPath}. 
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
		throws DrmaaException;
	
	/** This method set the attribute {@link SimpleJobTemplate#outputPath}. 
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
		throws DrmaaException;
	
	/** This method set the attribute {@link SimpleJobTemplate#errorPath}. 
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
		throws DrmaaException;
	
	/** This method set the attribute {@link SimpleJobTemplate#joinFiles}. 
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
		throws DrmaaException;
	
	/** This method set the attribute {@link SimpleJobTemplate#transferFiles}. 
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
		throws DrmaaException;
	
	/** This method set the attribute {@link SimpleJobTemplate#deadlineTime}. 
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
		throws DrmaaException;
	
	/** This method set the attribute {@link SimpleJobTemplate#hardWallclockTimeLimit}. 
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
		throws DrmaaException;
	
	/** This method set the attribute {@link SimpleJobTemplate#softWallclockTimeLimit}. 
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
		throws DrmaaException;
	
	/** This method set the attribute {@link SimpleJobTemplate#hardRunDurationLimit}. 
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
		throws DrmaaException;
	
	/** This method set the attribute {@link SimpleJobTemplate#softRunDurationLimit}. 
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
		throws DrmaaException;
	
	/** This method get the attribute {@link SimpleJobTemplate#remoteCommand}. 
 	 * 
	 *  @return A {@link String} with the remoteCommand value
 	 *
 	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public  String getRemoteCommand()
		throws DrmaaException;		
	
	/** This method get the attribute {@link SimpleJobTemplate#args}. 
 	 * 
	 *  @return A {@link String} array with the args value
 	 *
	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException 	
 	 */
	public  java.util.List getArgs()
		throws DrmaaException;
	
	/** This method get the attribute {@link SimpleJobTemplate#jobSubmissionState}. 
 	 * 
	 *  @return A {@link String} with the jobSubmissionState value
	 
	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException 	
 	 */
	public  int getJobSubmissionState()
		throws DrmaaException;
	
	/** This method get the attribute {@link SimpleJobTemplate#jobEnvironment}. 
 	 * 
	 *  @return A {@link Properties} object with the jobEnvironment value
 	 *
 	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
	 */
	public  java.util.Map getJobEnvironment()
		throws DrmaaException;
	
	/** This method get the attribute {@link SimpleJobTemplate#workingDirectory}. 
 	 * 
	 *  @return A {@link String} with the workingDirectory value
 	 *
	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public  String getWorkingDirectory()
		throws DrmaaException;

	/** This method get the attribute {@link SimpleJobTemplate#jobCategory}. 
 	 * 
	 *  @return A {@link String} with the jobCategory value
 	 *
 	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public  String getJobCategory()
		throws DrmaaException;
	
	/** This method get the attribute {@link SimpleJobTemplate#nativeSpecification}. 
 	 * 
	 *  @return A {@link String} with the nativeSpecificationnativeSpecification value
 	 *
 	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public  String getNativeSpecification()
		throws DrmaaException;
	
	/** This method get the attribute {@link SimpleJobTemplate#email}. 
 	 * 
	 *  @return A {@link String} array with the email value
 	 *
 	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public  java.util.Set getEmail()
		throws DrmaaException;
	
	/** This method get the attribute {@link SimpleJobTemplate#blockEmail}. 
 	 * 
	 *  @return A boolean with the blockEmail value
 	 *
 	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public  boolean getBlockEmail()
		throws DrmaaException;
	
	/** This method get the attribute {@link SimpleJobTemplate#startTime}. 
 	 * 
	 *  @return A PartialTimestamp object  with the startTime value
 	 *
 	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public  Date getStartTime()
		throws DrmaaException;
	
	/** This method get the attribute {@link SimpleJobTemplate#jobName}. 
 	 * 
	 *  @return A {@link String} with the jobName value
 	 *
 	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public  String getJobName()
		throws DrmaaException;
	
	/** This method get the attribute {@link SimpleJobTemplate#inputPath}. 
 	 * 
	 *  @return A {@link String} with the inputPath value
 	 *
 	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public  String getInputPath()
		throws DrmaaException;
	
	/** This method get the attribute {@link SimpleJobTemplate#outputPath}. 
 	 * 
	 *  @return A {@link String} with the outputPath value
 	 *
 	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public  String getOutputPath()
		throws DrmaaException;	
	
	/** This method get the attribute {@link SimpleJobTemplate#errorPath}. 
 	 * 
	 *  @return A {@link String} with the errorPath value
 	 *
 	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public  String getErrorPath()
		throws DrmaaException;	
	
	/** This method get the attribute {@link SimpleJobTemplate#joinFiles}. 
 	 * 
	 *  @return A boolean with the joinFiles value
 	 *
 	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public  boolean getJoinFiles()
		throws DrmaaException;
	
	/** This method get the attribute {@link SimpleJobTemplate#transferFiles}. 
 	 * 
	 *  @return A FileTransferMode object with the transferFiles value
 	 *
 	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public  FileTransferMode getTransferFiles()
		throws DrmaaException;
	
	/** This method get the attribute {@link SimpleJobTemplate#deadlineTime}. 
 	 * 
	 *  @return A PartialTimestamp object with the deadlineTime value
 	 *
 	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public  PartialTimestamp getDeadlineTime()
		throws DrmaaException;
	
	/** This method get the attribute {@link SimpleJobTemplate#hardWallclockTimeLimit}. 
 	 * 
	 *  @return A long with the hardWallclockTimeLimit value
 	 *
 	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public  long getHardWallclockTimeLimit()
		throws DrmaaException;
	
	/** This method get the attribute {@link SimpleJobTemplate#softWallclockTimeLimit}. 
 	 * 
	 *  @return A long with the softWallclockTimeLimit value
 	 *
 	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public  long getSoftWallclockTimeLimit()
		throws DrmaaException;	
	
	/** This method get the attribute {@link SimpleJobTemplate#hardRunDurationLimit}. 
 	 * 
	 *  @return A long with the hardRunDurationLimit value
 	 *
 	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public  long getHardRunDurationLimit()
		throws DrmaaException;	
	
	/** This method get the attribute {@link SimpleJobTemplate#softRunDurationLimit}. 
 	 * 
	 *  @return A long with the softRunDurationLimit value
 	 *
 	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public  long getSoftRunDurationLimit()
		throws DrmaaException;
	
	/** This method return the list of supported property names.
 	 *  The required Gridway property names are:
	 * <ul>
	 * <li> {@link SimpleJobTemplate#remoteCommand}, {@link SimpleJobTemplate#args}, {@link SimpleJobTemplate#jobSubmissionState}</li>
	 * <li> {@link SimpleJobTemplate#jobEnvironment}, {@link SimpleJobTemplate#workingDirectory}, {@link SimpleJobTemplate#jobName}</li>		
	 * <li> {@link SimpleJobTemplate#inputPath}, {@link SimpleJobTemplate#outputPath}, {@link SimpleJobTemplate#errorPath}</li>		
	 * </ul>
	 * 
	 *  The optional Gridway property names (implemented in {@link GridWayJobTemplate}) are:
	 * <ul>
	 * <li> inputFiles, outputFiles, restartFiles</li>
	 * <li> rescheduleOnFailure, numberOfRetries, rank</li>
	 * <li>	requirements</li>
	 * </ul>
 	 *
  	 *  @throws NoActiveSessionException
	 *  @throws java.lang.OutOfMemoryError
	 *  @throws DrmCommunicationException
	 *  @throws AuthorizationException
	 *  @throws java.lang.IllegalArgumentException
	 *  @throws InternalException
 	 */
	public  java.util.Set getAttributeNames()
		throws DrmaaException;
	
	
}
