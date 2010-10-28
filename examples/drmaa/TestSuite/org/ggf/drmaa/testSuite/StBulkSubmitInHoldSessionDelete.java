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
import java.util.*;

public class StBulkSubmitInHoldSessionDelete extends StBulkSubmitInHold
{
	public StBulkSubmitInHoldSessionDelete(String executable)
	{
		super(executable, ST_BULK_SUBMIT_IN_HOLD_SESSION_DELETE);
	}

	public void run()
	{
		try
		{				
			this.runHold();
			
			this.session.control(Session.DRMAA_JOB_IDS_SESSION_ALL,Session.DRMAA_CONTROL_TERMINATE);
			
			for (int i=0;i<this.jobChunk;i++)
			{
				this.status = this.session.getJobProgramStatus((String) this.ids.get(i));	
				if (this.status!=Session.DRMAA_PS_FAILED)
				{
					System.err.println("Test " + this.type + " failed");
					this.stateAllTest = false;
					return;
				}
			}
			
			for (int i=0;i<this.jobChunk;i++)
				this.session.wait((String) this.ids.get(i), Session.DRMAA_TIMEOUT_WAIT_FOREVER);
			
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
