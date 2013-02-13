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
/* -------------------------------------------------------------------------  */

package org.ggf.drmaa;
/**  Not relevant for the current GridWay implementation, will be ignored.
 */
public class PartialTimestamp extends java.util.Calendar
{

	public static final int CENTURY=21;
	public static final int UNSET = 0;

	public int getModifier(int field)
	{
		return this.get(field);
	}

	public void setModifier(int field, int value)
	{
		this.set(field,value);
	}

	public  void add(int field, int amount){}

	protected  void computeFields(){}

	protected  void computeTime(){}

	public  int getGreatestMinimum(int field)
	{
		return 0;
	}

	public int getLeastMaximum(int field)
	{
		return 0;
	}

 	public int getMaximum(int field)
	{
		return 0;
	}

	public int getMinimum(int field)
	{
		return 0;
	}

	public void roll(int field, boolean up){}
}
