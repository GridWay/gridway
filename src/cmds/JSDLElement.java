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

public class JSDLElement 
{
	private String	name;
	
	private String	text;
	
	/*
	 * FUTURE IMPLEMENTATIONS:
	 * private Vector	attributes;
	 */
	
	private String  attributes;
	
	private Vector	children;
	
	private boolean hasAttributes;
	
	private boolean hasChildren;
	
	private boolean hasText;
	
	private boolean hasTarget;
	
	private boolean hasSource;
	
	public JSDLElement()
	{
		this.name=null;
		
		this.text=null;
		
		this.attributes=null;
		
		this.children=null;
		
		this.hasAttributes=false;
		
		this.hasChildren=false;
		
		this.hasText=false;
		
		this.hasTarget=false;
		
		this.hasSource=false;
	}
	
	public void setName(String name)
	{
		this.name = name;
		
	}
	
	public void setText(String text)
	{
		this.text = text;
		
	}
	
	public void setAttributes(String attributes)
	{
		this.attributes = attributes;
	}
	
	public void setChildren(Vector children)
	{
		this.children = children;
	}
	
	public void setHasAttributes(boolean hasAttributes)
	{
		this.hasAttributes = hasAttributes;
	}
	
	public void setHasChildren(boolean hasChildren)
	{
		this.hasChildren = hasChildren;
	}
	
	public void setHasText(boolean hasText)
	{
		this.hasText = hasText;
	}
	
	public void setHasTarget(boolean hasTarget)
	{
		this.hasTarget = hasTarget;
	}
	
	public void setHasSource(boolean hasSource)
	{
		this.hasSource = hasSource;
	}
	
	
	public String getName()
	{
		return this.name;
	}
	
	public String getText()
	{
		return this.text;
	}
	
	public String getAttributes()
	{
		return this.attributes;
	}
	
	public Vector getChildren()
	{
		return this.children;
	}
	
	public boolean getHasAttributes()
	{
		return this.hasAttributes;
	}
	
	public boolean getHasChildren()
	{
		return this.hasChildren;
	}
	
	public boolean getHasText()
	{
		return this.hasText;
	}
	
	public boolean getHasTarget()
	{
		return this.hasTarget;
	}
	
	public boolean getHasSource()
	{
		return this.hasSource;
	}
	public String toString()
	
	{
		return this.getChildren().toString();
	}
}
