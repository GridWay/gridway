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


package org.ggf.drmaa;
import java.util.*;


/** DRMAA application uses the JobTemplate  class, in order to define the attributes associated with a job. 
 * JobTemplates are created via the active {@link Session} implementation. A DRMAA application
 * gets a JobTemplate from the active {@link Session}, specifies in the JobTemplate any required job
 * parameters, and the passes the JobTemplate back to the Session when requesting that a job be
 * executed. When finished, the DRMAA application should call the
 * {@link Session#deleteJobTemplate} method to allow the underlying implementation to free any
 * resources bound to the JobTemplate object. 
 */

public abstract class JobTemplate 
{
	
	/** Pre-defined string to refer to the HOLD state on submission. 
 	 *  Use this preprocessor directive to assign the value of the 
 	 *  drmaa_js_state attribute through the {@link #setJobSubmissionState} method call.
 	 */ 
	public static final String 	HOLD				= "drmaa_hold";
	
	/** Pre-defined string to refer to the ACTIVE state on submission. Use this
 	 *  preprocessor directive to assign the value of the drmaa_js_state attribute
 	 *  through the {@link #setJobSubmissionState} method call.
 	 */
	public static final String 	ACTIVE				= "drmaa_active";
	
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
	
	/** Attribute to refer to the command to be executed on the remote 
 	 *  host. DRMAA_REMOTE_COMMAND MUST BE RELATIVE to the working directory 
 	 *  (DRMAA_WD). Architecture-dependent DRMAA_REMOTE_COMMAND can be generated
 	 *  with DRMAA_GW_ARCH.
 	 */
	protected java.lang.String 					drmaa_remote_command;
	
	/** String array attribute to refer to the DRMAA_REMOTE_COMMAND arguments.  
 	 */
	protected java.lang.String[]					drmaa_v_argv;
	
	/** String attribute to refer to the job state at submission, the job will 
 	 *  enter either the QUEUED_ACTIVE state or HOLD state when submitted. The 
 	 *  preprocessor directives DRMAA_SUBMISSION_STATE_ACTIVE and 
 	 *  DRMAA_SUBMISSION_STATE_HOLD SHOULD be used to assign the value of this attribute. 
 	 *  The default value is ACTIVE.
 	 */
	protected String						drmaa_js_state = ACTIVE;
	
	/** String attribute to refer to the {@link #drmaa_remote_command} environment 
 	 *  variables. 
	 */
	protected java.util.Properties					drmma_v_env;
	
	/** Not relevant for the current GridWay implementation, will be ignored
 	*/
	protected java.lang.String					drmaa_job_category;
	
	/** Not relevant for the current GridWay implementation, will be ignored
 	*/
	protected java.lang.String					drmaa_native_specification;
	
	/** Not relevant for the current GridWay implementation, will be ignored
 	 */
	protected java.lang.String[]					drmaa_v_email;
	
	/** Not relevant for the current GridWay implementation, will be ignored
 	*/
	protected boolean						drmaa_block_email;
	
	/** Not relevant for the current GridWay implementation, will be ignored
 	*/
	protected PartialTimestamp		drmaa_start_time;
	
	/** String attribute to refer to the DRMAA job-name. The current GridWay
 	*  DRMAA implementation will generate a job template file with name 
 	*  drmaa_job_name in the job working directory (DRMAA_WD). drmaa_job_name is
 	*  a MANDATORY attribute value and MUST BE DEFINED. The default value is 
 	*  "job_template".
 	*/
	protected java.lang.String					drmaa_job_name;
	
	/** String attribute to refer to standard input file for the  
 	 *  {@link #drmaa_remote_command}. The standard input file IS RELATIVE TO THE 
 	 *  WORKING DIRECTORY.
 	 */
	protected java.lang.String					drmaa_input_path;
	
	/** String attribute to refer to standard output file for the  
 	 *  {@link #drmaa_remote_command}. The standard ouput file IS RELATIVE TO THE 
 	 *  WORKING DIRECTORY.
 	 */
	protected java.lang.String					drmaa_output_path;
	
