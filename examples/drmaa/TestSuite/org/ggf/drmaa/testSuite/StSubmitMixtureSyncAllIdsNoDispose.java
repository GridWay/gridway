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
import java.util.*;

public class StSubmitMixtureSyncAllIdsNoDispose extends StSubmitMixtureSync
{
	public StSubmitMixtureSyncAllIdsNoDispose(String executable)
	{
		super(executable, ST_SUBMITMIXTURE_SYNC_ALLIDS_NODISPOSE);
	}


	public void run()
	{
		try
		{
			ArrayList	allJobIdsList = new ArrayList();				
			
			this.runMixture();
			
			for (int i=0;i<this.allJobIds.length;i++)
				allJobIdsList.add(this.allJobIds[i]);
			
			this.session.synchronize(allJobIdsList, Session.DRMAA_TIMEOUT_WAIT_FOREVER, false);
			
			for (int count = 0; count < this.nBulks*this.jobChunk+this.jobChunk;count ++)
			{
				JobInfo info = session.wait(this.allJobIds[count], Session.DRMAA_TIMEOUT_WAIT_FOREVER);
				
				if (info.ifAborted())
					System.out.println("Job " + info.getJobId() + " never ran");
				else if (info.ifExited())
					System.out.println("Job " + info.getJobId() + " finished regularly with exit status " + info.getExitStatus());
				else if (info.ifSignaled())
					System.out.println("Job " + info.getJobId() + " finished due to signal " + info.getTerminatingSignal());
				else 
					System.out.println("Job " + info.getJobId() + " finished with unclear conditions");

				
				System.out.println("Job usage:");
				Map rmap = info.getResourceUsage();
				Iterator r = rmap.keySet().iterator();
				
				while(r.hasNext())
				{
					String name2 = (String) r.next();
					String value = (String) rmap.get(name2);
					System.out.println(" " + name2 + "=" + value);
				}
			}
			
			System.out.println("Succesfully finished test "+ this.type);
		}
		catch (ExitTimeoutException e)
		{
			System.err.println("Test "+ this.type +" failed");
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
			System.err.println("drmaa_exit() failed");
            		e.printStackTrace();	
			this.stateAllTest = false;
		}
	}
}
