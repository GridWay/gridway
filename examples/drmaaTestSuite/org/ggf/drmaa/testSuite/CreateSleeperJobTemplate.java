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

package org.ggf.drmaa.testSuite;
import org.ggf.drmaa.*;

public class CreateSleeperJobTemplate extends CreateJobTemplate
{

	private	boolean	hold;
	private String	seconds;
	
	CreateSleeperJobTemplate()
	{
		super();
	}	

	
	CreateSleeperJobTemplate(Session session, String executable, String seconds, boolean hold)
	{
		super(session, executable);
		
		this.seconds = seconds;
		this.hold = hold;
		
		
	} 
	
	public JobTemplate getJobTemplate() throws DrmaaException
	{
		this.jt = this.session.createJobTemplate();
			
		String	    cwd;		
		this.jt.setWorkingDirectory(java.lang.System.getProperty("user.dir"));
		cwd = this.jt.getWorkingDirectory();
		System.out.println("The working directory is: " + cwd);
		
		String	    name;		
		this.jt.setJobName("sleeper");
		name = this.jt.getJobName();
								
		this.jt.setRemoteCommand(this.executable);
		this.jt.setArgs(new String[] {seconds});
			
		this.jt.setOutputPath("stdout." + SessionImpl.DRMAA_GW_JOB_ID);
		this.jt.setErrorPath("stderr." + SessionImpl.DRMAA_GW_JOB_ID);
			
		if (this.hold)
			jt.setJobSubmissionState(JobTemplate.HOLD);
				
		return this.jt;				
	}
	
	
}
