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
/* -------------------------------------------------------------------------- */

package org.ggf.drmaa.testSuite;
import org.ggf.drmaa.*;

public class StSubmitKillSig extends Test 
{
	private static final String[] signals	={"SIGUSR1", "SIGTERM", "SIGALARM", "SIGUSR2",
						"SIGSEGV", "SIGHUP", "SIGQUIT", "SIGILL",
						"SIGABRT", "SIGFPE", "SIGKILL"};
	
	public StSubmitKillSig(String executable)
	{
		super(executable, ST_SUBMIT_KILL_SIG);
	}
	
	public void run()
	{
		try
		{
			this.session.init(null);
			System.out.println("Session Init success");
			for (int i=0; i<signals.length;i++)
			{
				this.createJob = new CreateKillerJobTemplate(this.session, this.executable, this.signals[i]);
				this.jt = this.createJob.getJobTemplate();
				this.id = this.session.runJob(this.jt);
				System.out.println("Job successfully submitted ID: " + this.id);
				JobInfo info = this.session.wait(this.id, Session.DRMAA_TIMEOUT_WAIT_FOREVER);
				
				if (!info.ifSignaled() || !info.getTerminatingSignal().equals(this.signals[i]))
				{
					System.err.println("Test "+ this.type +" failed");
					this.stateAllTest = false;
					return;
				}
				
				System.out.println("Job " + this.id + "killed with" + this.signals[i]);
				
			}
			
			System.out.println("Succesfully finished test "+ this.type);
		}
		catch (Exception e)
		{
			System.err.println("Test "+ this.type +" failed");
            		e.printStackTrace();
			this.stateAllTest = false;
		}
	}
}
