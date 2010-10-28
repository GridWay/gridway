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

import org.ggf.drmaa.*;

import java.util.*;

public class Howto4
{
	public static void main (String[] args)
	{
		SessionFactory	factory = SessionFactory.getFactory();
		Session		session = factory.getSession();
		
		try
		{
			session.init(null);
			System.out.println("Session Init success");
					
			
			JobTemplate jt = session.createJobTemplate();
			
			jt.setWorkingDirectory(java.lang.System.getProperty("user.dir"));
			
			String	    name;		
			jt.setJobName("ht4");
									
			jt.setRemoteCommand("/bin/ls");

			ArrayList   jobArgs = new ArrayList();

			jobArgs.add("-l");
			jobArgs.add("-a");

			jt.setArgs(jobArgs);
			
			jt.setOutputPath("stdout." + GridWaySession.GW_JOB_ID);
			jt.setErrorPath("stderr." + GridWaySession.GW_JOB_ID);
			
			int start = 0;
			int end   = 3;
			int step  = 1;
			int i;
			
			String id;
			
			java.util.List 		ids = session.runBulkJobs(jt, start, end, step);
			java.util.Iterator	iter = ids.iterator();
			
			System.out.println("Bulk job successfully submitted IDs are: ");
			
			while(iter.hasNext())
			{
				System.out.println("\t" + iter.next());
			}
			
			session.deleteJobTemplate(jt);
			
			session.synchronize(Collections.singletonList(Session.JOB_IDS_SESSION_ALL), Session.TIMEOUT_WAIT_FOREVER, false);
			
			for (int count = start; count <= end;count += step)
			{
				JobInfo info = session.wait(Session.JOB_IDS_SESSION_ANY, Session.TIMEOUT_WAIT_FOREVER);
				
				if (info.wasAborted())
					System.out.println("Job " + info.getJobId() + " never ran");
				else if (info.hasExited())
					System.out.println("Job " + info.getJobId() + " finished regularly with exit status " + info.getExitStatus());
				else if (info.hasSignaled())
					System.out.println("Job " + info.getJobId() + " finished due to signal " + info.getTerminatingSignal());
				else 
					System.out.println("Job " + info.getJobId() + " finished with unclear conditions");

				
				System.out.println("Job usage:");
				Map rmap = info.getResourceUsage();
				Iterator r = rmap.keySet().iterator();
				
				while(r.hasNext())
				{
					String name2 = (String) r.next();
					String value = (String) rmap.get(name2);
					System.out.println(" " + name2 + "=" + value);
				}
			}
			session.exit();
			System.out.println("Session Exit success");

		}
		catch (DrmaaException e)
		{
			System.out.println("Error:   " + e.getMessage());
		}
	}
}
