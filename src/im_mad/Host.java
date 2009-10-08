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

/**
 * Host contains among others, information about the cpu, the memory and the queues 
 * of a host. 
 * <p>
 * @author      Katia Leal (ASD)
 */

import java.util.*;

public class Host 
{
    private String name;
    
    private String arch;

    private String os;

    private String osVersion;

    private String cpuModel;

    private String cpuMhz;

    private String freeCpu;

    private String cpuSmp;

    private String nodeCount;
    
    private String sizeMemMB;

    private String freeMemMB;

    private String sizeDiskMB;

    private String freeDiskMB;
    
    private String forkName;

    private String lrmsName;

    private String lrmsType;
    
    private String staticInfo;

    private Vector queues;
    
    private boolean upToDate;
    
    /**
     * Creates an empty Host. 
     */
    public Host(String staticInfo){
        name = "NULL";
        arch = "NULL";
        os = "NULL";
        osVersion = "NULL";
        cpuModel = "NULL";
        cpuMhz = "0";
        freeCpu = "0";
        cpuSmp = "0";
        nodeCount = "0";
        sizeMemMB ="0";
        freeMemMB = "0";
        sizeDiskMB = "0";
        freeDiskMB = "0";
        forkName = "NULL";
        lrmsName = "NULL";
        lrmsType = "NULL";
        queues = new Vector();
        upToDate = false;
        this.staticInfo = staticInfo;
    }
    
    public void add(Queue q){
        queues.add(q);
    }
    
    public void removeQueues(){
        queues.removeAllElements();
    }
    
    public void setName(String name){
        this.name = name;
    }
    
    public void setArch(String arch){
        this.arch = arch;
    }

    public String getArch(){
        return this.arch;
    }


    public void setOs(String os){
        this.os = os;
    }

    public void setOsVersion(String osVersion){
        this.osVersion = osVersion; 
    }

    public void setCpuModel(String cpuModel){
        this.cpuModel = cpuModel;
    }

    public void setCpuMhz(String cpuMhz){
        this.cpuMhz = cpuMhz;
    }

    public void setFreeCpu(String freeCpu){
        this.freeCpu = freeCpu;
    }

    public void setCpuSmp(String cpuSmp){
        this.cpuSmp = cpuSmp;
    }
    
    public String getCpuSmp(){
        return cpuSmp;
    }
    
    public void setNodeCount(String nodeCount){
        this.nodeCount = nodeCount;
    }
    
    public void setSizeMemMB(String sizeMemMB){
        this.sizeMemMB = sizeMemMB;
    }

    public void setFreeMemMB(String freeMemMB){
        this.freeMemMB = freeMemMB;
    }

    public void setSizeDiskMB(String sizeDiskMB){
        this.sizeDiskMB = sizeDiskMB;
    }

    public void setFreeDiskMB(String freeDiskMB){
        this.freeDiskMB = freeDiskMB;
    }
    
    public void setForkName(String value){
        forkName = value;
    }
    
    public String getForkName(){
        return forkName;
    }
    
    public void setLrmsName(String value){
        lrmsName = value;
    }

    public void setLrmsType(String value){
        lrmsType = value;
    }
    
    public String getLrmsName(){
        return lrmsName;
    }
    
    public int getNodeCount(){
        return (new Integer(nodeCount).intValue());
    }
    
    /**
     * Reset host information
     */
    public void reset(){
        name = "NULL";
        arch = "NULL";
        os = "NULL";
        osVersion = "NULL";
        cpuModel = "NULL";
        cpuMhz = "0";
        freeCpu = "0";
        cpuSmp = "0";
        nodeCount = "0";
        sizeMemMB ="0";
        freeMemMB = "0";
        sizeDiskMB = "0";
        freeDiskMB = "0";
        forkName = "NULL";
        //lrmsName = "NULL";
        //lrmsType = "NULL";
        //queues = new Vector();
        upToDate = false;
    }
    
    /**
     * Returns an string with the information of the host.
     * @return           a String formatted.
     */
    public String info(){
        StringBuffer out = new StringBuffer();
        out.append("HOSTNAME=\"" + this.name + "\" ARCH=\"" + this.arch + "\" OS_NAME=\"" + this.os + 
                "\" OS_VERSION=\"" + this.osVersion + "\" CPU_MODEL=\"" + this.cpuModel + 
                "\" CPU_MHZ=" + this.cpuMhz + " CPU_FREE=" + this.freeCpu + " CPU_SMP=" + this.cpuSmp +
                " NODECOUNT=" + this.nodeCount + " SIZE_MEM_MB=" + this.sizeMemMB + 
                " FREE_MEM_MB=" + this.freeMemMB + " SIZE_DISK_MB=" + this.sizeDiskMB + 
                " FREE_DISK_MB=" + this.freeDiskMB + " FORK_NAME=\"" + this.forkName +
                "\" LRMS_NAME=\"" + this.lrmsName + "\" LRMS_TYPE=\"" + this.lrmsType + "\" ");
        if (staticInfo != "")
        	out.append( staticInfo + " ");
        
        out.append(queuesInfo());
        return out.toString();
    }
    
    /**
     * Returns an string with the information of the host queues.
     * @return           a String formatted.
     */
    private String queuesInfo(){
        StringBuffer out = new StringBuffer();
        Iterator it = queues.iterator();
        int i = 0;
        
        while(it.hasNext()){
            Queue q = (Queue)it.next();
            out.append(q.info(i));
            i++;
        }
        return out.toString();
    } 
    
    /**
     * Determines whether the host information is up to date or not.
     */
    public boolean isUpToDate(){
        return upToDate;
    }   
    
    public void setUpToDate(boolean value){
        upToDate = value; 
    }  
}
