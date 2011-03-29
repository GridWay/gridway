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

public class StDRMSystem extends StContactDRMDRMAA
{
	public StDRMSystem()
	{
		super(ST_DRM_SYSTEM);
		this.method = "getDrmsInfo()";
	}


	public void run()
	{
		this.value = this.session.getDrmsInfo();
		this.runInit();
		this.value = this.session.getDrmsInfo();
		this.runAfterInit();
		System.out.println("Succesfully finished test "+ this.type);
	}
}
