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

import org.ggf.drmaa.*;
import java.util.*;

public class Howto3
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
			jt.setJobName("ht3");
									
			jt.setRemoteCommand("/bin/ls");
			jt.setArgs(new String[] {"-l","-a"});
			
			jt.setOutputPath("stdout." + SessionImpl.DRMAA_GW_JOB_ID);
			jt.setErrorPath("stderr." + SessionImpl.DRMAA_GW_JOB_ID);
			jt.setJobSubmissionState(JobTemplate.HOLD);
			
			String id = session.runJob(jt);
			
			System.out.println("Job successfully submitted ID: " + id);
			
			try
			{
				Thread.sleep(5 * 1000);
			}
			catch (InterruptedException e)
			{
				// Don't care
			}
			
			printJobStatus(id, session);
			
			
			try
			{
				Thread.sleep(1000);
			}
			catch (InterruptedException e)
			{
				// Don't care
			}
			
			
			System.out.println("Releasing the Job");
			
			session.control(id, Session.DRMAA_CONTROL_RELEASE);
			
			printJobStatus(id, session);
			
			System.out.println("Synchronizing with job...");
			
			session.synchronize(Collections.singletonList(Session.DRMAA_JOB_IDS_SESSION_ALL),
					    Session.DRMAA_TIMEOUT_WAIT_FOREVER, false);
			
			System.out.println("Killing the Job");
			
			session.control(id, Session.DRMAA_CONTROL_TERMINATE);
			
			System.out.println("The job has been deleted...");
			
			session.deleteJobTemplate(jt);
			
			session.exit();
			System.out.println("Session Exit success");

		}
		catch (DrmaaException e)
		{
			e.printStackTrace();
		}
	}
	
	private static void printJobStatus(String id, Session session)
	{
		int status = 0;
		try 
		{
			status = session.getJobProgramStatus(id);
		}
		catch (DrmaaException e)
		{
			e.printStackTrace();
		}
			
		switch (status)
		{
			case Session.DRMAA_PS_UNDETERMINED:
				System.out.println("Job status cannot be determined");
				break;
				
			case Session.DRMAA_PS_QUEUED_ACTIVE:
				System.out.println("Job is queued and active");
				break;
				
			case Session.DRMAA_PS_SYSTEM_ON_HOLD:
				System.out.println("Job is queued and in system hold");
				break;
					
			case Session.DRMAA_PS_USER_ON_HOLD:
				System.out.println("Job is queued and in user hold");
				break;
					
			case Session.DRMAA_PS_USER_SYSTEM_ON_HOLD:
				System.out.println("Job is queued and in user and system hold");
				break;
					
			case Session.DRMAA_PS_RUNNING:
				System.out.println("Job is running");
				break;
				
			case Session.DRMAA_PS_SYSTEM_SUSPENDED:
				System.out.println("Job is system suspended");
				break;
					
			case Session.DRMAA_PS_USER_SUSPENDED:
				System.out.println("Job is user suspended");
				break;
				
			case Session.DRMAA_PS_USER_SYSTEM_SUSPENDED:
				System.out.println("Job is user and system suspended");
				break;
				
			case Session.DRMAA_PS_DONE:
				System.out.println("Job finished normally");
				break;
					
			case Session.DRMAA_PS_FAILED:
				System.out.println("Job finished, but failed");
				break;
			
		}
			
	}
}