	/** String attribute to refer to standard error file for the  
 	 *  DRMAA_REMOTE_COMMAND. The standard error file IS RELATIVE TO THE 
 	 *  WORKING DIRECTORY.
 	 */
	protected java.lang.String					drmaa_error_path;
	
	/** Not relevant for the current GridWay implementation, will be ignored
 	*/
	protected boolean						drmaa_join_files;
	
	/** String attribute to refer to the JOB working directory. The current
 	 *  GridWay DRMAA implementation will generate a job template file with
 	 *  name {@link #drmaa_job_name} in the job working directory (drmaa_wd). It is a
	 *  MANDATORY attribute value and MUST BE DEFINED. Plase note that ALL FILES
         *  ARE NAMED RELATIVE TO THE WORKING DIRECTORY. Also this is a LOCAL PATH NAME, 
         *  this directory will be recreated in the remote host, and it will be the 
         *  working directory of the job on the execution host.
         */ 
	protected java.lang.String					drmaa_wd;

	/** Not relevant for the current GridWay implementation, will be ignored
 	*/
	protected FileTransferMode			drmaa_transfer_files;
	
	/** Not relevant for the current GridWay implementation, will be ignored
 	*/
	protected PartialTimestamp			drmaa_deadline_time;
	
	/** Not relevant for the current GridWay implementation, will be ignored
 	*/
	protected long 							drmaa_wct_hlimit;
	
	/** Not relevant for the current GridWay implementation, will be ignored
 	*/
	protected long 							drmaa_wct_slimit;
	
	/** Not relevant for the current GridWay implementation, will be ignored
 	*/
	protected long 							drmaa_run_duration_hlimit;
	
	/** Not relevant for the current GridWay implementation, will be ignored
 	*/
	protected long 							drmaa_run_duration_slimit;	
		
	/** The JobTemplate() constructor creates a new instance of a JobTemplate. In most cases, a
	 * DRMAA implementation will require that JobTemplates be created through the
	 * {@link Session#createJobTemplate} method, however. In those cases, passing a JobTemplate created
	 * through the JobTemplate() constructor to the {@link Session#deleteJobTemplate}, {@link Session#runJob},
	 * or {@link Session#runBulkJobs} methods will result in an {@link InvalidJobTemplateException} being thrown.
	 */
	public JobTemplate()
	{
		this.drmaa_job_name = new String("job_template");
	}
		
	/** This method set the attribute {@link #drmaa_remote_command}. 
 	 * 
	 *  @param command A command to set
 	 *
	 *  @throws InvalidAttributeValueException
 	 */
	public  void setRemoteCommand(java.lang.String command)
		throws DrmaaException
	{
		if (command == null)
			throw new InvalidAttributeValueException("The command attribute is NULL");	
		else
			this.drmaa_remote_command = new String(command);
	}

	/** This method set the attribute {@link #drmaa_v_argv}. 
 	 * 
	 *  @param args The attributes to set
 	 *
	 *  @throws InvalidAttributeValueException
 	 */
	public  void setArgs(java.lang.String[] args)
		throws DrmaaException
	{
		if (args == null)
			throw new InvalidAttributeValueException("The args attribute is NULL");	
		else
		{
			this.drmaa_v_argv = new String[args.length];
			int	i;
			for (i=0;i<args.length;i++)
				this.drmaa_v_argv[i] = new String(args[i]);
		}
	}
	
	/** This method set the attribute {@link #drmaa_js_state}. 
 	 * 
	 *  @param state The state to set
 	 *
	 *  @throws InvalidAttributeValueException
 	 */
	public  void setJobSubmissionState(String state)
		throws DrmaaException
	{
		this.drmaa_js_state = new String(state);
	}
	
