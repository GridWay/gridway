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
/* -------------------------------------------------------------------------  */



package org.ggf.drmaa;

/**  Not relevant for the current GridWay implementation, will be ignored.
 */
public class FileTransferMode implements java.io.Serializable, java.lang.Cloneable
{
	boolean		transferInputStream;
	boolean		transferOutputStream;
	boolean		transferErrorStream;

	public FileTransferMode() 
	{
	
		
	}
	
	
	public FileTransferMode(boolean transferInputStream, boolean transferOutputStream, 
										boolean transferErrorStream) 
	{	
		this.transferInputStream 	= transferInputStream;
		this.transferOutputStream	= transferOutputStream;
		this.transferErrorStream		= transferErrorStream;
	}
	
	public void setTransferInputStream(boolean transferInputStream)
	{
		this.transferInputStream	= transferInputStream;
	}
	public void setTransferOutputStream(boolean transferOutputStream)
	{
		this.transferOutputStream	= transferOutputStream;
	}
	
	public void setTransferErrorStream(boolean transferErrorStream)
	{
		this.transferErrorStream	= transferErrorStream;
	}
	
	public boolean getTransferInputStream()
	{
		return this.transferInputStream;
	}
	public boolean getTransferOutputStream()
	{
		return this.transferOutputStream;
	}
	
	public boolean getTransferErrorStream()
	{
		return this.transferErrorStream;
	}

	public java.lang.Object clone()
	{
	 	FileTransferMode	clone = new FileTransferMode(this.transferInputStream, this.transferOutputStream, this.transferErrorStream);
		return clone;
	}
	
	public boolean equals(java.lang.Object obj)
	{
		FileTransferMode obj1= (FileTransferMode) obj;
	 	return  (obj1.getTransferInputStream() == this.transferInputStream && 
			 obj1.getTransferOutputStream() == this.transferOutputStream &&
			 obj1.getTransferErrorStream() == this.transferErrorStream);
	}
	
}
