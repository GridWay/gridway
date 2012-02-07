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

package org.ggf.drmaa.testSuite;
import org.ggf.drmaa.*;
import java.util.*;

public abstract class Test 
{
	public static final java.lang.String ALL_TEST 					= "ALL_TEST";
	public static final java.lang.String ST_SUBMIT_WAIT 				= "ST_SUBMIT_WAIT";
	public static final java.lang.String MT_SUBMIT_WAIT 				= "MT_SUBMIT_WAIT";
	public static final java.lang.String MT_SUBMIT_BEFORE_INIT_WAIT 		= "MT_SUBMIT_BEFORE_INIT_WAIT";
	public static final java.lang.String ST_MULT_INIT 				= "ST_MULT_INIT";
	public static final java.lang.String ST_MULT_EXIT 				= "ST_MULT_EXIT";
	public static final java.lang.String MT_EXIT_DURING_SUBMIT 			= "MT_EXIT_DURING_SUBMIT";
	public static final java.lang.String MT_SUBMIT_MT_WAIT				= "MT_SUBMIT_MT_WAIT";
	public static final java.lang.String MT_EXIT_DURING_SUBMIT_OR_WAIT		= "MT_EXIT_DURING_SUBMIT_OR_WAIT";
	public static final java.lang.String ST_BULK_SUBMIT_WAIT 			= "ST_BULK_SUBMIT_WAIT";
	public static final java.lang.String ST_BULK_SINGLESUBMIT_WAIT_INDIVIDUAL	= "ST_BULK_SINGLESUBMIT_WAIT_INDIVIDUAL";
	public static final java.lang.String ST_SUBMITMIXTURE_SYNC_ALL_DISPOSE		= "ST_SUBMITMIXTURE_SYNC_ALL_DISPOSE";
	public static final java.lang.String ST_SUBMITMIXTURE_SYNC_ALL_NODISPOSE	= "ST_SUBMITMIXTURE_SYNC_ALL_NODISPOSE";
	public static final java.lang.String ST_SUBMITMIXTURE_SYNC_ALLIDS_DISPOSE 	= "ST_SUBMITMIXTURE_SYNC_ALLIDS_DISPOSE";
	public static final java.lang.String ST_SUBMITMIXTURE_SYNC_ALLIDS_NODISPOSE 	= "ST_SUBMITMIXTURE_SYNC_ALLIDS_NODISPOSE";
	public static final java.lang.String ST_INPUT_FILE_FAILURE 			= "ST_INPUT_FILE_FAILURE";
	public static final java.lang.String ST_OUTPUT_FILE_FAILURE 			= "ST_OUTPUT_FILE_FAILURE";
	public static final java.lang.String ST_ERROR_FILE_FAILURE 			= "ST_ERROR_FILE_FAILURE";
	public static final java.lang.String ST_SUBMIT_IN_HOLD_RELEASE			= "ST_SUBMIT_IN_HOLD_RELEASE";
	public static final java.lang.String ST_SUBMIT_IN_HOLD_DELETE 			= "ST_SUBMIT_IN_HOLD_DELETE";
	public static final java.lang.String ST_BULK_SUBMIT_IN_HOLD_SINGLE_RELEASE 	= "ST_BULK_SUBMIT_IN_HOLD_SINGLE_RELEASE";
	public static final java.lang.String ST_BULK_SUBMIT_IN_HOLD_SESSION_RELEASE 	= "ST_BULK_SUBMIT_IN_HOLD_SESSION_RELEASE";
	public static final java.lang.String ST_BULK_SUBMIT_IN_HOLD_SINGLE_DELETE 	= "ST_BULK_SUBMIT_IN_HOLD_SINGLE_DELETE";
	public static final java.lang.String ST_BULK_SUBMIT_IN_HOLD_SESSION_DELETE	= "ST_BULK_SUBMIT_IN_HOLD_SESSION_DELETE";
	public static final java.lang.String ST_EXIT_STATUS				= "ST_EXIT_STATUS";
	public static final java.lang.String ST_SUPPORTED_ATTR				= "ST_SUPPORTED_ATTR";
	public static final java.lang.String ST_SUPPORTED_VATTR 			= "ST_SUPPORTED_VATTR";
	public static final java.lang.String ST_VERSION 				= "ST_VERSION";
	public static final java.lang.String ST_CONTACT 				= "ST_CONTACT";
	public static final java.lang.String ST_DRM_SYSTEM 				= "ST_DRM_SYSTEM";
	public static final java.lang.String ST_DRMAA_IMPL 				= "ST_DRMAA_IMPL";
	public static final java.lang.String ST_EMPTY_SESSION_WAIT 			= "ST_EMPTY_SESSION_WAIT";
	public static final java.lang.String ST_EMPTY_SESSION_SYNCHRONIZE_DISPOSE 	= "ST_EMPTY_SESSION_SYNCHRONIZE_DISPOSE";
	public static final java.lang.String ST_EMPTY_SESSION_SYNCHRONIZE_NODISPOSE 	= "ST_EMPTY_SESSION_SYNCHRONIZE_NODISPOSE";
	public static final java.lang.String ST_EMPTY_SESSION_CONTROL 			= "ST_EMPTY_SESSION_CONTROL";
	public static final java.lang.String ST_SUBMIT_SUSPEND_RESUME_WAIT 		= "ST_SUBMIT_SUSPEND_RESUME_WAIT";
	public static final java.lang.String ST_SUBMIT_POLLING_WAIT_TIMEOUT		= "ST_SUBMIT_POLLING_WAIT_TIMEOUT";
	public static final java.lang.String ST_SUBMIT_POLLING_WAIT_ZEROTIMEOUT 	= "ST_SUBMIT_POLLING_WAIT_ZEROTIMEOUT";
	public static final java.lang.String ST_SUBMIT_POLLING_SYNCHRONIZE_TIMEOUT 	= "ST_SUBMIT_POLLING_SYNCHRONIZE_TIMEOUT";
	public static final java.lang.String ST_SUBMIT_POLLING_SYNCHRONIZE_ZEROTIMEOUT  = "ST_SUBMIT_POLLING_SYNCHRONIZE_ZEROTIMEOUT";
	public static final java.lang.String ST_ATTRIBUTE_CHANGE			= "ST_ATTRIBUTE_CHANGE";
	public static final java.lang.String ST_USAGE_CHECK 				= "ST_USAGE_CHECK";
	public static final java.lang.String ST_UNSUPPORTED_ATTR 			= "ST_UNSUPPORTED_ATTR";
	public static final java.lang.String ST_UNSUPPORTED_VATTR 			= "ST_UNSUPPORTED_VATTR";
	public static final java.lang.String ST_SUBMIT_KILL_SIG 			= "ST_SUBMIT_KILL_SIG";
	
	protected String			type;
	protected String 			id;
	protected SessionFactory  		factory = SessionFactory.getFactory();
	protected Session         		session = factory.getSession();
	protected JobTemplate			jt;
	protected CreateJobTemplate		createJob;
	protected int 				jobChunk = 2;
	protected int 				nBulks 	 = 3;
	protected boolean			stateAllTest = true;
	protected String			executable;

	
	Test(){}
	
	Test(String type)
	{
		this.type = type;
		System.out.println("TEST: " + this.type);
	}
	
	Test(String executable, String type)
	{
		this.executable = executable;
		this.type = type;
		System.out.println("TEST: " + this.type);
	}
	
	public abstract void run();
	public boolean getState()
	{
		return this.stateAllTest;
	}
}
