/* -------------------------------------------------------------------------- */
/* Copyright 2002-2013, GridWay Project Leads (GridWay.org)                   */
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

/** The Version class is a holding class for the major and minor version numbers of the GridWay
 *  DRMAA implementation.
 */
 
public  class Version implements java.io.Serializable, java.lang.Cloneable
{
	private int 		major;
	private int		minor;
	
	/** Constructor that initialize the major and minor values
	 */
	public Version(int major, int minor)
	{
	   this.major = major;
	   this.minor = minor;
	}
	
	/** This method gets the major versions of the DRMAA JAVA
 	 *  binding specification implemented by the DRMAA implementation. 
	 * Current implementation*  is 1.0
 	 *
 	 *  @return  The major value, currently 1
	 * 
	 */
	public int getMajor()
	{
	  return this.major;
	}
	
	/** This method gets the minor versions of the DRMAA JAVA
 	 *  binding specification implemented by the DRMAA implementation. 
	 * Current implementation*  is 1.0
 	 *
 	 *  @return  The major value, current 0
	 * 
	 */
	public int getMinor()
	{
	  return this.minor; 	
	}
	
	public java.lang.Object clone()
	{
	  return (new Version(this.major, this.minor));
	}
	
	public java.lang.String toString()
	{
	  return new String(this.major + "." + this.minor);
	}
	
	public boolean equals(java.lang.Object obj)
	{
	  return (((Version)obj).getMajor() == this.major) && (((Version)obj).getMinor() == this.minor);
	}
	
	/** This method return the hash code of the DRMAA JAVA
 	 *  binding specification implemented by the DRMAA implementation. 
	 *
 	 *  @return  The hash code of the current implementation
	 * 
	 */
	public int hashCode()
	{
	  return major+minor;
	}
}
