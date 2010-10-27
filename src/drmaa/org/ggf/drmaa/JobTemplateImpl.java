/* -------------------------------------------------------------------------- */
/* Copyright 2002-2010, GridWay Project Leads (GridWay.org)                   */
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

/** This class implements the GridWay specific methods. These methods are abstract methods in the {@link JobTemplate} class.
 */

public class JobTemplateImpl extends JobTemplate
{

	private long  		jobTemplatePointer;
	private java.util.List 	attrNames;
	private java.util.List 	optAttrNames;
	
	private String[]	drmaa_v_gw_input_files;
	
	private String[]	drmaa_v_gw_output_files;
	
	private String[]	drmaa_v_gw_restart_files;
		
	private String		drmaa_gw_reschedule_on_failure; 
	
	private String		drmaa_gw_number_of_retries;
	
	private String 		drmaa_gw_rank;

	private String 		drmaa_gw_requirements;

	private String 		drmaa_gw_priority;
	
	private String		drmaa_gw_type = SessionImpl.DRMAA_GW_TYPE_SINGLE;
	
	private String 	    drmaa_gw_np;
	
				
	public JobTemplateImpl() 
	{		
		super();
		attrNames = new java.util.ArrayList();
		attrNames.add("drmaa_remote_command");
		attrNames.add("drmaa_v_argv");
		attrNames.add("drmaa_js_state");
		attrNames.add("drmma_v_env");
		attrNames.add("drmaa_wd");
		attrNames.add("drmaa_job_name");		
		attrNames.add("drmaa_input_path");		
		attrNames.add("drmaa_output_path");		
		attrNames.add("drmaa_error_path");						
		attrNames.add("drmaa_output_path");		
		
		
		optAttrNames = new java.util.ArrayList();		
		optAttrNames.add("drmaa_v_gw_input_files");
		optAttrNames.add("drmaa_v_gw_output_files");
		optAttrNames.add("drmaa_v_gw_restart_files");
		optAttrNames.add("drmaa_gw_reschedule_on_failure");
		optAttrNames.add("drmaa_gw_number_of_retries");
		optAttrNames.add("drmaa_gw_rank");
		optAttrNames.add("drmaa_gw_requirements");
		optAttrNames.add("drmaa_gw_priority");
		optAttrNames.add("drmaa_gw_type");
		optAttrNames.add("drmaa_gw_np");	
				
		attrNames.addAll(optAttrNames);
	}
	
	public JobTemplateImpl(String name) 
	{		
		this();
		try
		{	
			
			this.setJobName(name);
		}
		catch (DrmaaException e)
		{
			System.out.println("Error:   " + e.getMessage());
		}
	}
	
	public  void setJobTemplatePointer(long ptr)
	{
		this.jobTemplatePointer = ptr;
	}
	
	
	public  void setInputFiles(String[] inputFiles)
		throws DrmaaException
	{
		
		if (inputFiles == null)
			throw new InvalidAttributeValueException("The inputFiles are NULL");
		else
		{
			this.drmaa_v_gw_input_files = new String[inputFiles.length];
			int	i;
			for (i=0;i<inputFiles.length;i++)
				this.drmaa_v_gw_input_files[i] = new String(inputFiles[i]);
		}
	}
	
	public  void setOutputFiles(String[] outputFiles)
		throws DrmaaException
	{
		if (outputFiles == null)
			throw new InvalidAttributeValueException("The outputFiles are NULL");
		else
		{
			this.drmaa_v_gw_output_files = new String[outputFiles.length];
			int	i;
			for (i=0;i<outputFiles.length;i++)
				this.drmaa_v_gw_output_files[i] = new String(outputFiles[i]);	
		}
	}
	
	public  void setRestartFiles(String[] restartFiles)
		throws DrmaaException
	{
		if (restartFiles == null)
			throw new InvalidAttributeValueException("The restartFiles are NULL");
		else
		{
			this.drmaa_v_gw_restart_files = new String[restartFiles.length];
			int	i;
			for (i=0;i<restartFiles.length;i++)
				this.drmaa_v_gw_restart_files[i] = new String(restartFiles[i]);
		}
	}
	
	public  void setRescheduleOnFailure(String onFailure)
		throws DrmaaException
	{
		if (onFailure == null)
			throw new InvalidAttributeValueException("The onFailure attribute is NULL");	
		else
			this.drmaa_gw_reschedule_on_failure = new String(onFailure);
	}
	
	public  void setNumberOfRetries(String numberOfRetries)
		throws DrmaaException
	{
		if (numberOfRetries == null)
			throw new InvalidAttributeValueException("The numberOfRetries attribute is NULL");	
		else
			this.drmaa_gw_number_of_retries = new String(numberOfRetries);	
	}

	
	public  void setRank(String rank)
		throws DrmaaException
	{
		if (rank == null)
			throw new InvalidAttributeValueException("The rank attribute is NULL");	
		else
			this.drmaa_gw_rank = new String(rank);	
	}

	public  void setRequirements(String requirements)
		throws DrmaaException
	{
		if (requirements == null)
			throw new InvalidAttributeValueException("The requirements attribute is NULL");	
		else
			this.drmaa_gw_requirements = new String(requirements);	
	}
	
        public  void setPriority(String priority)
                throws DrmaaException
        {
                if (priority == null)
                        throw new InvalidAttributeValueException("The priority attribute is NULL");
                else
                        this.drmaa_gw_priority = new String(priority);
        }

	public  void setType(String type)
		throws DrmaaException
	{
		if (type == null)
			throw new InvalidAttributeValueException("The type attribute is NULL");	
		else
			this.drmaa_gw_type = new String(type);	
	}
	
	public  void setNp(String np)
		throws DrmaaException
	{
		if (np == null)
			throw new InvalidAttributeValueException("The np attribute is NULL");	
		else
			this.drmaa_gw_np = new String(np);	
	}

	
	public  long getJobTemplatePointer()
	{
		return this.jobTemplatePointer;
	}
	
	public java.util.List getAttributeNames()
	{					
		return this.attrNames;
	}
	
	public java.util.List getOptionalAttributeNames()
	{						
		return this.optAttrNames;
	}
	
	public  String[] getInputFiles()
	{
		return this.drmaa_v_gw_input_files;
	}
	
	public  String[] getOutputFiles()
	{
		return this.drmaa_v_gw_output_files;
	}
	
	public  String[] getRestartFiles()
	{
		return this.drmaa_v_gw_restart_files;
	}
		
	
	public  String getRescheduleOnFailure()
	{
		return this.drmaa_gw_reschedule_on_failure;	
	}
	
	public  String getNumberOfRetries()
	{
		return this.drmaa_gw_number_of_retries;	
	}
	
	
	public  String getRank()
	{
		return this.drmaa_gw_rank;	
	}

	public  String getRequirements()
	{
		return this.drmaa_gw_requirements;	
	}
	
        public  String  getPriority()
        {
                return  this.drmaa_gw_priority;
        }

	public  String  getType()
	{
		return	this.drmaa_gw_type;	
	}
	
	public  String getNp()
	{
		return 	this.drmaa_gw_np;	
	}	
}
