/* -------------------------------------------------------------------------- */
/* Copyright 2002-2013, GridWay Project Leads (GridWay.org)                   */
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

public class StVersion extends Test 
{
	public StVersion()
	{
		super(ST_VERSION);
	}

	public void run()
	{
		try
		{
			Version		version;
		
			version = this.session.getVersion();
		
			if (version.getMajor()!=1 || version.getMinor()!=0)
			{
				System.out.println("Wrong DRMAA version returned on drmaa_version() (was " + version + ", should be 1.0)");
				this.stateAllTest = false;
			}
            		else
			{
				System.out.println("version " + version);
			
				System.out.println("Succesfully finished test " + this.type);
			}

		}
		catch (Exception e)
		{
			System.err.println("Test " + this.type +" failed");
            		e.printStackTrace();
			this.stateAllTest = false;
		}
	}
}
