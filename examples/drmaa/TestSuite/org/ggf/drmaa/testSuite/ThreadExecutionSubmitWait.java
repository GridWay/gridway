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
/* -------------------------------------------------------------------------- */

package org.ggf.drmaa.testSuite;
import org.ggf.drmaa.*;
import java.util.*;

public class ThreadExecutionSubmitWait extends ThreadExecution 
{
	private Vector<String>		idVector = new Vector<String>();
	
	public ThreadExecutionSubmitWait(Session session, String executable, int nThreads, String type, int nJobs)
	{
		super(session, executable, nThreads, type, nJobs);
	}
	
	public void run() 
	{
		try
		{
			JobTemplate jt = this.session.createJobTemplate();
			
			String	    cwd;		
			jt.setWorkingDirectory(java.lang.System.getProperty("user.dir"));
			cwd = jt.getWorkingDirectory();
			System.out.println("The working directory is: " + cwd);
			
			String	    name;		
			jt.setJobName(this.name);
			name = jt.getJobName();
			System.out.println("The jobTemplate name is: " + name);
						
			jt.setRemoteCommand(this.executable);
			jt.setArgs(this.args);
			
			jt.setOutputPath("stdout." + SessionImpl.DRMAA_GW_JOB_ID);
			jt.setErrorPath("stderr." + SessionImpl.DRMAA_GW_JOB_ID);
			
			for (int i=0;i<this.nJobs;i++)
			{
				idVector.add(session.runJob(jt));
				System.out.println("Job successfully submitted ID: " + idVector.get(i));
			}
			
			while(!idVector.isEmpty())
			{
				JobInfo info = this.session.wait(Session.DRMAA_JOB_IDS_SESSION_ANY, Session.DRMAA_TIMEOUT_WAIT_FOREVER);
				idVector.remove(info.getJobId());
			}
		}
		catch (Exception e)
		{
			this.setException(e);
		}
	}
}
