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

public class StAttributeChange extends Test
{
	public StAttributeChange()
	{
		super(ST_ATTRIBUTE_CHANGE);
	}

	public void run()
	{
		int 				n = 20;
				
		try
		{
		
			Properties		job_env 	= new Properties();
			Date				startTime ;
			Calendar		calendar 	= Calendar.getInstance();
			
			job_env.setProperty("job_env", "env1");
			
			calendar.set(1975, 11, 20);
			startTime= calendar.getTime();
			
			this.session.init(null);
            		System.out.println("Session Init success");
			
			System.out.println("Testing change of job template attributes");
			System.out.println("Getting job template");
			
			
			this.jt = this.session.createJobTemplate();
			
			System.out.println("Filling job template for the first time");
			
			ArrayList  args = new ArrayList();
			args.add("argv1");
			
			HashSet email = new HashSet();
			email.add("email1");
			
			this.jt.setRemoteCommand("job1");
			this.jt.setArgs(args);
			this.jt.setJobSubmissionState(JobTemplate.HOLD_STATE);
			this.jt.setJobEnvironment(job_env);
			this.jt.setWorkingDirectory("/tmp1");
			this.jt.setJobCategory("category1");
			this.jt.setNativeSpecification("native1");
			this.jt.setEmail(email);
			this.jt.setBlockEmail(true);
			
			this.jt.setStartTime(startTime);
			this.jt.setJobName("jobName1");
			this.jt.setInputPath(":/dev/null1");
			this.jt.setOutputPath(":/dev/null1");
			this.jt.setErrorPath(":/dev/null1");
			this.jt.setJoinFiles(true);
			
			System.out.println("Filling job template for the second time");
			
			job_env = new Properties();
			job_env.setProperty("job_env", "env2");
			job_env.setProperty("job_env5", "env7");
			job_env.setProperty("job_env10", "env10");
			job_env.setProperty("job_env99", "env99");
			
			
			calendar.set(1978, 12, 6);
			startTime = calendar.getTime();
			
			args = new ArrayList();
			args.add("argv2");
			
			email = new HashSet();
			email.add("email2");
			
			
			this.jt.setRemoteCommand("job2");
			this.jt.setArgs(args);
			this.jt.setJobSubmissionState(JobTemplate.ACTIVE_STATE);
			this.jt.setJobEnvironment(job_env);
			this.jt.setWorkingDirectory("/tmp2");
			this.jt.setJobCategory("category2");
			this.jt.setNativeSpecification("native2");
			this.jt.setEmail(email);
			this.jt.setBlockEmail(false);
			this.jt.setStartTime(startTime);
			this.jt.setJobName("jobName2");
			this.jt.setInputPath(":/dev/null2");
			this.jt.setOutputPath(":/dev/null2");
			this.jt.setErrorPath(":/dev/null2");
			this.jt.setJoinFiles(false);
			
			System.out.println("Checking current values of job template");
			
			this.session.runJob(jt);
			
		
			if (this.jt.getArgs().size() != 1)
				System.out.println("getArgs failed. The legnth of the arguments is no correct");
			else if (!this.jt.getArgs().get(0).equals("argv2"))
				System.out.println("Incorrect value after change for args attribute");
			else if (!((String) this.jt.getJobEnvironment().get("job_env")).equals("env2"))
				System.out.println("Incorrect value after change for jobEnvironment attribute");
			else if (this.jt.getEmail().size() !=1)
				System.out.println("getEmail failed. The length of the arguments is no correct");
			else if (this.jt.getEmail().iterator()!=null)
			{
				Iterator it = this.jt.getEmail().iterator();
				String value = (String) it.next();
				
				if (value != null && value.equals("email2"))
					System.out.println("Incorrect value after change for email attribute");
			}
			else if (!this.jt.getRemoteCommand().equals("job2"))
				System.out.println("Incorrect value after change for remoteCommand attribute");
			else if (this.jt.getJobSubmissionState()!=JobTemplate.ACTIVE_STATE)
				System.out.println("Incorrect value after change for jobSubmissionState attribute");
			else if (!this.jt.getWorkingDirectory().equals("/tmp2"))
				System.out.println("Incorrect value after change for workingDirectory attribute");
			else if (!this.jt.getJobCategory().equals("category2"))
				System.out.println("Incorrect value after change for jobCategory attribute");
			else if (!this.jt.getNativeSpecification().equals("native2"))
				System.out.println("Incorrect value after change for nativeSpecification attribute");
			else if (this.jt.getBlockEmail())
				System.out.println("Incorrect value after change for blockEmail attribute");
			else if (this.jt.getStartTime().compareTo(calendar.getTime())!=0)
				System.out.println("Incorrect value after change for startTime attribute");	
			else if (!this.jt.getJobName().equals("jobName2"))
				System.out.println("Incorrect value after change for jobName attribute");
			else if (!this.jt.getInputPath().equals(":/dev/null2"))
				System.out.println("Incorrect value after change for inputPath attribute");
			else if (!this.jt.getOutputPath().equals(":/dev/null2"))
				System.out.println("Incorrect value after change for outputPath attribute");
			else if (!this.jt.getErrorPath().equals(":/dev/null2"))
				System.out.println("Incorrect value after change for errorPath attribute");
			else if (this.jt.getJoinFiles())
				System.out.println("Incorrect value after change for joinFiles attribute");
			else
				System.out.println("Succesfully finished test "+ this.type);	
		}
		catch (DrmaaException e)
		{
			System.err.println("Test "+ this.type +" failed");
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
