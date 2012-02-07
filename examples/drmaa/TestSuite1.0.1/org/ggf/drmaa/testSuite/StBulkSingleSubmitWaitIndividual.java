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

public class StBulkSingleSubmitWaitIndividual extends Test 
{
	public StBulkSingleSubmitWaitIndividual(String executable)
	{
		super(executable, ST_BULK_SINGLESUBMIT_WAIT_INDIVIDUAL);
	}

	public void run()
	{
		int 				n = 20;
		int				pos = 0;
		String[]			allJobIds;
		
			
		this.createJob = new CreateSleeperJobTemplate(this.session, this.executable, "5", false);
		
		allJobIds = new String[this.nBulks*this.jobChunk+this.jobChunk];
	
		try
		{
			this.session.init(null);
            		System.out.println("Session Init success");
		
            		this.jt = this.createJob.getJobTemplate();
			
			for (int i=0;i<this.nBulks;i++)
			{
				java.util.List 		ids = this.session.runBulkJobs(this.jt, 1, this.jobChunk, 1);
				java.util.Iterator	iter = ids.iterator();
			
				System.out.println("Bulk job successfully submitted IDs are: ");
			
				while(iter.hasNext())
				{
					allJobIds[pos++]=(String)(iter.next());
					System.out.println("\t" + allJobIds[pos-1]);
					
				}
			}
			
			for (int i=0;i<this.jobChunk;i++)
			{
				this.id = this.session.runJob(this.jt);
				System.out.println("Job successfully submitted ID: " + this.id);
				allJobIds[pos++]=this.id;
			}
			
			this.session.deleteJobTemplate(this.jt); 
			
			for (int i=0;i<this.jobChunk*this.nBulks;i++)
			{
				this.session.wait(allJobIds[i], Session.TIMEOUT_WAIT_FOREVER);
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
