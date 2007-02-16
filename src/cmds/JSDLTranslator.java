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

import java.util.*;

/***************************************/
/* 
 * Class Translator JSDL -> GWJT
 * =============================
 * 
 * JSDL Elements translated:
 * 
 * 	Application:
 * 		PosixApplication:
 * 			Executable
 * 			Argument
 * 			Input
 * 			Output
 * 			Error
 * 			Environment
 * 			
 * 	Resources:
 * 		CandidateHost:
 * 			HostName
 * 		OperatingSystem:
 * 			OperatingSystemType:
 * 				OperatingSystemName
 * 			OperatingSystemVersion
 * 		CPUArchitecture:
 * 			CPUArchitectureName
 * 		IndividualCPUSpeed			<---> JSDLRange
 * 		IndividualCPUCount			<---> JSDLRange
 * 		IndividualPhysicalMemory	<---> JSDLRange
 * 		IndividualDiskSpace			<---> JSDLRange
 * 
 *	DataStaging:
 *		FileName
 *		Source						<---> URI
 *		Target						<---> URI
 * 
 */

public class JSDLTranslator 
{
	private	JSDL		jsdl;
	private	GWJT		gwjt;
	private Vector		jsdlElements;
	private	Vector		gwjtElements;
		
	public JSDLTranslator(String jsdlFileName)
	{
		this.jsdl = new JSDL(jsdlFileName);
		this.gwjt = new GWJT(null);
		this.jsdlElements = this.jsdl.getJSDLElements();
		this.gwjtElements = new Vector();
		this.createGWJTElements();	
	}
	
	
	public JSDLTranslator(String jsdlFileName, String gwjtFileName)
	{
		this.jsdl = new JSDL(jsdlFileName);
		this.gwjt = new GWJT(gwjtFileName);
		this.jsdlElements = this.jsdl.getJSDLElements();
		this.gwjtElements = new Vector();
		this.createGWJTElements();	
	}
	
	public void doTranslation()
	{
		this.gwjt.generateGWJTFile(this.gwjtElements);
	}
	
	
	private void createGWJTElements()
	{
		JSDLElement noInputOutputElement=null;
		GWJTElement	inputElement;
		GWJTElement outputElement;
		String		inputFiles = new String("");
		String		outputFiles = new String("");
		
		for (int i = 0; i < this.jsdlElements.size(); i++)
		{
			JSDLElement jsdlElement = (JSDLElement) this.jsdlElements.get(i);	
			
			if (jsdlElement.getName().equals(JSDL.applicationJD))
				createGWJTApplicationElements(jsdlElement);
			else if (jsdlElement.getName().equals(JSDL.resourcesJD))
				createGWJTResourcesElements(jsdlElement);
			else if (jsdlElement.getName().equals(JSDL.dataStagingJD))
			{
				if (jsdlElement.getHasSource())
					inputFiles+= createGWJTDataStagingElements(jsdlElement, noInputOutputElement);
				else if (jsdlElement.getHasTarget())
					outputFiles+= createGWJTDataStagingElements(jsdlElement, noInputOutputElement);
				
			}	
			else if (jsdlElement.getName().equals(JSDL.noInputOutputFiles))
				noInputOutputElement = jsdlElement;
		}	
		
		if (!inputFiles.equals(""))
		{
			inputElement = new GWJTElement(GWJT.inputFiles, inputFiles.substring(0,inputFiles.length()-2));
			this.gwjtElements.add(inputElement);
		}	
		
		if (!outputFiles.equals(""))
		{
			outputElement = new GWJTElement(GWJT.outputFiles, outputFiles.substring(0,outputFiles.length()-2));
			this.gwjtElements.add(outputElement);
		}
	}
	
