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


import java.util.*;
import java.io.*;
import javax.xml.parsers.*;
import org.w3c.dom.*;
 

public class JSDLReader 
{
	private String								jsdlFileName;
	
	private File									jsdlFile;
	
	private DocumentBuilderFactory		jsdlBuilderFactory;
	
	private DocumentBuilder				jsdlBuilder;
	
	private Document							jsdlDocument;
	
	private Element								jsdlRoot;
	
	private JSDLElement						noInputOutputElement;		
	
	
	public JSDLReader(String jsdlFileName) throws Exception
	{
		this.jsdlFileName = jsdlFileName;
			
		// Open the JSDL file
		this.jsdlFile = new File(this.jsdlFileName);
				
		// Create a DocumentBuilderFactory
		this.jsdlBuilderFactory = DocumentBuilderFactory.newInstance();
			
		// Use a validating parser
		this.jsdlBuilderFactory.setValidating(true);
			
		// Create a document builder
		this.jsdlBuilder = this.jsdlBuilderFactory.newDocumentBuilder();
		this.jsdlBuilder.setErrorHandler(null);
		    
		// Create a document 
		this.jsdlDocument = this.jsdlBuilder.parse(this.jsdlFile);
		    
		// Create the tree root
			
		this.jsdlRoot = this.jsdlDocument.getDocumentElement() ;
		    
		this.noInputOutputElement = new JSDLElement();
		this.noInputOutputElement.setName(JSDL.noInputOutputFiles);
	}	
	
	public Vector getJSDLElements()
	{
		Vector				jsdlElements = new Vector();
		JSDLElement		element = null;		
		Node					actual;
		NodeList 			root = this.jsdlRoot.getElementsByTagName(JSDL.jobDescription);
		NodeList			children = root.item(0).getChildNodes();
		
		for (int i=0; i < children.getLength(); i++)
		{
			actual = children.item(i);
			if (!actual.getNodeName().equals("#text") && !actual.getNodeName().equals("#comment")) 
			{
				if (actual.getNodeName().equals(JSDL.jobIdentificationJD))
					element = writeJobIdentificationElement(actual);
				else if (actual.getNodeName().equals(JSDL.applicationJD))
					element = writeApplicationElement(actual);					
				else if (actual.getNodeName().equals(JSDL.resourcesJD))
					element = writeResourcesElement(actual);
				else if (actual.getNodeName().equals(JSDL.dataStagingJD))
					element = writeDataStagingElement(actual);
				
				if (element != null)
					jsdlElements.add(element);
			}
		}
		
		if (this.noInputOutputElement.getHasChildren())
			jsdlElements.add(0,this.noInputOutputElement);
			
		return jsdlElements;
	}

	private JSDLElement writeJobIdentificationElement(Node root)
	{
		Node		actual;
		NodeList	childrenNodes = root.getChildNodes();
		JSDLElement	element = new JSDLElement();
		
		element.setName(root.getNodeName());
		element.setHasAttributes(false);
		element.setHasText(false);
		element.setHasChildren(root.hasChildNodes());
								
		Vector			childrenElements = new Vector();
		
		for (int i=0; i<childrenNodes.getLength(); i++)
		{
			actual = childrenNodes.item(i);
			
			if (!actual.getNodeName().equals("#text") && !actual.getNodeName().equals("#comment") && actual.getNodeName().equals(JSDL.jobNameJIJD)) 
			{
				JSDLElement 	childElement = new JSDLElement();
				
				childElement.setName(actual.getNodeName());
				childElement.setText(actual.getTextContent());
				childElement.setHasText(true);
				childElement.setHasChildren(false);				
				childrenElements.add(childElement);
			}
		}
		
		element.setChildren(childrenElements);
		return element;
	}
	
	private JSDLElement writeApplicationElement(Node root)
	{
		Node		actual;
		NodeList	childrenNodes = root.getChildNodes();
		JSDLElement	element = new JSDLElement();
		
		element.setName(root.getNodeName());
		element.setHasAttributes(false);
		element.setHasText(false);
		element.setHasChildren(root.hasChildNodes());
								
		for (int i=0; i<childrenNodes.getLength(); i++)
		{
			actual = childrenNodes.item(i);
			
			if (!actual.getNodeName().equals("#text") && !actual.getNodeName().equals("#comment")) 
			{
				Vector			childrenElements = null;
				
				if (actual.getNodeName().equals(JSDL.applicationPOSIXAJD))
						childrenElements = writePOSIXApplicationElements(actual);
				else if (actual.getNodeName().equals(JSDL.applicationHPCPAJD))
						childrenElements = writeHPCPApplicationElements(actual);
						
				element.setHasChildren(true);
				element.setChildren(childrenElements);
			}
			
		}
		
		return element;
	}
	
