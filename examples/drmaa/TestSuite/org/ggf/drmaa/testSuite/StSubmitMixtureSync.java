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

public abstract class StSubmitMixtureSync extends Test 
{
	protected String[]		allJobIds = new String[this.nBulks*this.jobChunk+this.jobChunk];
	
	StSubmitMixtureSync(String executable, String type)
	{
		super(executable, type);
	}
	
	public void runMixture()
	{
		int	pos = 0;
		this.createJob = new CreateSleeperJobTemplate(this.session, this.executable, "5", false);
		
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


		}
		catch (Exception e)
		{
			System.err.println("Test "+ this.type +" failed");
            		e.printStackTrace();
			this.stateAllTest = false;
		}
	}
}
