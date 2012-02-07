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

import org.ggf.drmaa.*;

import java.util.*;

public class Howto1
{
	public static void main (String[] args)
	{
		SessionFactory	factory = SessionFactory.getFactory();
		Session		session = factory.getSession();
		
		try
		{
			session.init(null);
			
			System.out.println("Session Init success");
			
			System.out.println ("Using " + session.getDrmaaImplementation() + ", details:");
			System.out.println ("\t DRMAA version " + session.getVersion());
			System.out.println ("\t DRMS " + session.getDrmsInfo() + "(contact: " + session.getContact() + ")");
		
			session.exit();
			System.out.println("Session Exit success");

		}
		catch (DrmaaException e)
		{
			e.printStackTrace();
		}
	}
}