	private void createGWJTApplicationElements(JSDLElement jsdlElement)
	{
		Vector		children = jsdlElement.getChildren();
		String		environmentString = new String("");
		
		for (int i=0; i < children.size(); i++)
		{
			JSDLElement	childElement = (JSDLElement) children.get(i);
			GWJTElement	gwjtElement = new GWJTElement();
			
			if (childElement.getName().equals(JSDL.environmentPAAJD))
			{
				environmentString+= childElement.getAttributes() + "="  + childElement.getText() + ", ";
			}
			else
			{
				gwjtElement.setName(translationTAG(childElement.getName()));
				gwjtElement.setValue(childElement.getText());
				this.gwjtElements.add(gwjtElement);
			}		
		}
		
		if (!environmentString.equals("="))
		{
			GWJTElement	gwjtElement = new GWJTElement();
			gwjtElement.setName(translationTAG(JSDL.environmentPAAJD));
			gwjtElement.setValue(environmentString.substring(0,environmentString.length()-2));
			this.gwjtElements.add(gwjtElement);
		}
	}
	
	private void createGWJTResourcesElements(JSDLElement jsdlElement)
	{
		
		GWJTElement	gwjtElement = new GWJTElement();
		Vector		children 	= jsdlElement.getChildren();
		String		values		= new String("");
		
		gwjtElement.setName(translationTAG(jsdlElement.getName()));
	
		for (int i=0; i < children.size(); i++)
		{
			JSDLElement	childElement = (JSDLElement) children.get(i);
			values+=createGWJTRequirementsValues(childElement);
			
			if (i < children.size()-1)
				values+=" & ";
		}
		
		gwjtElement.setValue(values);
		this.gwjtElements.add(gwjtElement);
	}
	
	private String createGWJTDataStagingElements(JSDLElement jsdlElement, JSDLElement noInputOutputElement)
	{
		Vector		children 		= jsdlElement.getChildren();		
		Vector		noChildren 		= null;
		String		filesString		= new String("");
		
		if (noInputOutputElement!=null)
			noChildren = noInputOutputElement.getChildren();
		
		for (int i=0; i < children.size(); i++)
		{
			JSDLElement	childElement = (JSDLElement) children.get(i);
					
			if (childElement.getName().equals(JSDL.fileNameDSJD))
			{	
				boolean 	insertElement = true;
				
				if (noChildren!=null)
				{
					int 	j=0;
					while(j<noChildren.size() && insertElement)
					{
						JSDLElement noChildElement = (JSDLElement) noChildren.get(j);
						insertElement = !noChildElement.getText().equals(childElement.getText());
						j++;
					}
				}
				
				if (insertElement)
								
					filesString = childElement.getText() + ", ";
			}	
		}
		
		return filesString;
	}
	
	private String createGWJTRequirementsValues(JSDLElement jsdlElement)
	{
		String 	values = new String("");
		
		if (jsdlElement.getHasChildren())
		{
			if (jsdlElement.getName().equals(JSDL.candidateHostsRJD) || jsdlElement.getName().equals(JSDL.cpuArchitectureRJD))
			{	
				Vector	children = jsdlElement.getChildren();
			
				for (int i=0; i < children.size(); i++)
				{
					JSDLElement	childElement = (JSDLElement) children.get(i);
					values+=translationTAG(childElement.getName())+ "=\"" + childElement.getText() +"\"";
					
					if (i < children.size()-1)
						values+=" & ";
				}
			}
			else if (jsdlElement.getName().equals(JSDL.operatingSystemRJD))
					values+=createGWJTOperatingSystemValues(jsdlElement);
			else if (jsdlElement.getName().equals(JSDL.individualCPUSpeedRJD) ||
					 jsdlElement.getName().equals(JSDL.individualCPUCountRJD) ||
					 jsdlElement.getName().equals(JSDL.individualPhysicalMemoryRJD) ||
					 jsdlElement.getName().equals(JSDL.individualDiskSpaceRJD))
				values+=createGWJTIndividualValues(jsdlElement);
		} 
		return values;
	}
	
