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
/* -------------------------------------------------------------------------  */



package org.ggf.drmaa;

/**  Not relevant for the current GridWay implementation, will be ignored.
 */
public class FileTransferMode implements java.io.Serializable, java.lang.Cloneable
{
	boolean		inputStream;
	boolean		outputStream;
	boolean		errorStream;

	public FileTransferMode() 
	{
	
		
	}
	
	
	public FileTransferMode(boolean inputStream, boolean outputStream, boolean errorStream) 
	{	
		this.inputStream 	= inputStream;
		this.outputStream	= outputStream;
		this.errorStream	= errorStream;
	}
	
	public void setInputStream(boolean inputStream)
	{
		this.inputStream	= inputStream;
	}
	public void setOutputStream(boolean outputStream)
	{
		this.outputStream	= outputStream;
	}
	
	public void setErrorStream(boolean errorStream)
	{
		this.errorStream	= errorStream;
	}
	
	public boolean getInputStream()
	{
		return this.inputStream;
	}
	public boolean getOutputStream()
	{
		return this.outputStream;
	}
	
	public boolean getErrorStream()
	{
		return this.errorStream;
	}

	public java.lang.Object clone()
	{
	 	FileTransferMode	clone = new FileTransferMode(this.inputStream, this.outputStream, this.errorStream);
		return clone;
	}
	
	public boolean equals(java.lang.Object obj)
	{
		FileTransferMode obj1= (FileTransferMode) obj;
	 	return  (obj1.getInputStream() == this.inputStream && 
			 obj1.getOutputStream() == this.outputStream &&
			 obj1.getErrorStream() == this.errorStream);
	}
	
}
