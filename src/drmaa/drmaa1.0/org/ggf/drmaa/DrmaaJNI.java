/* -------------------------------------------------------------------------- */
/* Copyright 2002-2007 GridWay Team, Distributed Systems Architecture         */
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
/* -------------------------------------------------------------------------  */

package org.ggf.drmaa;

/** This class is the native implementation of DRMAA GridWay and it <b><i>SHOULD NEVER</i></b> used by the normal user.
 */
 
public class DrmaaJNI 
{
	/**The normal user of this API <b><i>SHOULD NEVER</i></b> use this method.
	  */
	public DrmaaJNI()
	{
		
	};
	
	/**The normal user of this API <b><i>SHOULD NEVER</i></b> use this method.
	  */
	public native void init(String contactString);
	
	/**The normal user of this API <b><i>SHOULD NEVER</i></b> use this method.
	  */
	public native void exit();

	/**The normal user of this API <b><i>SHOULD NEVER</i></b> use this method.
	  */
	public native JobTemplate createJobTemplate();
	
	/**The normal user of this API <b><i>SHOULD NEVER</i></b> use this method.
	  */
	public native void deleteJobTemplate(JobTemplate jt);
	
	/**The normal user of this API <b><i>SHOULD NEVER</i></b> use this method.
	  */
	public native String runJob(GridWayJobTemplate jt);
	
	/**The normal user of this API <b><i>SHOULD NEVER</i></b> use this method.
	  */
	public native java.util.List runBulkJobs(GridWayJobTemplate jt, int beginIndex, int endIndex, int step);
	
	/**The normal user of this API <b><i>SHOULD NEVER</i></b> use this method.
	  */
	public native void control(String jobName, int operation);
	
	/**The normal user of this API <b><i>SHOULD NEVER</i></b> use this method.
	  */
	public native void synchronize(java.util.List jobList, long timeout, boolean dispose);
	
	/**The normal user of this API <b><i>SHOULD NEVER</i></b> use this method.
	  */
	public native JobInfo wait(String jobName, long timeout);
	
	/**The normal user of this API <b><i>SHOULD NEVER</i></b> use this method.
	  */
	public native int getJobProgramStatus(String jobName);
	
	/**The normal user of this API <b><i>SHOULD NEVER</i></b> use this method.
	  */
	public native String getContact();
	
	/**The normal user of this API <b><i>SHOULD NEVER</i></b> use this method.
	  */
	public native Version getVersion();
	
	/**The normal user of this API <b><i>SHOULD NEVER</i></b> use this method.
	  */
	public native String getDrmsInfo();
	
	/**The normal user of this API <b><i>SHOULD NEVER</i></b> use this method.
	  */
	public native String getDrmaaImplementation();
	
	static
	{
		System.loadLibrary("DrmaaJNI");
	}
}