	/** This method set the attribute {@link #drmma_v_env}. 
 	 * 
	 *  @param env The environment to set
 	 *
	 *  @throws InvalidAttributeValueException
 	 */
	public  void setJobEnvironment(java.util.Properties env)
		throws DrmaaException
	{
		if (env == null)
			throw new InvalidAttributeValueException("The env attribute is NULL");	
		else
		{
			this.drmma_v_env = new Properties();
			Enumeration e = env.propertyNames();
			
			while (e.hasMoreElements())
			{
				String name  = (String) e.nextElement();
				String value = env.getProperty(name);
				
			/*	if (value.charAt(0) != '\"')
					value = "\"" + value +  "\"";*/
				
				this.drmma_v_env.setProperty(name, value);
			}
		}
			
	}
	
	/** This method set the attribute {@link #drmaa_wd}. 
 	 * 
	 *  @param wd The working directoy to set
 	 *
	 *  @throws InvalidAttributeValueException
 	 */
	public  void setWorkingDirectory(String wd)
		throws DrmaaException
	{
		if (wd == null)
			throw new InvalidAttributeValueException("The workingDirectory attribute is NULL");	
		else
			this.drmaa_wd = new String(wd);
	}

	/** This method set the attribute {@link #drmaa_job_category}. 
 	 * 
	 *  @param category The category to set
 	 *
	 *  @throws InvalidAttributeValueException
 	 */
	public  void setJobCategory(String category)
		throws DrmaaException
	{
		if (category == null)
			throw new InvalidAttributeValueException("The category attribute is NULL");	
		else
			this.drmaa_job_category = new String(category);
	}
	
	/** This method set the attribute {@link #drmaa_native_specification}. 
 	 * 
	 *  @param spec The native specification to set
 	 *
	 *  @throws InvalidAttributeValueException
 	 */
	public  void setNativeSpecification(String spec)
		throws DrmaaException
	{
		if (spec == null)
			throw new InvalidAttributeValueException("The nativeSpecification attribute is NULL");	
		else
			this.drmaa_native_specification = new String(spec);
	}

	
	/** This method set the attribute {@link #drmaa_v_email}. 
 	 * 
	 *  @param email The email to set
 	 *
	 *  @throws InvalidAttributeValueException
 	 */
	public  void setEmail(String[] email)
		throws DrmaaException
	{
		if (email == null)
			throw new InvalidAttributeValueException("The email attribute is NULL");	
		else
		{
			this.drmaa_v_email = new String[email.length];
			int	i;
			for (i=0;i<email.length;i++)
				this.drmaa_v_email[i] = new String(email[i]);
		}
	}		

	
	/** This method set the attribute {@link #drmaa_block_email}. 
 	 * 
	 *  @param blockEmail The blockEmail to set
 	 *
	 *  @throws InvalidAttributeValueException
 	 */
	public  void setBlockEmail(boolean blockEmail)
		throws DrmaaException
	{
		this.drmaa_block_email = blockEmail;
	}
	
	/** This method set the attribute {@link #drmaa_start_time}. 
 	 * 
	 *  @param startTime The startTime to set
 	 *
	 *  @throws InvalidAttributeValueException
 	 */
	public  void setStartTime(PartialTimestamp startTime)
		throws DrmaaException
	{
		if (startTime == null)
			throw new InvalidAttributeValueException("The startTime attribute is NULL");	
		else
			this.drmaa_start_time = startTime;
	}
	
	/** This method set the attribute {@link #drmaa_job_name}. 
 	 * 
	 *  @param name The Job name to set
 	 *
	 *  @throws InvalidAttributeValueException
 	 */
	public  void setJobName(String name)
		throws DrmaaException
	{
		if (name == null)
			throw new InvalidAttributeValueException("The JobTemplate name attribute is NULL");	
		else
			this.drmaa_job_name = new String(name);	
	}		
	
	/** This method set the attribute {@link #drmaa_input_path}. 
 	 * 
	 *  @param inputPath The input path to set
 	 *
	 *  @throws InvalidAttributeValueException
 	 */
	public  void setInputPath(String inputPath)
		throws DrmaaException
	{
		if (inputPath == null)
			throw new InvalidAttributeValueException("The inputPath attribute is NULL");	
		else
			this.drmaa_input_path = new String(inputPath);		
	}		
	
