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

public class StEmptySessionControl extends Test 
{
	public StEmptySessionControl()
	{
		super(ST_EMPTY_SESSION_CONTROL);
	}

	public void run()
	{
		try
		{
			this.session.init(null);
            		System.out.println("Session Init success");
			
			System.out.println(this.type + ": SUSPEND signal testing...");
			
			this.session.control(Session.JOB_IDS_SESSION_ALL, Session.SUSPEND);
			
			System.out.println(this.type + ": RESUME signal testing...");
			
			this.session.control(Session.JOB_IDS_SESSION_ALL, Session.RESUME);
			
			System.out.println(this.type + ": TERMINATE signal testing...");
			
			this.session.control(Session.JOB_IDS_SESSION_ALL, Session.TERMINATE);
			
			System.out.println(this.type + ": HOLD signal testing...");
			
			this.session.control(Session.JOB_IDS_SESSION_ALL, Session.HOLD);
			
			System.out.println(this.type + ": RELEASE signal testing...");
			
			this.session.control(Session.JOB_IDS_SESSION_ALL, Session.RELEASE);
			
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
