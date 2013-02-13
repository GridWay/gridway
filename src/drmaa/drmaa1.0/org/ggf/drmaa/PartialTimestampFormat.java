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

import java.util.*;


/**  Not relevant for the current GridWay implementation, will be ignored.
 */
public  class PartialTimestampFormat extends java.text.Format
{
	public PartialTimestampFormat()
	{
	    super();	
	}
	
	/**  Not relevant for the current GridWay implementation, will be ignored.
	 */
	public StringBuffer format(Object obj, StringBuffer stringBuffer, java.text.FieldPosition fieldPosition)
	{
		return null;
	}
	
	/**  Not relevant for the current GridWay implementation, will be ignored.
	 */
	public StringBuffer format(PartialTimestamp obj, StringBuffer stringBuffer, java.text.FieldPosition fieldPosition)
	{
		return null;
	}
	
	/** This method translate the PartialTimestamp instance in GridWay format.
	 *  The GridWay format is:
	 *
	 * [[DD:][HH:]]MM where, DD is the number of days HH is the number of hours MM 
	 *  is the number of minutes
	 *
	 * Example: 01:22 The job should finished one hour and 22 minutes after submission.
	 *
	 * @param obj The object to format 
 	 *
 	 */
	public String format(PartialTimestamp obj)
	{
		Calendar now = Calendar.getInstance();
		long 	  distance = obj.getTimeInMillis() - now.getTimeInMillis();
		
		return parseTimeInMillis(distance);
	}
	
	/**  Not relevant for the current GridWay implementation, will be ignored.
	 */
	public PartialTimestamp parse(String string) 
		throws java.text.ParseException
	{
		return null;
	}
	
	/**  Not relevant for the current GridWay implementation, will be ignored.
	 */
	public PartialTimestamp parse(String string, java.text.ParsePosition parsePosition) 
	{
		return null;
	}
	
	/**  Not relevant for the current GridWay implementation, will be ignored.
	 */
	public Object parseObject(String string, java.text.ParsePosition parsePosition)
	{
		return null;
	}
	
	private String parseTimeInMillis(long timeInMillis)
	{
		long seconds = timeInMillis / 1000;	
		int days			= new Long(seconds / 86400).intValue();
		int hours     	= new Long((seconds % 86400)/3600).intValue();
		int minutes  	= new Long(((seconds % 86400)%3600)/60).intValue();
		
		String daysString;
		String hoursString;
		String minutesString;
		
		if (days < 10)
			daysString = new String("0" + days);
		else
			daysString = new String("" + days);
		
		if (hours < 10)
			hoursString = new String("0" + hours);
		else
			hoursString = new String("" + hours);
		
		if (minutes < 10)
			minutesString = new String("0" + minutes);
		else
			minutesString = new String("" + minutes);
		
		return new String(daysString +":" + hoursString +":" + minutesString);
	}
}
