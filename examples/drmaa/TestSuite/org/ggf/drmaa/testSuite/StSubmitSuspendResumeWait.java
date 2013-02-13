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

public class StSubmitSuspendResumeWait extends Test 
{
	public StSubmitSuspendResumeWait(String executable)
	{
		super(executable, ST_SUBMIT_SUSPEND_RESUME_WAIT);
	}

	public void run()
	{
		int		status;
		
		this.createJob = new CreateSleeperJobTemplate(this.session, this.executable, "5", false);
		
		try
		{
			this.session.init(null);
            		System.out.println("Session Init success");

            		this.jt = this.createJob.getJobTemplate();
			
			this.id = this.session.runJob(this.jt);
			System.out.println("Job successfully submitted ID: " + this.id);
						
			do
			{
				status = this.session.getJobProgramStatus(this.id);
				
				if (status!=Session.DRMAA_PS_RUNNING)
					Thread.sleep(5000);
				
			}while(status != Session.DRMAA_PS_RUNNING);
			
			
			this.session.control(this.id, Session.DRMAA_CONTROL_SUSPEND);
			
			System.out.println ("Suspended job " + this.id);
			
			status = this.session.getJobProgramStatus(this.id);
				
			if (status!=Session.DRMAA_PS_USER_SUSPENDED)
			{
				System.err.println("getJobProgramStatus(" + this.id +") failed returns unexpected job state after the control_suspend call");
				System.err.println("Test "+ this.type +" failed");
				this.stateAllTest = false;
				return;
			}
			 
			
			System.out.println("Verified suspend was done for job " + this.id);
			
			this.session.control(this.id, Session.DRMAA_CONTROL_RESUME);
			
			System.out.println("Resumed job " + this.id);
			
			Thread.sleep(10000);
						
			status = this.session.getJobProgramStatus(this.id);
			
			
			if (status!=Session.DRMAA_PS_RUNNING && status!=Session.DRMAA_PS_FAILED && status!=Session.DRMAA_PS_QUEUED_ACTIVE)
			{
				System.err.println("getJobProgramStatus(" + this.id +") failed returns unexpected job state after the control_resume call" + status);
				this.stateAllTest = false;
				return;
			}
			else if (status==Session.DRMAA_PS_FAILED)
			{
				System.err.println("Warning: Job failed after resuming");
				this.stateAllTest = false;
				return;
			}
			
			System.out.println("Verified resume was done for job " + this.id);
			
			JobInfo info = this.session.wait(this.id, Session.DRMAA_TIMEOUT_WAIT_FOREVER);
			
			if (!info.ifExited() || info.getExitStatus()!=0)
			{
				System.err.println("Test "+ this.type +" failed");
				this.stateAllTest = false;
				return;
			}
			
			System.out.println("Succesfully finished test "+ this.type);
		}
		catch (Exception e)
		{
			System.out.println("Test "+ this.type +" failed");
            		e.printStackTrace();
			this.stateAllTest = false;
		}
		
		try
		{
			this.session.exit();
		}
		catch (DrmaaException e)
		{
			System.out.println("drmaa_exit() failed");
            		e.printStackTrace();	
			this.stateAllTest = false;
		}
	}
}
