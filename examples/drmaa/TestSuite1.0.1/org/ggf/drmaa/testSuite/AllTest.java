/* -------------------------------------------------------------------------- */
/* Copyright 2002-2013, GridWay Project Leads (GridWay.org)                   */
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

public class AllTest extends Test 
{
	String	exec1;
	String	exec2;
	String	exec3;
	
	public AllTest(String exec1)
	{
		this.type = this.ALL_TEST;
		this.exec1 = exec1;
	}

	
	public AllTest(String exec1, String exec2, String exec3)
	{
		this.type = this.ALL_TEST;
		this.exec1 = exec1;
		this.exec2 = exec2;
		this.exec3 = exec3;
	}

	public void run()
	{
		Test		testDrmaa;
		boolean		state = true;
	
		testDrmaa = new StMultInit();
		testDrmaa.run();
		state = state && testDrmaa.getState();
		System.out.println();
		
		testDrmaa = new StMultExit();
		testDrmaa.run();
		state = state && testDrmaa.getState();
		System.out.println();
		
		testDrmaa = new StSubmitWait(this.exec1);
		testDrmaa.run();
		state = state && testDrmaa.getState();
		System.out.println();
		
		testDrmaa = new StSubmitPollingWaitTimeout(this.exec1);
		testDrmaa.run();
		state = state && testDrmaa.getState();
		System.out.println();
		
		testDrmaa = new StSubmitPollingWaitZeroTimeout(this.exec1);
		testDrmaa.run();
		state = state && testDrmaa.getState();
		System.out.println();
		
		testDrmaa = new StSubmitPollingSynchronizeTimeout(this.exec1);
		testDrmaa.run();
		state = state && testDrmaa.getState();
		System.out.println();
		
		testDrmaa = new StSubmitPollingSynchronizeZeroTimeout(this.exec1);
		testDrmaa.run();
		state = state && testDrmaa.getState();
		System.out.println();
		
		testDrmaa = new StBulkSubmitWait(this.exec1);
		testDrmaa.run();
		state = state && testDrmaa.getState();
		System.out.println();
		
		testDrmaa = new StBulkSingleSubmitWaitIndividual(this.exec1);
		testDrmaa.run();
		state = state && testDrmaa.getState();
		System.out.println();
		
		testDrmaa = new StSubmitMixtureSyncAllDispose(this.exec1);
		testDrmaa.run();
		state = state && testDrmaa.getState();
		System.out.println();
		
		testDrmaa = new StSubmitMixtureSyncAllNoDispose(this.exec1);
		testDrmaa.run();
		state = state && testDrmaa.getState();
		System.out.println();
		
		testDrmaa = new StSubmitMixtureSyncAllIdsDispose(this.exec1);
		testDrmaa.run();
		state = state && testDrmaa.getState();
		System.out.println();
		
		testDrmaa = new StSubmitMixtureSyncAllIdsNoDispose(this.exec1);
		testDrmaa.run();
		state = state && testDrmaa.getState();
		System.out.println();
		
		testDrmaa = new StInputFileFailure(this.exec1);
		testDrmaa.run();
		state = state && testDrmaa.getState();
		System.out.println();
		
		testDrmaa = new StOutputFileFailure(this.exec1);
		testDrmaa.run();
		state = state && testDrmaa.getState();
		System.out.println();
		
		testDrmaa = new StErrorFileFailure(this.exec1);
		testDrmaa.run();
		state = state && testDrmaa.getState();
		System.out.println();
		
		testDrmaa = new StSupportedAttr(this.exec1);
		testDrmaa.run();
		state = state && testDrmaa.getState();
		System.out.println();
		
		testDrmaa = new StVersion();
		testDrmaa.run();
		state = state && testDrmaa.getState();
		System.out.println();
		
		testDrmaa = new StContact();
		testDrmaa.run();
		state = state && testDrmaa.getState();
		System.out.println();
		
		testDrmaa = new StDRMSystem();
		testDrmaa.run();
		state = state && testDrmaa.getState();
		System.out.println();
		
		testDrmaa = new StDRMAAImpl();
		testDrmaa.run();
		state = state && testDrmaa.getState();
		System.out.println();
		
		testDrmaa = new StSubmitSuspendResumeWait(this.exec1);	
		testDrmaa.run();
		state = state && testDrmaa.getState();
		System.out.println();
		
		testDrmaa = new StEmptySessionWait();	
		testDrmaa.run();
		state = state && testDrmaa.getState();
		System.out.println();
		
		testDrmaa = new StEmptySessionSynchronizeDispose();	
		testDrmaa.run();
		state = state && testDrmaa.getState();
		System.out.println();
		
		testDrmaa = new StEmptySessionSynchronizeNoDispose();	
		testDrmaa.run();
		state = state && testDrmaa.getState();
		System.out.println();
		
		testDrmaa = new StEmptySessionControl();
		testDrmaa.run();
		state = state && testDrmaa.getState();
		System.out.println();
		
		testDrmaa = new StAttributeChange();	
		testDrmaa.run();
		state = state && testDrmaa.getState();
		System.out.println();
		
		testDrmaa = new StUsageCheck(this.exec1);	
		testDrmaa.run();
		state = state && testDrmaa.getState();
		System.out.println();
		
		testDrmaa = new MtSubmitWait(this.exec1);	
		testDrmaa.run();
		state = state && testDrmaa.getState();
		System.out.println();
		
		testDrmaa = new MtSubmitBeforeInitWait(this.exec1);	
		testDrmaa.run();
		state = state && testDrmaa.getState();
		System.out.println();
		
		testDrmaa = new MtExitDuringSubmit(this.exec1);			
		testDrmaa.run();
		state = state && testDrmaa.getState();
		System.out.println();
		
		testDrmaa = new MtSubmitMtWait(this.exec1);		
		testDrmaa.run();
		state = state && testDrmaa.getState();
		System.out.println();
		
		testDrmaa = new MtExitDuringSubmitOrWait(this.exec1);		
		testDrmaa.run();
		state = state && testDrmaa.getState();
		System.out.println();
		
		testDrmaa = new StSubmitInHoldRelease(this.exec1);		
		testDrmaa.run();
		state = state && testDrmaa.getState();
		System.out.println();
		
		testDrmaa = new StSubmitInHoldDelete(this.exec1);				
		testDrmaa.run();
		state = state && testDrmaa.getState();
		System.out.println();
		
		testDrmaa = new StBulkSubmitInHoldSessionRelease(this.exec1);
		testDrmaa.run();
		state = state && testDrmaa.getState();
		System.out.println();
		
		testDrmaa = new StBulkSubmitInHoldSingleRelease(this.exec1);
		testDrmaa.run();
		state = state && testDrmaa.getState();
		System.out.println();
		
		testDrmaa = new StBulkSubmitInHoldSessionDelete(this.exec1);		
		testDrmaa.run();
		state = state && testDrmaa.getState();
		System.out.println();
		
		testDrmaa = new StBulkSubmitInHoldSingleDelete(this.exec1);		
		testDrmaa.run();	
		state = state && testDrmaa.getState();
		System.out.println();
		
		if (state)
			System.out.println("All drmaa tests success");
	}
}