	/** This method set the attribute {@link #drmaa_output_path}. 
 	 * 
	 *  @param outputPath The output path to set
 	 *
	 *  @throws InvalidAttributeValueException
 	 */
	public  void setOutputPath(String outputPath)
		throws DrmaaException
	{
		if (outputPath == null)
			throw new InvalidAttributeValueException("The outputPath attribute is NULL");	
		else
			this.drmaa_output_path = new String(outputPath);		
	}
	
	/** This method set the attribute {@link #drmaa_error_path}. 
 	 * 
	 *  @param errorPath The error path to set
 	 *
	 *  @throws InvalidAttributeValueException
 	 */
	public  void setErrorPath(String errorPath)
		throws DrmaaException
	{
		if (errorPath == null)
			throw new InvalidAttributeValueException("The errorPath attribute is NULL");	
		else
			this.drmaa_error_path = new String(errorPath);		
	}
	
	/** This method set the attribute {@link #drmaa_join_files}. 
 	 * 
	 *  @param joinFiles The joinFiles to set
 	 *
	 *  @throws InvalidAttributeValueException
 	 */
	public  void setJoinFiles(boolean joinFiles)
		throws DrmaaException
	{
		this.drmaa_join_files = joinFiles;		
	}
	
	/** This method set the attribute {@link #drmaa_transfer_files}. 
 	 * 
	 *  @param mode The transfer mode to set
 	 *
	 *  @throws InvalidAttributeValueException
 	 */
	public  void setTransferFiles(FileTransferMode mode)
		throws DrmaaException
	{
		if (mode == null)
			throw new InvalidAttributeValueException("The fileTransferMode attribute is NULL");	
		else
			this.drmaa_transfer_files = mode;		
	}
	
	/** This method set the attribute {@link #drmaa_deadline_time}. 
 	 * 
	 *  @param deadline The deadline to set
 	 *
	 *  @throws InvalidAttributeValueException
 	 */
	public  void setDeadlineTime(PartialTimestamp deadline)
		throws DrmaaException
	{
		if (deadline == null)
			throw new InvalidAttributeValueException("The deadline attribute is NULL");	
		else
			this.drmaa_deadline_time = deadline;		
	}

	/** This method set the attribute {@link #drmaa_wct_hlimit}. 
 	 * 
	 *  @param limit The hard wall clock time limit to set
 	 *
	 *  @throws InvalidAttributeValueException
 	 */
	public  void setHardWallclockTimeLimit(long limit)
		throws DrmaaException
	{
		this.drmaa_wct_hlimit = limit;		
	}
	
	/** This method set the attribute {@link #drmaa_wct_slimit}. 
 	 * 
	 *  @param limit The soft wall clock time timit to set
 	 *
	 *  @throws InvalidAttributeValueException
 	 */
	public  void setSoftWallclockTimeLimit(long limit)
		throws DrmaaException
	{
		this.drmaa_wct_slimit = limit;		
	}
	
	/** This method set the attribute {@link #drmaa_run_duration_hlimit}. 
 	 * 
	 *  @param limit The hard run duration limit to set
 	 *
	 *  @throws InvalidAttributeValueException
 	 */
	public  void setHardRunDurationLimit(long limit)
		throws DrmaaException
	{
		this.drmaa_run_duration_hlimit = limit;		
	}
	
	/** This method set the attribute {@link #drmaa_run_duration_slimit}. 
 	 * 
	 *  @param limit The soft run duration limit to set
 	 *
	 *  @throws InvalidAttributeValueException
 	 */
	public  void setSoftRunDurationLimit(long limit)
		throws DrmaaException
	{
		this.drmaa_run_duration_slimit = limit;
	}
	
	/** This method get the attribute {@link #drmaa_remote_command}. 
 	 * 
	 *  @return A {@link String} with the drmaa_remote_command value
 	 *
 	 */
	public  String getRemoteCommand()		
	{
		return this.drmaa_remote_command;
	}

