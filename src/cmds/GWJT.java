/* -------------------------------------------------------------------------- */
/* Copyright 2002-2011, GridWay Project Leads (GridWay.org)                   														*/
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


import java.util.Vector;

public class GWJT 
{
	//	 Job Template Options
	public static String name	    						= "NAME";
	
	public static String executable    					= "EXECUTABLE";
	
	public static String arguments     					= "ARGUMENTS";
	
	public static String environment   					= "ENVIRONMENT";
	
	public static String inputFiles    						= "INPUT_FILES";
	
	public static String outputFiles   					= "OUTPUT_FILES";
	
	public static String stdinFile     						= "STDIN_FILE";
	
	public static String stdoutFile    						= "STDOUT_FILE";
	
	public static String stderrFile    						= "STDERR_FILE";
	
	public static String restartFiles  						= "RESTART_FILES";
	
	public static String checkPointInterval				= "CHECKPOINT_INTERVAL";
	
	public static String checkPointURL 					= "CHECKPOINT_URL";
	
	public static String requirements  					= "REQUIREMENTS";
	
	public static String reschedulingInterval			= "RESCHEDULING_INTERVAL";
	
	public static String reschedulingThreshold 		= "RESCHEDULING_THRESHOLD";
	
	public static String suspensionTimeout			= "SUSPENSION_TIMEOUT";
	
	public static String cpuLoadThreshold				= "CPULOAD_THRESHOLD";
	
	public static String monitor							= "MONITOR";
	
	public static String rescheduleOnFailure			= "RESCHEDULE_ON_FAILURE";
	
	public static String numberOfRetries				= "NUMBER_OF_RETRIES";
	
	public static String wrapper							= "WRAPPER";
	
	public static String preWrapper						= "PRE_WRAPPER";
	
	public static String preWrapperArguments		= "PRE_WRAPPER_ARGUMENTS";
	
	// Requirements and Rank Variables
	
	public static String varHostname					= "HOSTNAME";
	
	public static String varArch							= "ARCH";
	
	public static String varOsName						= "OS_NAME";
	
	public static String varOsVersion					= "OS_VERSION";
	
	public static String varCpuModel					= "CPU_MODEL";
	
	public static String varCpuMhz						= "CPU_MHZ";
	
	public static String varCpuFree						= "CPU_FREE";
	
	public static String varCpuSMP 						= "CPU_SMP";
	
	public static String varNodeCount 					= "NODECOUNT";
	
	public static String varSizeMemMB 				= "SIZE_MEM_MB";
	
	public static String varFreeMemMB 				= "FREE_MEM_MB";
	
	public static String varSizeDiskMB 					= "SIZE_DISK_MB";
	
	public static String varFreeDiskMB 					= "FREE_DISK_MB";
	
	public static String varLrmsName 					= "LRMS_NAME";
	
	public static String varLrmsType 					= "LRMS_TYPE";
	
	public static String varQueueName 				= "QUEUE_NAME";
	
	public static String varQueueNodeCount			= "QUEUE_NODECOUNT";
	
	public static String varQueueFreeNodeCount	= "QUEUE_FREENODECOUNT";
	
	public static String varQueueMaxTime			= "QUEUE_MAXTIME";
	
	public static String varQueueMaxCpuTime		= "QUEUE_MAXCPUTIME";
	
	public static String varQueueMaxCount			= "QUEUE_MAXCOUNT";
	
	public static String varQueueMaxRunningJobs	= "QUEUE_MAXRUNNINGJOBS";
	
	public static String varQueueMaxJobsInQueue	= "QUEUE_MAXJOBSINQUEUE";
	
	public static String varQueueDispatchType		= "QUEUE_DISPATCHTYPE";
	
	public static String varQueuePriority				= "QUEUE_PRIORITY";
	
	public static String varQueueStatus 				= "QUEUE_STATUS";
	

	// Private attributes
	
	private String				nameGWJTFile;
	
	private GWJTWriter		gwjtWriter;
	
	private Vector				gwjtElements;
	
	// Constructor
	
	public GWJT(String nameGWJTFile)
	{
		this.nameGWJTFile = nameGWJTFile;	
	}
	
	public void generateGWJTFile(Vector gwtElements)
	{
		this.gwjtElements = gwtElements;
		this.gwjtWriter = new GWJTWriter(this.nameGWJTFile, this.gwjtElements);
		this.gwjtWriter.generateFile();
	}
}