	private String createGWJTOperatingSystemValues(JSDLElement jsdlElement)
	{
		String 	values = new String("");
	
		if (jsdlElement.getHasChildren())
		{
			Vector	children = jsdlElement.getChildren();
			
			for (int i=0; i < children.size(); i++)
			{
				JSDLElement	childElement = (JSDLElement) children.get(i);
				
				if (childElement.getName().equals(JSDL.operatingSystemTypeOSRJD) && childElement.getHasChildren())
				{
					Vector	grandChildren = childElement.getChildren();
				
					JSDLElement	grandChildElement = (JSDLElement) grandChildren.get(0);
				
					if (grandChildElement.getText().equals("other") && grandChildren.size()>1 )
						grandChildElement = (JSDLElement) grandChildren.get(1);
					
					values+= "OS_NAME" + "=\"" + grandChildElement.getText() +"\"";
						
				}
				else if (childElement.getName().equals(JSDL.operatingSystemVersionOSRJD)) 	
					values+=translationTAG(childElement.getName())+ "=\"" + childElement.getText() +"\"";
				
				if (i < children.size()-1)
					values+=" & ";
			}
		}
		
		return values;
	}
	
	private String createGWJTIndividualValues(JSDLElement jsdlElement)
	{
		String 	values = new String(translationTAG(jsdlElement.getName()));
	
		if (jsdlElement.getHasChildren())
		{
			Vector	children = jsdlElement.getChildren();
			
			for (int i=0; i < children.size(); i++)
			{
				JSDLElement	childElement = (JSDLElement) children.get(i);
				
				if (!jsdlElement.getName().equals(JSDL.individualCPUCountRJD))
				{
					Float 	data	= new Float(childElement.getText());
					float 	dataValue = data.floatValue() / 1000000;
					values+= translationRange(childElement.getName()) + String.valueOf(dataValue);
				}
				else
					values+= translationRange(childElement.getName()) + childElement.getText();
				
				 
				
				if (i < children.size()-1)
					values+=" & ";
			}
		}
		
		return values;
	}
	
		
	private String translationTAG(String jsdlTAG)
	{
		if (jsdlTAG.equals(JSDL.executablePAAJD))
			return GWJT.executable;
		else if (jsdlTAG.equals(JSDL.argumentPAAJD))
			return GWJT.arguments;
		else if (jsdlTAG.equals(JSDL.inputPAAJD))
			return GWJT.stdinFile;
		else if (jsdlTAG.equals(JSDL.outputPAAJD))
			return GWJT.stdoutFile;
		else if (jsdlTAG.equals(JSDL.errorPAAJD))
			return GWJT.stderrFile;
		else if (jsdlTAG.equals(JSDL.environmentPAAJD))
			return GWJT.environment;
		else if (jsdlTAG.equals(JSDL.resourcesJD))
			return GWJT.requirements;
		else if (jsdlTAG.equals(JSDL.hostNameCHRJD))
			return GWJT.varHostname;
		else if (jsdlTAG.equals(JSDL.cpuArchitectureNameCARJD))
			return GWJT.varArch;
		else if (jsdlTAG.equals(JSDL.operatingSystemVersionOSRJD))
			return GWJT.varOsVersion;
		else if (jsdlTAG.equals(JSDL.individualCPUSpeedRJD))
			return GWJT.varCpuMhz;
		else if (jsdlTAG.equals(JSDL.individualCPUCountRJD))
			return GWJT.varNodeCount;
		else if (jsdlTAG.equals(JSDL.individualPhysicalMemoryRJD))
			return GWJT.varSizeMemMB;
		else if (jsdlTAG.equals(JSDL.individualDiskSpaceRJD))
			return GWJT.varSizeDiskMB;
		else	
			return null;
	}
	
	private String translationRange(String jsdlTAG)
	{
		if (jsdlTAG.equals(JSDL.lowerBoundedRange))
			return ">";
		else if (jsdlTAG.equals(JSDL.upperBoundedRange))
			return "<";
		else if (jsdlTAG.equals(JSDL.exact))
			return "=";
		else
			return null;
	}

}