	/** This method get the attribute {@link #drmaa_v_argv}. 
 	 * 
	 *  @return A {@link String} array with the drmaa_v_argv value
 	 *
 	 */
	public  java.lang.String[] getArgs()
	{
		return this.drmaa_v_argv;
	}
	
	/** This method get the attribute {@link #drmaa_js_state}. 
 	 * 
	 *  @return A {@link String} with the drmaa_js_state value
 	 *
 	 */
	public  String getJobSubmissionState()
	{
		return this.drmaa_js_state;
	}
	
	/** This method get the attribute {@link #drmma_v_env}. 
 	 * 
	 *  @return A {@link Properties} object with the drmma_v_env value
 	 *
 	 */
	public  java.util.Properties getJobEnvironment()
	{
		return this.drmma_v_env;
	}

	/** This method get the attribute {@link #drmaa_wd}. 
 	 * 
	 *  @return A {@link String} with the drmaa_remote_command value
 	 *
 	 */
	public  String getWorkingDirectory()
	{
		return this.drmaa_wd;		
	}

	/** This method get the attribute {@link #drmaa_job_category}. 
 	 * 
	 *  @return A {@link String} with the drmaa_job_category value
 	 *
 	 */
	public  String getJobCategory()
	{
		return this.drmaa_job_category;		
	}
	
	/** This method get the attribute {@link #drmaa_native_specification}. 
 	 * 
	 *  @return A {@link String} with the drmaa_native_specification value
 	 *
 	 */
	public  String getNativeSpecification()
	{
		return this.drmaa_native_specification;		
	}
	
	/** This method get the attribute {@link #drmaa_v_email}. 
 	 * 
	 *  @return A {@link String} array with the drmaa_v_email value
 	 *
 	 */
	public  String[] getEmail()
	{
		return this.drmaa_v_email;
	}		

	/** This method get the attribute {@link #drmaa_block_email}. 
 	 * 
	 *  @return A boolean with the drmaa_block_email value
 	 *
 	 */
	public  boolean getBlockEmail()
	{
		return this.drmaa_block_email;
	}
	
	/** This method get the attribute {@link #drmaa_start_time}. 
 	 * 
	 *  @return A PartialTimestamp object  with the drmaa_block_email value
 	 *
 	 */
	public  PartialTimestamp getStartTime()
	{
		return this.drmaa_start_time;
	}
	
	/** This method get the attribute {@link #drmaa_job_name}. 
 	 * 
	 *  @return A {@link String} with the drmaa_job_name value
 	 *
 	 */
	public  String getJobName()
	{
		return this.drmaa_job_name;
	}		
	
	/** This method get the attribute {@link #drmaa_input_path}. 
 	 * 
	 *  @return A {@link String} with the drmaa_input_path value
 	 *
 	 */
	public  String getInputPath()
	{
		return this.drmaa_input_path;
	}		
	
	/** This method get the attribute {@link #drmaa_output_path}. 
 	 * 
	 *  @return A {@link String} with the drmaa_output_path value
 	 *
 	 */
	public  String getOutputPath()
	{
		return this.drmaa_output_path;
	}
	
	/** This method get the attribute {@link #drmaa_error_path}. 
 	 * 
	 *  @return A {@link String} with the drmaa_error_path value
 	 *
 	 */
	public  String getErrorPath()
	{
		return this.drmaa_error_path;		
	}
	
	/** This method get the attribute {@link #drmaa_join_files}. 
 	 * 
	 *  @return A boolean with the drmaa_join_files value
 	 *
 	 */
	public  boolean getJoinFiles()
	{
		return this.drmaa_join_files;		
	}
	
	/** This method get the attribute {@link #drmaa_transfer_files}. 
 	 * 
	 *  @return A FileTransferMode object with the drmaa_transfer_files value
 	 *
 	 */
	public  FileTransferMode getTransferFiles()
	{
		return this.drmaa_transfer_files;		
	}
	
	/** This method get the attribute {@link #drmaa_deadline_time}. 
 	 * 
	 *  @return A PartialTimestamp object with the drmaa_deadline_time value
 	 *
 	 */
	public  PartialTimestamp getDeadlineTime()
	{
		return this.drmaa_deadline_time;
	}

