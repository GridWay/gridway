/* -------------------------------------------------------------------------- */
/* Copyright 2002-2007 GridWay Team, Distributed Systems Architecture         														*/
/* Group, Universidad Complutense de Madrid                                   																	*/
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

import java.util.*;

public class JSDL 
{
	// Main TAG - First Level
	
	public static String	jobDefinition									= "jsdl:JobDefinition";
	
	public static String	jobDescription								= "jsdl:JobDescription";
	
	// JobDescription TAGS - Second Level
		
	public static String	jobIdentificationJD							= "jsdl:JobIdentification";
	
	public static String	applicationJD									= "jsdl:Application";
	
	public static String	resourcesJD									= "jsdl:Resources";
	
	public static String	dataStagingJD								= "jsdl:DataStaging";
	
	
	// JobIdentification TAGS - Third Level
	
	public static String 	jobNameJIJD									= "jsdl:JobName";
	
	public static String 	descriptionJIJD								= "jsdl:Description";
	
	public static String 	jobAnnotationJIJD							= "jsdl:JobAnnotation";
	
	public static String 	jobProjectJIJD									= "jsdl:JobProject";
	

	// Application TAGS - Third Level
	
	public static String 	applicationNameAJD						= "jsdl:ApplicationName";
	
	public static String 	applicationVersionAJD						= "jsdl:ApplicationVersion";
	
	public static String 	descriptionAJD								= "jsdl:Description";
	
	public static String 	applicationPOSIXAJD						= "jsdl-posix:POSIXApplication";
	
	public static String	applicationHPCPAJD						= "jsdl-hpcpa:HPCProfileApplication";
	
	// Resources TAGS - Third Level
	
	public static String 	candidateHostsRJD							= "jsdl:CandidateHost";
	
	public static String 	fileSystemRJD									= "jsdl:FileSystem";
	
	public static String 	exclusiveExecutionRJD					= "jsdl:ExclusiveExecution";
	
	public static String 	operatingSystemRJD						= "jsdl:OperatingSystem";

	public static String 	cpuArchitectureRJD							= "jsdl:CPUArchitecture";
	
	public static String 	individualCPUSpeedRJD					= "jsdl:IndividualCPUSpeed";
	
	public static String 	individualCPUTimeRJD						= "jsdl:IndividualCPUTime";
	
	public static String 	individualCPUCountRJD					= "jsdl:IndividualCPUCount";
	
	public static String 	individualNetworkBandwidthRJD		= "jsdl:IndividualNetworkBandwidth";
	
	public static String 	individualPhysicalMemoryRJD			= "jsdl:IndividualPhysicalMemory";
	
	public static String 	individualVirtualMemoryRJD				= "jsdl:IndividualVirtualMemory";

	public static String 	individualDiskSpaceRJD					= "jsdl:IndividualDiskSpace";

	public static String 	totalCPUTimeRJD							= "jsdl:TotalCPUTime";
	
	public static String 	totalCPUCountRJD							= "jsdl:TotalCPUCount";
	
	public static String 	totalPhysicalMemoryRJD					= "jsdl:TotalPhysicalMemory";
	
	public static String 	totalVirtualMemoryRJD					= "jsdl:TotalVirtualMemory";

	public static String 	totalDiskSpaceRJD							= "jsdl:TotalDiskSpace";
	
	public static String 	totalResourceCountRJD					= "jsdl:TotalResourceCount";
	
	// DataStaging TAGS - Third Level
	
	public static String 	fileNameDSJD									= "jsdl:FileName";

	public static String 	fileSystemNameDSJD						= "jsdl:FileSystemName";

	public static String 	creationFlagDSJD							= "jsdl:CreationFlag";
	
	public static String 	deleteOnTerminationDSJD				= "jsdl:DeleteOnTermination";
	
	public static String 	sourceDSJD									= "jsdl:Source";
	
	public static String 	targetDSJD										= "jsdl:Target";

	// POSIXApplication TAGS - Fourth Level
	
	public static String 	executablePAAJD							= "jsdl-posix:Executable";
	
	public static String 	argumentPAAJD								= "jsdl-posix:Argument";
	
	public static String 	inputPAAJD									= "jsdl-posix:Input";
	
	public static String 	outputPAAJD									= "jsdl-posix:Output";
	
	public static String 	errorPAAJD										= "jsdl-posix:Error";
	
	public static String 	workingDirectoryPAAJD					= "jsdl-posix:WorkingDirectory";
	
	public static String 	environmentPAAJD							= "jsdl-posix:Environment";
															
	public static String 	wallTimeLimitPAAJD						= "jsdl-posix:WallTimeLimit";
	
	public static String 	fileSizeLimitPAAJD							= "jsdl-posix:FileSizeLimit";
	
