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

public class StSupportedAttr extends Test 
{
	public StSupportedAttr(String executable)
	{
		super(executable, ST_SUPPORTED_ATTR);
	}

	public void run()
	{
		ArrayList	listAttr;
		
		this.createJob = new CreateSleeperJobTemplate(this.session, this.executable, "5", false);
		
		try
		{
			this.session.init(null);
            		System.out.println("Session Init success");

            		this.jt = this.createJob.getJobTemplate();
			
			listAttr = (ArrayList) this.jt.getAttributeNames();
			
			for (int i=0; i<listAttr.size();i++)
			{
				String	attr = (String) listAttr.get(i);
				System.out.println(attr);
			}
			
			System.out.println("Succesfully finished test " + this.type);

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
