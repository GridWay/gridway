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

public class StExitStatus extends Test 
{
	private String[] allJobs = new String[255];
	
	public StExitStatus(String executable)
	{
		super(executable, ST_EXIT_STATUS);
	}
	
	public void run()
	{
		try
		{
			this.session.init(null);
			System.out.println("Session Init success");
			for (int i=0; i<255;i++)
			{
				this.createJob = new CreateExitJobTemplate(this.session, this.executable,i);
				this.jt = this.createJob.getJobTemplate();
				allJobs[i] = this.session.runJob(this.jt);
				System.out.println("Job successfully submitted ID: " + this.id);
			
			}	
			
			for (int i=0; i<255;i++)
			{
				JobInfo info = this.session.wait(allJobs[i], Session.TIMEOUT_WAIT_FOREVER);
				
				if (info.getExitStatus()!=i)
				{
					System.err.println("Test "+ this.type +" failed");
					this.stateAllTest = false;
					return;
				}
				
				System.out.println("Job " + this.id + "finished with " + info.getExitStatus() + " status");
				
			}
			
			System.out.println("Succesfully finished test "+ this.type);
		}
		catch (Exception e)
		{
			System.err.println("Test "+ this.type +" failed");
            		e.printStackTrace();
			this.stateAllTest = false;
		}
		
		try
		{
			this.session.exit();
		}
		catch (DrmaaException e)
		{
			System.err.println("drmaa_exit() failed");
            		e.printStackTrace();	
			this.stateAllTest = false;
		}
	}
}
