/* -------------------------------------------------------------------------- */
/* Copyright 2002-2006 GridWay Team, Distributed Systems Architecture         */
/* Group, Universidad Complutense de Madrid                                   */
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
		
			Properties		job_env = new Properties();
			PartialTimestamp	startTime = new PartialTimestamp();
			
			
			job_env.setProperty("job_env", "env1");
			startTime.setModifier(Calendar.HOUR_OF_DAY, 11);
			startTime.setModifier(Calendar.MINUTE, 11);
			
			
			this.session.init(null);
            		System.out.println("Session Init success");
			
			System.out.println("Testing change of job template attributes");
			System.out.println("Getting job template");
			
			
			this.jt = this.session.createJobTemplate();
			
			System.out.println("Filling job template for the first time");
			
			this.jt.setRemoteCommand("job1");
			this.jt.setArgs(new String[] {"argv1"});
			this.jt.setJobSubmissionState("drmaa_hold");
			this.jt.setJobEnvironment(job_env);
			this.jt.setWorkingDirectory("/tmp1");
			this.jt.setJobCategory("category1");
			this.jt.setNativeSpecification("native1");
			this.jt.setEmail(new String[] {"email1"});
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
			startTime.setModifier(Calendar.HOUR_OF_DAY, 11);
			startTime.setModifier(Calendar.MINUTE, 22);
			
			this.jt.setRemoteCommand("job2");
			this.jt.setArgs(new String[] {"argv2"});
			this.jt.setJobSubmissionState("drmaa_active");
			this.jt.setJobEnvironment(job_env);
			this.jt.setWorkingDirectory("/tmp2");
			this.jt.setJobCategory("category2");
			this.jt.setNativeSpecification("native2");
			this.jt.setEmail(new String[] {"email2"});
			this.jt.setBlockEmail(false);
			this.jt.setStartTime(startTime);
			this.jt.setJobName("jobName2");
			this.jt.setInputPath(":/dev/null2");
			this.jt.setOutputPath(":/dev/null2");
			this.jt.setErrorPath(":/dev/null2");
			this.jt.setJoinFiles(false);
			
			System.out.println("Checking current values of job template");
			
			this.session.runJob(jt);
			
			if (this.jt.getArgs().length != 1)
				System.out.println("getArgs failed. The legnth of the arguments is no correct");
			else if (!this.jt.getArgs()[0].equals("argv2"))
				System.out.println("Incorrect value after change for DRMAA_V_ARGV attribute");
			else if (!this.jt.getJobEnvironment().getProperty("job_env").equals("env2"))
				System.out.println("Incorrect value after change for DRMAA_V_ENV attribute");
			else if (this.jt.getEmail().length !=1)
				System.out.println("getEmail failed. The length of the arguments is no correct");
			else if (!this.jt.getEmail()[0].equals("email2"))
				System.out.println("Incorrect value after change for DRMAA_V_EMAIL attribute");
			else if (!this.jt.getRemoteCommand().equals("job2"))
				System.out.println("Incorrect value after change for DRMAA_REMOTE_COMMAND attribute");
			else if (!this.jt.getJobSubmissionState().equals("drmaa_active"))
				System.out.println("Incorrect value after change for DRMAA_JS_STATE attribute");
			else if (!this.jt.getWorkingDirectory().equals("/tmp2"))
				System.out.println("Incorrect value after change for DRMAA_WD attribute");
			else if (!this.jt.getJobCategory().equals("category2"))
				System.out.println("Incorrect value after change for DRMAA_JOB_CATEGORY attribute");
			else if (!this.jt.getNativeSpecification().equals("native2"))
				System.out.println("Incorrect value after change for DRMAA_NATIVE_SPECIFICATION attribute");
			else if (this.jt.getBlockEmail())
				System.out.println("Incorrect value after change for DRMAA_BLOCK_EMAIL attribute");
			else if (this.jt.getStartTime().getModifier(Calendar.HOUR_OF_DAY)!=11 &&
				 this.jt.getStartTime().getModifier(Calendar.HOUR_OF_DAY)!=22)
				System.out.println("Incorrect value after change for DRMAA_START_TIME attribute");	
			else if (!this.jt.getJobName().equals("jobName2"))
				System.out.println("Incorrect value after change for DRMAA_JOB_NAME attribute");
			else if (!this.jt.getInputPath().equals(":/dev/null2"))
				System.out.println("Incorrect value after change for DRMAA_INPUT_PATH attribute");
			else if (!this.jt.getOutputPath().equals(":/dev/null2"))
				System.out.println("Incorrect value after change for DRMAA_OUTPUT_PATH attribute");
			else if (!this.jt.getErrorPath().equals(":/dev/null2"))
				System.out.println("Incorrect value after change for DRMAA_ERROR_PATH attribute");
			else if (this.jt.getJoinFiles())
				System.out.println("Incorrect value after change for DRMAA_JOIN_FILES attribute");
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