	/** This method get the attribute {@link #drmaa_wct_hlimit}. 
 	 * 
	 *  @return A long with the drmaa_wct_hlimit value
 	 *
 	 */
	public  long getHardWallclockTimeLimit()
	{
		return this.drmaa_wct_hlimit;
		
	}
	
	/** This method get the attribute {@link #drmaa_wct_slimit}. 
 	 * 
	 *  @return A long with the drmaa_wct_slimit value
 	 *
 	 */
	public  long getSoftWallclockTimeLimit()
	{
		return this.drmaa_wct_slimit;
	}
	
	/** This method get the attribute {@link #drmaa_run_duration_hlimit}. 
 	 * 
	 *  @return A long with the drmaa_run_duration_hlimit value
 	 *
 	 */
	public  long getHardRunDurationLimit()
	{
		return this.drmaa_run_duration_hlimit;
	}
	
	/** This method get the attribute {@link #drmaa_run_duration_slimit}. 
 	 * 
	 *  @return A long with the drmaa_run_duration_slimit value
 	 *
 	 */
	public  long getSoftRunDurationLimit()
	{
		return this.drmaa_run_duration_slimit;
	}
	
	/** This method return the list of supported property names.
 	 *  The required Gridway property names are:
	 * <ul>
	 * <li> {@link #drmaa_remote_command}, {@link #drmaa_v_argv}, {@link #drmaa_js_state}</li>
	 * <li> {@link #drmma_v_env}, {@link #drmaa_wd}, {@link #drmaa_job_name}, {@link #drmaa_input_path}</li>		
	 * <li> {@link #drmaa_output_path}, {@link #drmaa_error_path}, {@link #drmaa_output_path}</li>		
	 * </ul>
	 * 
	 *  The optional Gridway property names (implemented in {@link JobTemplateImpl}) are:
	 * <ul>
	 * <li> drmaa_v_gw_input_files, drmaa_v_gw_output_files, drmaa_v_gw_restart_files</li>
	 * <li> drmaa_gw_reschedule_on_failure, drmaa_gw_number_of_retries, drmaa_gw_rank</li>
	 * <li>	drmaa_gw_requirements</li>
	 * </ul>
 	 */
	public  abstract java.util.List getAttributeNames();
	
	/** This method set the attribute {@link JobTemplateImpl}.drmaa_v_gw_input_files. 
	 *  This attribute is an array of strings to refer to the input files of {@link #drmaa_remote_command}.  
 	 *  Each vector entry is a pair of the form "source destination" filenames. 
 	 *  If the destination filename is missing, the source filename will be preserved 
 	 *  in the execution host. Input files (sources) ARE RELATIVE TO THE WORKING 
 	 *  DIRECTORY or can be a GSIFTP URL. Example: input_file[0]="param."DRMAA_GW_TASK_ID" param"
 	 *  will copy the local file param.2 (for task 2) as param in the remote working directory.
 	 *
	 *  @param inputFiles The input files to set
 	 *
	 *  @throws InvalidAttributeValueException
 	 */	
	public  abstract void setInputFiles(String[] inputFiles)
		throws DrmaaException;
	
	/** This method set the attribute {@link JobTemplateImpl}.drmaa_v_gw_output_files. 
 	 *  This attribute is an array of strings to refer to the input files of {@link #drmaa_remote_command}.  
 	 *  Each vector entry is a "source destination" filenames pair. If the
 	 *  destination filename is missing, the source filename will be preserved in the
 	 *  client host. Output files can be a GSIFTP URL. Example: 
 	 *  output_file[0]="binary binary."DRMAA_GW_ARCH will copy the output file "binary" to the
 	 *  client host with name binary.i686 (architecture of remote host is i686) 
 	 * 
	 *  @param outputFiles The output files to set
 	 *
	 *  @throws InvalidAttributeValueException
 	 */	
	public  abstract void setOutputFiles(String[] outputFiles)
		throws DrmaaException;
		
