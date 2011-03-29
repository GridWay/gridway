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
import java.util.*;

public abstract class StBulkSubmitInHold extends Test 
{
	protected int 	status;
	protected List	ids;

	StBulkSubmitInHold(String executable, String type)
	{
		super(executable, type);
	}	
	
	public void runHold()
	{
		this.createJob = new CreateSleeperJobTemplate(this.session, this.executable, "5", true);
		
		try
		{
		
			this.session.init(null);
            		System.out.println("Session Init success");
		
            		this.jt = this.createJob.getJobTemplate();
			this.ids = this.session.runBulkJobs(this.jt, 1, this.jobChunk, 1);
			
				
			java.util.Iterator	iter = this.ids.iterator();
			
			System.out.println("Bulk job successfully submitted IDs are: ");
			
			while(iter.hasNext())
			{
				String id = (String) iter.next();
				System.out.println("\t" + id);
				this.status = this.session.getJobProgramStatus(id);
				if (this.status!=Session.DRMAA_PS_USER_ON_HOLD)
				{
					System.err.println("Job " + id + " is not in user hold state");
					System.err.println("Test " + this.type + " failed");
					this.stateAllTest = false;
					return;
				}
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
