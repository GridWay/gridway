/* -------------------------------------------------------------------------- */
/* Copyright 2002-2011, GridWay Project Leads (GridWay.org)                   */
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

public class StBulkSubmitWait extends Test 
{
	public StBulkSubmitWait(String executable)
	{
		super(executable, ST_BULK_SUBMIT_WAIT);
	}

	public void run()
	{
		int 				n = 20;
		
		this.createJob = new CreateSleeperJobTemplate(this.session, this.executable, "5", false);
		

		try
		{			
			this.session.init(null);
            		System.out.println("Session Init success");
		
            		this.jt = this.createJob.getJobTemplate();
			
			for (int i=0;i<this.nBulks;i++)
			{
				java.util.List 		ids = this.session.runBulkJobs(this.jt, 1, jobChunk, 1);
				java.util.Iterator	iter = ids.iterator();
			
				System.out.println("Bulk job successfully submitted IDs are: ");
			
				while(iter.hasNext())
				{
					System.out.println("\t" + iter.next());
				}
			
			}
			this.session.deleteJobTemplate(this.jt); 
			for (int i=0;i<this.jobChunk*this.nBulks;i++)
			{
				JobInfo info = this.session.wait(Session.DRMAA_JOB_IDS_SESSION_ANY, Session.DRMAA_TIMEOUT_WAIT_FOREVER);
			}
				
			System.out.println("Succesfully finished test "+ this.type);
			
		}
		catch (Exception e)
		{
			System.err.println("Test " + this.type +" failed");
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