	/** This method set the attribute {@link JobTemplateImpl}.drmaa_v_gw_restart_files. 
 	 *  This attribute is an array of strings to refer to the re-start files generated by {@link #drmaa_remote_command}.  
 	 *  Each vector entry is the name of a checkpointing file. Re-start files can be used to preserve the execution
 	 *  context (at the application level) of the DRMAA_REMOTE_COMMAND on job 
 	 *  migration or stop/resume actions.
	 
	 *  @param restartFiles The restart files to set
 	 *
	 *  @throws InvalidAttributeValueException
 	 */	
	public  abstract void setRestartFiles(String[] restartFiles)
		throws DrmaaException;
	
	/** This method set the attribute {@link JobTemplateImpl}.drmaa_gw_reschedule_on_failure. 
	 *  This attribute is a String to refer to the ON_FAILURE GridWay scheduler
 	 *  parameter. If set to "yes" GridWay will reschedule the job  
 	 *  after retrying execution on a given host drmaa_gw_number_of_retries times.
 	 *  Values are "yes" or "no". Default value for this attribute is "no".
 	 *
	 *  @param onFailure The boolean reschedule on failure value to set
 	 *
	 *  @throws InvalidAttributeValueException
 	 */		
	 public  abstract void setRescheduleOnFailure(String onFailure)
		throws DrmaaException;
		
	/** This method set the attribute {@link JobTemplateImpl}.drmaa_gw_number_of_retries. 
	 *	
	 *  This attribute is a String to refer to the NUMBER_OF_RETRIES GridWay scheduler
 	 *  parameter, the number of times to retry the execution on a given host.
 	 *  Default value is 3.
	 *  @param numberOfRetries The number of retries to set
 	 *
	 *  @throws InvalidAttributeValueException
 	 */	
	public  abstract void setNumberOfRetries(String numberOfRetries)
		throws DrmaaException;
		
	/** This method set the attribute {@link JobTemplateImpl}.drmaa_gw_rank. 
 	 *  This attribute is a String to refer to the RANK job template parameter.
 	 *  The RANK is a mathematical expression evaluated for each
 	 *  candidate host (those for which the REQUIREMENTS expression is true).
 	 *  Those candidates with higher ranks are used first to execute your jobs. Example: 
 	 *  "(CPU_MHZ * 2) + FREE_MEM_MB;" (NOTE: Must end with ';')
	 *
	 *  @param rank The rank to set
 	 *
	 *  @throws InvalidAttributeValueException
 	 */	
	public  abstract void setRank(String rank)
		throws DrmaaException;
		
	/** This method set the attribute {@link JobTemplateImpl}.drmaa_gw_requirements. 
 	 *  This attribute is a String to refer to the REQUIREMENTS job template parameter.
 	 *  The REQUIREMENTS is a boolean expression evaluated for each host in the 
 	 *  Grid, if it is true the host will be considered to submit the job. 
 	 *  Example:"ARCH = "i686" & CPU_MHZ > 1000;" (NOTE: Must end with ';')
	 *
	 *  @param requirements The requirements to set
 	 *
	 *  @throws InvalidAttributeValueException
 	 */	
	public  abstract void setRequirements(String requirements)
		throws DrmaaException;

        public  abstract void setPriority(String priority)
                throws DrmaaException;
        public  abstract void setType(String type)
                throws DrmaaException;
        public  abstract void setNp(String np)
                throws DrmaaException;
	
	/** This method get the attribute {@link JobTemplateImpl}.drmaa_v_gw_input_files. 
	 *  This attribute is an array of strings to refer to the input files of {@link #drmaa_remote_command}.  
 	 *  Each vector entry is a pair of the form "source destination" filenames. 
 	 *  If the destination filename is missing, the source filename will be preserved 
 	 *  in the execution host. Input files (sources) ARE RELATIVE TO THE WORKING 
 	 *  DIRECTORY or can be a GSIFTP URL. Example: input_file[0]="param."DRMAA_GW_TASK_ID" param"
 	 *  will copy the local file param.2 (for task 2) as param in the remote working directory.
	 *  @return A String array with the drmaa_v_gw_input_files value
 	 *
 	 */
	public  abstract String[] getInputFiles();
	
