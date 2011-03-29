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

public abstract class ThreadExecution extends Thread 
{
	protected Session			session;
	protected String			name;
	protected String 			executable;
	protected List				args;
	protected String			type;
	protected boolean		isException = false;
	protected Exception		exception;
	protected int				nJobs;

	public ThreadExecution(Session session, String executable, int nThreads, String type, int nJobs)
	{
		this.session = session;
		this.name = "thread" + nThreads;
		this.executable = executable;
		this.args = new ArrayList();
		this.args.add(Integer.toString(nThreads));
		this.type = type;
		this.nJobs = nJobs; 
	}
	
	public void setException(Exception e) 
	{
		this.isException = true;
		this.exception = e;
	}
		
	public void throwException() throws Exception
	{
		if (this.isException)
			throw(this.exception);
	}
	
}