	private JSDLElement writeResourcesElement(Node root)
	{
		Node		actual;
		NodeList	childrenNodes = root.getChildNodes();
		JSDLElement	element = new JSDLElement();
		JSDLElement childrenElement = null;
		Vector		childrenElements = new Vector(); 
		
		element.setName(root.getNodeName());
		element.setHasAttributes(root.hasAttributes());
		element.setHasText(false);
		element.setHasChildren(root.hasChildNodes());
										
		for (int i=0; i<childrenNodes.getLength(); i++)
		{
			actual = childrenNodes.item(i);
			
			if (!actual.getNodeName().equals("#text") && !actual.getNodeName().equals("#comment")) 
			{
				if (actual.getNodeName().equals(JSDL.candidateHostsRJD))
				{
					childrenElement = writeFinalElements(actual);					
				}
				else if (actual.getNodeName().equals(JSDL.operatingSystemRJD))
				{
					childrenElement = writeOperatingSystem(actual);	
				}
				else if (actual.getNodeName().equals(JSDL.cpuArchitectureRJD))
				{
					childrenElement = writeFinalElements(actual);					
				}
				else if (actual.getNodeName().equals(JSDL.individualCPUSpeedRJD))
				{
					childrenElement = writeFinalElements(actual);
					
				}
				else if (actual.getNodeName().equals(JSDL.individualCPUCountRJD))
				{
					childrenElement = writeFinalElements(actual);
				}
				else if (actual.getNodeName().equals(JSDL.individualPhysicalMemoryRJD))
				{
					childrenElement = writeFinalElements(actual);
				}
				else if (actual.getNodeName().equals(JSDL.individualDiskSpaceRJD))
				{
					childrenElement = writeFinalElements(actual);
				}	
				
				if (childrenElement != null)
					childrenElements.add(childrenElement);
			}
		}
		
		element.setChildren(childrenElements);
		return element;
	}
	
	private Vector writePOSIXApplicationElements( Node actual)
	{
		Vector			childrenElements = new Vector();
			
		Vector			noInputOutputElements = new Vector();
		
		NodeList		posixNodes = actual.getChildNodes();
		
		for(int j=0; j<posixNodes.getLength(); j++) 
		{
			if (!posixNodes.item(j).getNodeName().equals("#text") && !posixNodes.item(j).getNodeName().equals("#comment"))
			{
				
				JSDLElement 	childElement = new JSDLElement();
				
				if (posixNodes.item(j).hasAttributes())
				{
					NamedNodeMap attrs			 	= posixNodes.item(j).getAttributes();
					Node 				nameNode 	= attrs.getNamedItem(JSDL.nameEnvironment);
					
					childElement.setAttributes(nameNode.getTextContent());	
				}
				
				childElement.setName(posixNodes.item(j).getNodeName());
				childElement.setText(posixNodes.item(j).getTextContent());
				childElement.setHasText(true);
				childElement.setHasChildren(false);
				
				childrenElements.add(childElement);
				
				if (posixNodes.item(j).getNodeName().equals(JSDL.inputPAAJD) ||
					posixNodes.item(j).getNodeName().equals(JSDL.outputPAAJD) ||
					posixNodes.item(j).getNodeName().equals(JSDL.errorPAAJD))
					
				{
					noInputOutputElements.add(childElement);
					this.noInputOutputElement.setHasChildren(true);
				}
			}
		}
		
		if (this.noInputOutputElement.getHasChildren())
			this.noInputOutputElement.setChildren(noInputOutputElements);
		
		return childrenElements;
	}
	
	
	private Vector writeHPCPApplicationElements( Node actual)
	{
		Vector			childrenElements = new Vector();
			
		Vector			noInputOutputElements = new Vector();
		
		NodeList		hpcpNodes = actual.getChildNodes();
		
		for(int j=0; j<hpcpNodes.getLength(); j++) 
		{
			if (!hpcpNodes.item(j).getNodeName().equals("#text") && !hpcpNodes.item(j).getNodeName().equals("#comment"))
			{
				
				JSDLElement 	childElement = new JSDLElement();
				
				if (hpcpNodes.item(j).hasAttributes())
				{
					NamedNodeMap attrs			 	= hpcpNodes.item(j).getAttributes();
					Node 				nameNode 	= attrs.getNamedItem(JSDL.nameEnvironment);
					
					childElement.setAttributes(nameNode.getTextContent());	
				}
				
				childElement.setName(hpcpNodes.item(j).getNodeName());
				childElement.setText(hpcpNodes.item(j).getTextContent());
				childElement.setHasText(true);
				childElement.setHasChildren(false);
				
				childrenElements.add(childElement);
				
				if (hpcpNodes.item(j).getNodeName().equals(JSDL.inputHPCPAJD) ||
					hpcpNodes.item(j).getNodeName().equals(JSDL.outputHPCPAJD) ||
					hpcpNodes.item(j).getNodeName().equals(JSDL.errorHPCPAJD))
					
				{
					noInputOutputElements.add(childElement);
					this.noInputOutputElement.setHasChildren(true);
				}
			}
		}
		
		if (this.noInputOutputElement.getHasChildren())
			this.noInputOutputElement.setChildren(noInputOutputElements);
		
		return childrenElements;
	}
	