	/** This method get the attribute {@link JobTemplateImpl}.drmaa_v_gw_output_files. 
 	 *  This attribute is an array of strings to refer to the output files of {@link #drmaa_remote_command}.  
 	 *  Each vector entry is a "source destination" filenames pair. If the
 	 *  destination filename is missing, the source filename will be preserved in the
 	 *  client host. Output files can be a GSIFTP URL. Example: 
 	 *  output_file[0]="binary binary."DRMAA_GW_ARCH will copy the output file "binary" to the
 	 *  client host with name binary.i686 (architecture of remote host is i686)  	 
 	 *
	 *  @return A String array with the drmaa_v_gw_output_files value
 	 *
 	 */
	public  abstract String[] getOutputFiles();
	
	/** This method get the attribute {@link JobTemplateImpl}.drmaa_v_gw_restart_files. 
 	 *  This attribute is an array of strings to refer to the re-start files generated by {@link #drmaa_remote_command}.  
 	 *  Each vector entry is the name of a checkpointing file. Re-start files can be used to preserve the execution
 	 *  context (at the application level) of the DRMAA_REMOTE_COMMAND on job 
 	 *  migration or stop/resume actions.
	 *
	 *  @return A String array with the drmaa_v_gw_restart_files value
 	 *
 	 */
	public  abstract String[] getRestartFiles();
	
	/** This method get the attribute {@link JobTemplateImpl}.drmaa_gw_reschedule_on_failure. 
 	 *  This attribute is a String to refer to the ON_FAILURE GridWay scheduler
 	 *  parameter. If set to "yes" GridWay will reschedule the job  
 	 *  after retrying execution on a given host drmaa_gw_number_of_retries times.
 	 *  Values are "yes" or "no". Default value for this attribute is "no".
 	 * 
	 *  @return A String array with the drmaa_gw_reschedule_on_failure value
 	 *
 	 */
	public  abstract String getRescheduleOnFailure();
	
	/** This method get the attribute {@link JobTemplateImpl}.drmaa_gw_number_of_retries. 
 	 *  This attribute is a String to refer to the NUMBER_OF_RETRIES GridWay scheduler
 	 *  parameter, the number of times to retry the execution on a given host.
 	 *  Default value is 3. 
	 *
	 *  @return A String array with the drmaa_gw_number_of_retries value
 	 *
 	 */
	public  abstract String getNumberOfRetries();
	
	/** This method get the attribute {@link JobTemplateImpl}.drmaa_gw_rank. 
 	 *  This attribute is a String to refer to the REQUIREMENTS job template parameter.
 	 *  The REQUIREMENTS is a boolean expression evaluated for each host in the 
 	 *  Grid, if it is true the host will be considered to submit the job. 
 	 *  Example:"ARCH = "i686" & CPU_MHZ > 1000;" (NOTE: Must end with ';')
	 *  @return A String array with the drmaa_gw_rank value
 	 *
 	 */
	public  abstract String getRank();

	/** This method get the attribute {@link JobTemplateImpl}.drmaa_gw_requirements. 
 	 *  This attribute is a String to refer to the RANK job template parameter.
 	 *  The RANK is a mathematical expression evaluated for each
 	 *  candidate host (those for which the REQUIREMENTS expression is true).
 	 *  Those candidates with higher ranks are used first to execute your jobs. Example: 
 	 *  "(CPU_MHZ * 2) + FREE_MEM_MB;" (NOTE: Must end with ';')
	 * 
	 *  @return A String array with the drmaa_gw_requirements value
 	 *
 	 */
	public  abstract String getRequirements();

	public  abstract String getPriority();
	public  abstract String getType();
	public  abstract String getNp();
	
	public java.lang.String toString()
	{
		String result;
		
		result = "Name: " + this.drmaa_job_name + "\n";
		return result;
	}
	
	protected abstract java.util.List getOptionalAttributeNames();
}
