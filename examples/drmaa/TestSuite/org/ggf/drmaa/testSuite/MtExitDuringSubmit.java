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

public class MtExitDuringSubmit extends Mt 
{	
	public MtExitDuringSubmit(String executable)
	{
		super(executable, MT_EXIT_DURING_SUBMIT);
	}
	
	public void run()
	{
		try
		{
			this.session.init(null);
            		System.out.println("Session Init success");
			
			for (int i=0; i<this.nThreads;i++)
			{
				threads[i] = new ThreadExecutionSubmit(this.session, this.executable, i, this.type, this.jobChunk);
				threads[i].start();
				threads[i].throwException();
			}

			Thread.sleep(1000);
			System.out.println("Exiting...");				
			this.session.exit();
			System.out.println("Join for jobs...");				
			
			for (int i=0; i<this.nThreads;i++)
			{	
				threads[i].join();
			 	threads[i].throwException();
			}
								
            		System.out.println("Succesfully finished test "+ this.type);
		}
		catch (NoActiveSessionException e)
		{
			System.out.println("Succesfully finished test "+ this.type);
		}
		catch (DrmCommunicationException e)
		{
			System.out.println("Succesfully finished test "+ this.type);
		}
		catch (Exception e)
		{
			System.err.println("Test " + this.type +" failed");
            		e.printStackTrace();
			this.stateAllTest = false;
		}
		
	}
}
