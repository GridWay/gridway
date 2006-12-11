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
import java.util.*;

public class StSubmitInHoldRelease extends StSubmitInHold
{
	public StSubmitInHoldRelease(String executable)
	{
		super(executable, ST_SUBMIT_IN_HOLD_RELEASE);
	}

	public void run()
	{
		try
		{				
			this.runHold();
					
			if (this.status!=Session.DRMAA_PS_USER_ON_HOLD)
			{
				System.err.println("Job " + this.id + " is not in user hold state");
				System.err.println("Test " + this.type + " failed");
				return;
			}
			
			this.session.control(this.id,Session.DRMAA_CONTROL_RELEASE);
						
			this.session.synchronize(Collections.singletonList(Session.DRMAA_JOB_IDS_SESSION_ALL),
					    	 Session.DRMAA_TIMEOUT_WAIT_FOREVER, false);
			
			this.status = this.session.getJobProgramStatus(this.id);
			
			if (this.status!=Session.DRMAA_PS_DONE)
			{
				System.err.println("Test " + this.type + " failed");
				return;
			}
			
			this.session.wait(this.id, Session.DRMAA_TIMEOUT_WAIT_FOREVER);
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
