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

public class TestDrmaa 
{
	
	public static void main(String[] args) 
	{
		Test	testDrmaa = null;
		String	name = args[0];
		String	executable = null;
		Usage   use = new Usage();
		
		if (args.length<1 || (args.length==1 && !(name.equals(Test.ST_MULT_INIT) 				||
							  name.equals(Test.ST_MULT_EXIT) 				||
							  name.equals(Test.ST_VERSION)	 				||
							  name.equals(Test.ST_CONTACT)					||
							  name.equals(Test.ST_DRM_SYSTEM)				||
							  name.equals(Test.ST_DRMAA_IMPL)				||
							  name.equals(Test.ST_EMPTY_SESSION_WAIT)			||
							  name.equals(Test.ST_EMPTY_SESSION_SYNCHRONIZE_DISPOSE)	||
							  name.equals(Test.ST_EMPTY_SESSION_SYNCHRONIZE_NODISPOSE)	||
							  name.equals(Test.ST_EMPTY_SESSION_CONTROL)))			||
		args.length>2)					
		{
			use.message();
			return;
		}
		
		
		
		if (args.length == 2) 
			executable = args[1];
		
		if (name.equals(Test.ST_MULT_INIT)) //OK
			testDrmaa = new StMultInit();
		else if (name.equals(Test.ST_MULT_EXIT)) //OK
			testDrmaa = new StMultExit();
		else if (name.equals(Test.ST_SUBMIT_WAIT)) //OK
			testDrmaa = new StSubmitWait(executable);
		else if (name.equals(Test.ST_SUBMIT_POLLING_WAIT_TIMEOUT)) //OK
			testDrmaa = new StSubmitPollingWaitTimeout(executable);
		else if (name.equals(Test.ST_SUBMIT_POLLING_WAIT_ZEROTIMEOUT))//OK
			testDrmaa = new StSubmitPollingWaitZeroTimeout(executable);
		else if (name.equals(Test.ST_SUBMIT_POLLING_SYNCHRONIZE_TIMEOUT))//OK
			testDrmaa = new StSubmitPollingSynchronizeTimeout(executable);
		else if (name.equals(Test.ST_SUBMIT_POLLING_SYNCHRONIZE_ZEROTIMEOUT))//OK
			testDrmaa = new StSubmitPollingSynchronizeZeroTimeout(executable);
		else if (name.equals(Test.ST_BULK_SUBMIT_WAIT))//OK
			testDrmaa = new StBulkSubmitWait(executable);
		else if (name.equals(Test.ST_BULK_SINGLESUBMIT_WAIT_INDIVIDUAL))//OK
			testDrmaa = new StBulkSingleSubmitWaitIndividual(executable);
		else if (name.equals(Test.ST_SUBMITMIXTURE_SYNC_ALL_DISPOSE))//OK
			testDrmaa = new StSubmitMixtureSyncAllDispose(executable);
		else if (name.equals(Test.ST_SUBMITMIXTURE_SYNC_ALL_NODISPOSE))//OK
			testDrmaa = new StSubmitMixtureSyncAllNoDispose(executable);
		else if (name.equals(Test.ST_SUBMITMIXTURE_SYNC_ALLIDS_DISPOSE))//OK
			testDrmaa = new StSubmitMixtureSyncAllIdsDispose(executable);
		else if (name.equals(Test.ST_SUBMITMIXTURE_SYNC_ALLIDS_NODISPOSE))//OK
			testDrmaa = new StSubmitMixtureSyncAllIdsNoDispose(executable);
		else if (name.equals(Test.ST_INPUT_FILE_FAILURE))//OK
			testDrmaa = new StInputFileFailure(executable);
		else if (name.equals(Test.ST_OUTPUT_FILE_FAILURE))//OK
			testDrmaa = new StOutputFileFailure(executable);
		else if (name.equals(Test.ST_ERROR_FILE_FAILURE))//OK
			testDrmaa = new StErrorFileFailure(executable);
		else if (name.equals(Test.ST_SUPPORTED_ATTR))//OK
			testDrmaa = new StSupportedAttr(executable);
		else if (name.equals(Test.ST_VERSION))//OK
			testDrmaa = new StVersion();
		else if (name.equals(Test.ST_CONTACT))//OK
			testDrmaa = new StContact();
		else if (name.equals(Test.ST_DRM_SYSTEM))//OK
			testDrmaa = new StDRMSystem();
		else if (name.equals(Test.ST_DRMAA_IMPL))//OK
			testDrmaa = new StDRMAAImpl();
		else if (name.equals(Test.ST_SUBMIT_SUSPEND_RESUME_WAIT))//OK
			testDrmaa = new StSubmitSuspendResumeWait(executable);	
		else if (name.equals(Test.ST_EMPTY_SESSION_WAIT))//OK
			testDrmaa = new StEmptySessionWait();	
		else if (name.equals(Test.ST_EMPTY_SESSION_SYNCHRONIZE_DISPOSE))//OK
			testDrmaa = new StEmptySessionSynchronizeDispose();	
		else if (name.equals(Test.ST_EMPTY_SESSION_SYNCHRONIZE_NODISPOSE))//OK
			testDrmaa = new StEmptySessionSynchronizeNoDispose();	
		else if (name.equals(Test.ST_EMPTY_SESSION_CONTROL))//OK
			testDrmaa = new StEmptySessionControl();
		else if (name.equals(Test.ST_ATTRIBUTE_CHANGE))//KO
			testDrmaa = new StAttributeChange();	
		else if (name.equals(Test.ST_USAGE_CHECK))//OK
			testDrmaa = new StUsageCheck(executable);	
		else if (name.equals(Test.MT_SUBMIT_WAIT))//OK
			testDrmaa = new MtSubmitWait(executable);	
		else if (name.equals(Test.MT_SUBMIT_BEFORE_INIT_WAIT))//OK
			testDrmaa = new MtSubmitBeforeInitWait(executable);	
		else if (name.equals(Test.MT_EXIT_DURING_SUBMIT))//OK
			testDrmaa = new MtExitDuringSubmit(executable);			
		else if (name.equals(Test.MT_SUBMIT_MT_WAIT))//OK
			testDrmaa = new MtSubmitMtWait(executable);		
		else if (name.equals(Test.MT_EXIT_DURING_SUBMIT_OR_WAIT))//OK
			testDrmaa = new MtExitDuringSubmitOrWait(executable);		
		else if (name.equals(Test.ST_SUBMIT_IN_HOLD_RELEASE))//OK
			testDrmaa = new StSubmitInHoldRelease(executable);		
		else if (name.equals(Test.ST_SUBMIT_IN_HOLD_DELETE))//OK
			testDrmaa = new StSubmitInHoldDelete(executable);				
		else if (name.equals(Test.ST_BULK_SUBMIT_IN_HOLD_SESSION_RELEASE))//OK
			testDrmaa = new StBulkSubmitInHoldSessionRelease(executable);
		else if (name.equals(Test.ST_BULK_SUBMIT_IN_HOLD_SINGLE_RELEASE))//OK
			testDrmaa = new StBulkSubmitInHoldSingleRelease(executable);
		else if (name.equals(Test.ST_BULK_SUBMIT_IN_HOLD_SESSION_DELETE))//OK
			testDrmaa = new StBulkSubmitInHoldSessionDelete(executable);		
		else if (name.equals(Test.ST_BULK_SUBMIT_IN_HOLD_SINGLE_DELETE))//OK
			testDrmaa = new StBulkSubmitInHoldSingleDelete(executable);		
		else if (name.equals(Test.ALL_TEST))//OK
			testDrmaa = new AllTest(executable);
		else
		{
			use.message();
			return;
		}
		testDrmaa.run();
	}

}