	private JSDLElement writeDataStagingElement(Node root)
	{
		Node		actual;
		NodeList	childrenNodes = root.getChildNodes();
		JSDLElement	element = new JSDLElement();
		JSDLElement childrenElement = null;
		Vector		childrenElements = new Vector(); 
		
		element.setName(root.getNodeName());
		element.setHasAttributes(root.hasAttributes());
		element.setHasText(false);
		element.setHasChildren(root.hasChildNodes());
										
		for (int i=0; i<childrenNodes.getLength(); i++)
		{
			actual = childrenNodes.item(i);
			
			if (!actual.getNodeName().equals("#text") && !actual.getNodeName().equals("#comment")) 
			{
				if (actual.getNodeName().equals(JSDL.fileNameDSJD ))
				{
					JSDLElement 	childElement = new JSDLElement();
					childElement.setName(actual.getNodeName());
					childElement.setText(actual.getTextContent());
					childElement.setHasText(true);
					childElement.setHasChildren(false);
					
					childrenElements.add(childElement);			
				}
				else if (actual.getNodeName().equals(JSDL.sourceDSJD))
				{
					childrenElement = writeFinalElements(actual);
					element.setHasSource(true);
					childrenElements.add(childrenElement);
					
				}
				else if (actual.getNodeName().equals(JSDL.targetDSJD))
				{
					childrenElement = writeFinalElements(actual);
					element.setHasTarget(true);
					childrenElements.add(childrenElement);
				}	
			}
		}
		
		element.setChildren(childrenElements);
		return element;
	}
	
	private JSDLElement writeFinalElements(Node root)
	{
		Node		actual;
		NodeList	childrenNodes = root.getChildNodes();
		JSDLElement	element = new JSDLElement();
		Vector		childrenElements = new Vector(); 
		
		element.setName(root.getNodeName());
		element.setHasAttributes(root.hasAttributes());
		element.setHasText(false);
		element.setHasChildren(root.hasChildNodes());
								
		for (int i=0; i<childrenNodes.getLength(); i++)
		{
			actual = childrenNodes.item(i);
			
			if (!actual.getNodeName().equals("#text") && !actual.getNodeName().equals("#comment"))
			{
				JSDLElement		childElement = new JSDLElement();
				
				childElement.setName(actual.getNodeName());
				childElement.setText(actual.getTextContent());
				childElement.setHasText(true);
				childElement.setHasChildren(false);
				
				childrenElements.add(childElement);
			}
		}	
		
		element.setChildren(childrenElements);
		return element;
	}

	private JSDLElement writeOperatingSystem(Node root)
	{
		Node		actual;
		NodeList	childrenNodes = root.getChildNodes();
		JSDLElement	element = new JSDLElement();
		JSDLElement childrenElement = null;
		Vector		childrenElements = new Vector(); 
		
		element.setName(root.getNodeName());
		element.setHasAttributes(root.hasAttributes());
		element.setHasText(false);
		element.setHasChildren(root.hasChildNodes());
								
		for (int i=0; i<childrenNodes.getLength(); i++)
		{
			actual = childrenNodes.item(i);
			
			if (!actual.getNodeName().equals("#text") && !actual.getNodeName().equals("#comment"))
				
				if (actual.getNodeName().equals(JSDL.operatingSystemTypeOSRJD ))
				{
					childrenElement = writeFinalElements(actual);
					childrenElements.add(childrenElement);
				}
				else if (actual.getNodeName().equals(JSDL.operatingSystemVersionOSRJD ))
				{
					JSDLElement		childElement = new JSDLElement();
					
					childElement.setName(actual.getNodeName());
					childElement.setText(actual.getTextContent());
					childElement.setHasText(true);
					childElement.setHasChildren(false);
					
					childrenElements.add(childElement);	
				}
		}	
		
		element.setChildren(childrenElements);
		return element;
	}

}
