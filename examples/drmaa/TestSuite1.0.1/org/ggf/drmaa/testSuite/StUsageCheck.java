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

public class StUsageCheck extends Test 
{
	public StUsageCheck(String executable)
	{
		super(executable, ST_USAGE_CHECK);
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
			
			this.id = this.session.runJob(this.jt);
			System.out.println("Job successfully submitted ID: " + this.id);
						
			 
			JobInfo info = this.session.wait(this.id, Session.TIMEOUT_WAIT_FOREVER);
						
			if (info==null)
			{
				System.err.println("drmaa_wait(" + this.id +") did not return usage information and did not throw NoResourceUsageException");
				this.session.exit();
				this.stateAllTest = false;
				return;
			}
			
            			
			System.out.println("Job usage:");
			Map 		rmap = info.getResourceUsage();
			Iterator 	r = rmap.keySet().iterator();
				
			while(r.hasNext())
			{
				String name = (String) r.next();
				String value = (String) rmap.get(name);
				System.out.println(" " + name + "=" + value);
			}
						
			session.deleteJobTemplate(jt);
			
			System.out.println("Succesfully finished test "+ this.type);

		}
		catch (DrmaaException e)
		{
			System.err.println("drmaa_wait(" + this.id +") did not return usage information");
			System.err.println("Test "+ this.type +" failed");
            		e.printStackTrace();
			this.stateAllTest = false;
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
			System.out.println("drmaa_exit() failed");
            		e.printStackTrace();	
			this.stateAllTest = false;
		}
	}
}
