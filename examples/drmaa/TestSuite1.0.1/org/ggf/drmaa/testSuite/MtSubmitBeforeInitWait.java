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

public class MtSubmitBeforeInitWait extends Mt 
{
	public MtSubmitBeforeInitWait(String executable)
	{
		super(executable, MT_SUBMIT_BEFORE_INIT_WAIT);
	}
		
	public void run()
	{
		try
		{
			for (int i=0; i<this.nThreads;i++)
			{
				threads[i] = new ThreadExecutionSubmit(session, executable, i, this.type, this.jobChunk);
				threads[i].start();
				threads[i].throwException();
			}

			Thread.sleep(5000);
				
			this.session.init(null);
            		System.out.println("Session Init success");

			
			for (int i=0; i<this.nThreads;i++)
				threads[i].join();
			 
			for (int i=0;i<this.nThreads*this.jobChunk;i++)
			{
				JobInfo info = this.session.wait(Session.JOB_IDS_SESSION_ANY, Session.TIMEOUT_WAIT_FOREVER);
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
