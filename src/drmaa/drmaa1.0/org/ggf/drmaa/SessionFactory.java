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

package org.ggf.drmaa;


/** This class allows the Java language binding implementation to support 
 *  multiple different DRMAA vendors. The SessionFactory class returns a 
 *  GridWay DRMAA SessionFactory implementation.
 */

public abstract class SessionFactory 
{
	private static SessionFactory sessionFactory = null;
	
	/** This method returns a SessionFactory object for the GridWay DRMAA Java implementation.*/
	public static SessionFactory getFactory()
	{
		if (sessionFactory == null)
		 sessionFactory = new GridWaySessionFactory();
		return sessionFactory;	
	}
	
	/** This method returns a Session object for the GridWay DRMAA Java implementation.*/
	public abstract Session getSession();
}
