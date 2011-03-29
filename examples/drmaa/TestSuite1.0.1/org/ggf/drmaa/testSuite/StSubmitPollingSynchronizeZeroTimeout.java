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

public class StSubmitPollingSynchronizeZeroTimeout extends StSubmitPolling
{
	public StSubmitPollingSynchronizeZeroTimeout(String executable)
	{
		super(executable, ST_SUBMIT_POLLING_SYNCHRONIZE_ZEROTIMEOUT);
	}

	public void run()
	{	
		try
		{
			this.runPolling();
			do
			{
				try
				{
					this.session.synchronize(Collections.singletonList(Session.JOB_IDS_SESSION_ALL), Session.TIMEOUT_NO_WAIT, false);
					this.exitTimeoutException=false;
				}
				catch (ExitTimeoutException e)
				{
					this.exitTimeoutException=true;
					try{Thread.sleep(this.timeout);}
					catch(Exception ee){}
					System.out.println("Sleep " + this.timeout/1000 + " seconds");
				}
			}	
			while(exitTimeoutException);
			
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
