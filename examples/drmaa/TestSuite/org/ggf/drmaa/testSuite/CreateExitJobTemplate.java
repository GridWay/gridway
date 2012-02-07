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
/* -------------------------------------------------------------------------- */

package org.ggf.drmaa.testSuite;
import org.ggf.drmaa.*;

public class CreateExitJobTemplate extends CreateJobTemplate
{

	private	String 	index;
		
	public CreateExitJobTemplate()
	{
		super();
	}	

	
	public CreateExitJobTemplate(Session session, String executable, int index)
	{
		super(session, executable);
		
		this.index = String.valueOf(index);
	} 
	
	public JobTemplate getJobTemplate() throws DrmaaException
	{
		this.jt = this.session.createJobTemplate();
			
		String	    cwd;		
		this.jt.setWorkingDirectory(java.lang.System.getProperty("user.dir"));
		cwd = this.jt.getWorkingDirectory();
		System.out.println("The working directory is: " + cwd);
		
		String	    name;		
		this.jt.setJobName("exit");
		name = this.jt.getJobName();
								
		this.jt.setRemoteCommand(this.executable);
		this.jt.setArgs(new String[] {this.index});
			
		this.jt.setOutputPath("stdout." + SessionImpl.DRMAA_GW_JOB_ID);
		this.jt.setErrorPath("stderr." + SessionImpl.DRMAA_GW_JOB_ID);
			
				
		return this.jt;
						
	}
}
