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

public class TestDrmaa 
{
	
	public static void main(String[] args) 
	{
		Test	testDrmaa = null;
		String	name;
		String	executable;
		Usage   use = new Usage();
		
		if (args.length<=1)
		{
			use.message();
			return;
		}
		
		name=args[0];
		executable = args[1];
		
		if (name.equals(Test.ST_MULT_INIT))
			testDrmaa = new StMultInit();
		else if (name.equals(Test.ST_MULT_EXIT))
			testDrmaa = new StMultExit();
		else if (name.equals(Test.ST_SUBMIT_WAIT))
			testDrmaa = new StSubmitWait(executable);
		else if (name.equals(Test.ST_SUBMIT_POLLING_WAIT_TIMEOUT))
			testDrmaa = new StSubmitPollingWaitTimeout(executable);
		else if (name.equals(Test.ST_SUBMIT_POLLING_WAIT_ZEROTIMEOUT))
			testDrmaa = new StSubmitPollingWaitZeroTimeout(executable);
		else if (name.equals(Test.ST_SUBMIT_POLLING_SYNCHRONIZE_TIMEOUT))
			testDrmaa = new StSubmitPollingSynchronizeTimeout(executable);
		else if (name.equals(Test.ST_SUBMIT_POLLING_SYNCHRONIZE_ZEROTIMEOUT))
			testDrmaa = new StSubmitPollingSynchronizeZeroTimeout(executable);
		else if (name.equals(Test.ST_BULK_SUBMIT_WAIT))
			testDrmaa = new StBulkSubmitWait(executable);
		else if (name.equals(Test.ST_BULK_SINGLESUBMIT_WAIT_INDIVIDUAL))
			testDrmaa = new StBulkSingleSubmitWaitIndividual(executable);
		else if (name.equals(Test.ST_SUBMITMIXTURE_SYNC_ALL_DISPOSE))
			testDrmaa = new StSubmitMixtureSyncAllDispose(executable);
		else if (name.equals(Test.ST_SUBMITMIXTURE_SYNC_ALL_NODISPOSE))
			testDrmaa = new StSubmitMixtureSyncAllNoDispose(executable);
		else if (name.equals(Test.ST_SUBMITMIXTURE_SYNC_ALLIDS_DISPOSE))
			testDrmaa = new StSubmitMixtureSyncAllIdsDispose(executable);
		else if (name.equals(Test.ST_SUBMITMIXTURE_SYNC_ALLIDS_NODISPOSE))
			testDrmaa = new StSubmitMixtureSyncAllIdsNoDispose(executable);
		else if (name.equals(Test.ST_INPUT_FILE_FAILURE))
			testDrmaa = new StInputFileFailure(executable);
		else if (name.equals(Test.ST_OUTPUT_FILE_FAILURE))
			testDrmaa = new StOutputFileFailure(executable);
		else if (name.equals(Test.ST_ERROR_FILE_FAILURE))
			testDrmaa = new StErrorFileFailure(executable);
		else if (name.equals(Test.ST_SUPPORTED_ATTR))
			testDrmaa = new StSupportedAttr(executable);
		else if (name.equals(Test.ST_VERSION))
			testDrmaa = new StVersion();
		else if (name.equals(Test.ST_CONTACT))
			testDrmaa = new StContact();
		else if (name.equals(Test.ST_DRM_SYSTEM))
			testDrmaa = new StDRMSystem();
		else if (name.equals(Test.ST_DRMAA_IMPL))
			testDrmaa = new StDRMAAImpl();
		else if (name.equals(Test.ST_SUBMIT_SUSPEND_RESUME_WAIT))
			testDrmaa = new StSubmitSuspendResumeWait(executable);	
		else if (name.equals(Test.ST_EMPTY_SESSION_WAIT))
			testDrmaa = new StEmptySessionWait();	
		else if (name.equals(Test.ST_EMPTY_SESSION_SYNCHRONIZE_DISPOSE))
			testDrmaa = new StEmptySessionSynchronizeDispose();	
		else if (name.equals(Test.ST_EMPTY_SESSION_SYNCHRONIZE_NODISPOSE))
			testDrmaa = new StEmptySessionSynchronizeNoDispose();	
		else if (name.equals(Test.ST_EMPTY_SESSION_CONTROL))
			testDrmaa = new StEmptySessionControl();
		else if (name.equals(Test.ST_ATTRIBUTE_CHANGE))
			testDrmaa = new StAttributeChange();	
		else if (name.equals(Test.ST_USAGE_CHECK))
			testDrmaa = new StUsageCheck(executable);	
		else if (name.equals(Test.MT_SUBMIT_WAIT))
			testDrmaa = new MtSubmitWait(executable);	
		else if (name.equals(Test.MT_SUBMIT_BEFORE_INIT_WAIT))
			testDrmaa = new MtSubmitBeforeInitWait(executable);	
		else if (name.equals(Test.MT_EXIT_DURING_SUBMIT))
			testDrmaa = new MtExitDuringSubmit(executable);			
		else if (name.equals(Test.MT_SUBMIT_MT_WAIT))
			testDrmaa = new MtSubmitMtWait(executable);		
		else if (name.equals(Test.MT_EXIT_DURING_SUBMIT_OR_WAIT))
			testDrmaa = new MtExitDuringSubmitOrWait(executable);		
		else if (name.equals(Test.ST_SUBMIT_IN_HOLD_RELEASE))
			testDrmaa = new StSubmitInHoldRelease(executable);		
		else if (name.equals(Test.ST_SUBMIT_IN_HOLD_DELETE))
			testDrmaa = new StSubmitInHoldDelete(executable);				
		else if (name.equals(Test.ST_BULK_SUBMIT_IN_HOLD_SESSION_RELEASE))
			testDrmaa = new StBulkSubmitInHoldSessionRelease(executable);
		else if (name.equals(Test.ST_BULK_SUBMIT_IN_HOLD_SINGLE_RELEASE))
			testDrmaa = new StBulkSubmitInHoldSingleRelease(executable);
		else if (name.equals(Test.ST_BULK_SUBMIT_IN_HOLD_SESSION_DELETE))
			testDrmaa = new StBulkSubmitInHoldSessionDelete(executable);		
		else if (name.equals(Test.ST_BULK_SUBMIT_IN_HOLD_SINGLE_DELETE))
			testDrmaa = new StBulkSubmitInHoldSingleDelete(executable);		
		else if (name.equals(Test.ALL_TEST))
			testDrmaa = new AllTest(executable);
		else
		{
			use.message();
			return;
		}
		testDrmaa.run();
	}

}
