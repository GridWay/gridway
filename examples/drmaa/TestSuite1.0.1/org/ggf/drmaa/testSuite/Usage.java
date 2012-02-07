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

public class Usage 
{
	public void message()
	{
		System.out.println("usage: test_drmaa <test_case>");
		System.out.println("\t<test_case> is one of the keywords below including the enlisted test case arguments");
		System.out.println("\t\tALL_TEST\t\t\t\t\t<sleep_job>");
		System.out.println("\t\tST_MULT_INIT");
		System.out.println("\t\tST_MULT_EXIT");
		System.out.println("\t\tST_SUBMIT_WAIT\t\t\t\t\t<sleep_job>");
		System.out.println("\t\tST_SUBMIT_POLLING_WAIT_TIMEOUT\t\t\t<sleep_job>");
		System.out.println("\t\tST_SUBMIT_POLLING_WAIT_ZEROTIMEOUT\t\t<sleep_job>");
		System.out.println("\t\tST_SUBMIT_POLLING_SYNCHRONIZE_TIMEOUT\t\t<sleep_job>");
		System.out.println("\t\tST_SUBMIT_POLLING_SYNCHRONIZE_ZEROTIMEOUT\t<sleep_job>");
		System.out.println("\t\tST_BULK_SUBMIT_WAIT\t\t\t\t<sleep_job>");
		System.out.println("\t\tST_BULK_SINGLESUBMIT_WAIT_INDIVIDUAL\t\t<sleep_job>");
		System.out.println("\t\tST_SUBMITMIXTURE_SYNC_ALL_DISPOSE\t\t<sleep_job>");
		System.out.println("\t\tST_SUBMITMIXTURE_SYNC_ALL_NODISPOSE\t\t<sleep_job>");
		System.out.println("\t\tST_SUBMITMIXTURE_SYNC_ALLIDS_DISPOSE\t\t<sleep_job>");
		System.out.println("\t\tST_SUBMITMIXTURE_SYNC_ALLIDS_NODISPOSE\t\t<sleep_job>");
		System.out.println("\t\tST_INPUT_FILE_FAILURE\t\t\t\t<sleep_job>");
		System.out.println("\t\tST_OUTPUT_FILE_FAILURE\t\t\t\t<sleep_job>");
		System.out.println("\t\tST_ERROR_FILE_FAILURE\t\t\t\t<sleep_job>");
		System.out.println("\t\tST_SUPPORTED_ATTR\t\t\t\t<sleep_job>");
		System.out.println("\t\tST_VERSION");
		System.out.println("\t\tST_DRM_SYSTEM");
		System.out.println("\t\tST_DRMAA_IMPL");
		System.out.println("\t\tST_CONTACT");
		System.out.println("\t\tST_SUBMIT_SUSPEND_RESUME_WAIT\t\t\t<sleep_job>");
		System.out.println("\t\tST_EMPTY_SESSION_WAIT");
		System.out.println("\t\tST_EMPTY_SESSION_SYNCHRONIZE_DISPOSE");
		System.out.println("\t\tST_EMPTY_SESSION_SYNCHRONIZE_NODISPOSE");
		System.out.println("\t\tST_EMPTY_SESSION_CONTROL");
		System.out.println("\t\tST_ATTRIBUTE_CHANGE");
		System.out.println("\t\tST_USAGE_CHECK\t\t\t\t\t<sleep_job>");
		System.out.println("\t\tMT_SUBMIT_WAIT\t\t\t\t\t<sleep_job>");
		System.out.println("\t\tMT_SUBMIT_BEFORE_INIT_WAIT\t\t\t<sleep_job>");
		System.out.println("\t\tMT_EXIT_DURING_SUBMIT\t\t\t\t<sleep_job>");
		System.out.println("\t\tMT_SUBMIT_MT_WAIT\t\t\t\t<sleep_job>");
		System.out.println("\t\tMT_EXIT_DURING_SUBMIT_OR_WAIT\t\t\t<sleep_job>");
		System.out.println("\t\tST_SUBMIT_IN_HOLD_RELEASE\t\t\t<sleep_job>");
		System.out.println("\t\tST_SUBMIT_IN_HOLD_DELETE\t\t\t<sleep_job>");
		System.out.println("\t\tST_BULK_SUBMIT_IN_HOLD_SESSION_RELEASE\t\t<sleep_job>");
		System.out.println("\t\tST_BULK_SUBMIT_IN_HOLD_SINGLE_RELEASE\t\t<sleep_job>");
		System.out.println("\t\tST_BULK_SUBMIT_IN_HOLD_SESSION_DELETE\t\t<sleep_job>");
		System.out.println("\t\tST_BULK_SUBMIT_IN_HOLD_SINGLE_DELETE\t\t<sleep_job>"); 
	}
	
	public void messageAllTest()
	{
		System.out.println("usage: test_drmaa ALL_TEST <sleep_job> <kill_job> <exit_job>");
	}
	
}
