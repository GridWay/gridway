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

/**
 * Queue represents the information of a host queue. 
 * <p>
 * @author      Katia Leal (ASD)
 */

public class Queue 
{

	private String name;
	
	private String nodes;
	
	private String freeNodes;
	
	private String maxTime;
	
	private String maxCpuTime;

	private String maxCount;

	private String maxRunningJobs;
	
	private String maxJobsWoutCpu;
	
	private String status;
	
	private String dispatchType;

	private String priority;
	
	/**
     * Creates an empty Queue. 
    */
	public Queue(){
		name = "NULL";
		nodes = "0";
		freeNodes = "0";
		maxTime = "0";
		maxCpuTime = "0";
		maxCount = "0";
		maxRunningJobs = "0";
		maxJobsWoutCpu = "0";
		status = "NULL";
		dispatchType = "NULL";
		priority = "NULL";	
	}
	
	public String getName(){
		return name;
	}
	
	public void setName(String name){
		this.name = name;
	}
	
	public void setNode(String nodes){
		this.nodes = nodes;
	}
	
	public void setFreeNode(String freeNodes){
		this.freeNodes = freeNodes;
	}
	
	
	public void setMaxTime(String maxTime){
		this.maxTime = maxTime;
	}
	
	public void setMaxCpuTime(String maxCpuTime){
		this.maxCpuTime = maxCpuTime;
	}

	public void setMaxCount(String maxCount){
		this.maxCount = maxCount;
	}

	public void setMaxRunningJobs(String maxRunningJobs){
		this.maxRunningJobs = maxRunningJobs;
	}
	
	public void setMaxJobsWoutCpu(String maxJobsWoutCpu){
		this.maxJobsWoutCpu = maxJobsWoutCpu;
	}
	
	public void setStatus(String status){
		this.status = status;
	}
	
	public void setDispatchType(String dispatchType){
		this.dispatchType = dispatchType;
	}

	public void setPriority(String priority){
		this.priority = priority;
	}
	
	public int getNodes(){
		return (new Integer(nodes).intValue());
	}
	
	/**
	 * Returns an string with the information of the queue.
	 * @return      	 a String formatted.
	 */
	public String info(int i){
		StringBuffer out = new StringBuffer();
		out.append("QUEUE_NAME[" + i + "]=\"" + this.name + "\" QUEUE_NODECOUNT[" + i +"]=" + this.nodes + 
				" QUEUE_FREENODECOUNT[" + i + "]=" + this.freeNodes + " QUEUE_MAXTIME[" + i +"]=" + this.maxTime +
				" QUEUE_MAXCPUTIME[" + i + "]=" + this.maxCpuTime + " QUEUE_MAXCOUNT[" + i + "]=" + this.maxCount + 
				" QUEUE_MAXRUNNINGJOBS[" + i + "]=" + this.maxRunningJobs + " QUEUE_MAXJOBSINQUEUE[" + i + 
				"]=" + this.maxJobsWoutCpu + " QUEUE_STATUS[" + i + "]=\"" + this.status + "\" QUEUE_DISPATCHTYPE[" + i + "]=\"" + this.dispatchType +
				"\" QUEUE_PRIORITY[" + i +"]=\"" + this.priority + "\" ");
		return out.toString();
	}
}