	public static String 	coreDumpLimitPAAJD						= "jsdl-posix:coreDumpLimit";
	
	public static String 	dataSegmentLimitPAAJD					= "jsdl-posix:DataSegmentLimit";
	
	public static String 	lockedMemoryLimitPAAJD				= "jsdl-posix:LockedMemoryLimit";
	
	public static String 	memoryLimitPAAJD							= "jsdl-posix:MemoryLimit";
	
	public static String 	openDescriptorsLimitPAAJD				= "jsdl-posix:OpenDescriptorsLimit";
	
	public static String 	pipeSizeLimitPAAJD							= "jsdl-posix:PipeSizeLimit";
	
	public static String 	stackSizeLimitPAAJD						= "jsdl-posix:StackSizeLimit";
	
	public static String 	cpuTimeLimitPAAJD						= "jsdl-posix:CPUTimeLimit";
	
	public static String 	processCountLimitPAAJD					= "jsdl-posix:ProcessCountLimit";
	
	public static String 	virtualMemoryLimitPAAJD				= "jsdl-posix:VirtualMemoryLimit";
	
	public static String 	threadCountLimitPAAJD					= "jsdl-posix:ThreadCountLimit";
	
	public static String 	userNamePAAJD								= "jsdl-posix:UserName";
	
	public static String 	groupNamePAAJD							= "jsdl-posix:GroupName";
	
	// HPCProfileApplication TAGS - Fourth Level
	
	public static String 	executableHPCPAJD						= "jsdl-hpcpa:Executable";
	
	public static String 	argumentHPCPAJD							= "jsdl-hpcpa:Argument";
	
	public static String 	inputHPCPAJD								= "jsdl-hpcpa:Input";
	
	public static String 	outputHPCPAJD								= "jsdl-hpcpa:Output";
	
	public static String 	errorHPCPAJD									= "jsdl-hpcpa:Error";
	
	public static String 	workingDirectoryHPCPAJD				= "jsdl-hpcpa:WorkingDirectory";
	
	public static String 	environmentHPCPAJD						= "jsdl-hpcpa:Environment";
															
	public static String 	userNameHPCPAJD							= "jsdl-hpcpa:UserName";
	

	// CandidateHosts TAGS - Fourth Level
	
	public static String 	hostNameCHRJD								= "jsdl:HostName";
	
	// FileSystem TAGS - Fourth Level
	
	public static String 	descriptionFSRJD							= "jsdl:Description";
	
	public static String 	mountPointFSRJD							= "jsdl:MountPoint";
	
	public static String 	mountSourceFSRJD							= "jsdl:MountSource";
	
	public static String 	diskSpaceFSRJD								= "jsdl:DiskSpace";
	
	public static String 	fileSystemTypeFSRJD						= "jsdl:FileSystemType";
	
	// OperatingSystem TAGS - Fourth Level
	
	public static String 	operatingSystemTypeOSRJD				= "jsdl:OperatingSystemType";
		
	public static String 	operatingSystemVersionOSRJD			= "jsdl:OperatingSystemVersion";
	
	public static String 	descriptionOSRJD							= "jsdl:Description";
	
	// CPUArchitecture TAGS - Fourth Level
	
	public static String 	cpuArchitectureNameCARJD			= "jsdl:CPUArchitectureName";
	
	// OperatingSystemType TAGS - Fifth Level	
	
	public static String 	operatingSystemNameOSTOSRJD		= "jsdl:OperatingSystemName";
	
	// Other TAGS - NonDefined Level
	
	public static String	lowerBoundedRange						= "jsdl:LowerBoundedRange";
	
	public static String	upperBoundedRange						= "jsdl:UpperBoundedRange";
	public static String	upperBound									= "jsdl:UpperBound";
	public static String	range											= "jsdl:Range";
	
	public static String	exact												= "jsdl:Exact";
	
	//	Range TAGS 
	public static String	lowerBoundR									= "jsdl:LowerBound";
	
	public static String	upperBoundR									= "jsdl:UpperBound";
	
	// Attributes TAGS
	
	public static String	nameEnvironment							= "name";
	
	// Special TAGS
	public static String	noInputOutputFiles							= "NO_INPUT_OUTPUT_FILES";
	
	// Private attributes
	
	private String			nameJSDLFile;
	
	private JSDLReader	jsdlReader;
	
	private Vector			jsdlElements;
	
	// Methods
	
	public JSDL(String nameJSDLFile) throws Exception
	{
		this.nameJSDLFile = nameJSDLFile;
		this.jsdlReader = new JSDLReader(this.nameJSDLFile);	
		this.jsdlElements = this.jsdlReader.getJSDLElements();
	}
	
	public Vector getJSDLElements()
	{
		return this.jsdlElements;
	}
	
	
}
