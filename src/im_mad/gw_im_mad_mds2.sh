#!/bin/bash

# -------------------------------------------------------------------------- 
# Copyright 2002-2006 GridWay Team, Distributed Systems Architecture         
# Group, Universidad Complutense de Madrid                                   
#                                                                            
# Licensed under the Apache License, Version 2.0 (the "License"); you may    
# not use this file except in compliance with the License. You may obtain    
# a copy of the License at                                                   
#                                                                            
# http://www.apache.org/licenses/LICENSE-2.0                                 
#                                                                            
# Unless required by applicable law or agreed to in writing, software        
# distributed under the License is distributed on an "AS IS" BASIS,          
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   
# See the License for the specific language governing permissions and        
# limitations under the License.                                             
# -------------------------------------------------------------------------- 

dynamic_discover (){
    if [ -n "$SERVER" ]
    then
        TMPFILE=".search.$$.$RANDOM"
        ERRFILE=".error.$$.$RANDOM"

        grid-info-search -x -LLL -nowrap -h $SERVER -b $BASE \
                "(&(objectclass=MdsHost)$HOSTFILTER)" Mds-Host-hn \
                > $TMPFILE 2>$ERRFILE
                
        if [ $? -eq 0 ]
        then
            HOSTNAMES=`grep Mds-Host-hn: $TMPFILE | awk '{print $2}'`

            while read HOSTNAME
            do
                INFO="$INFO $HOSTNAME"
            done < $HOSTNAMES

            echo "DISCOVER - SUCCESS $INFO"
        else
        	INFO=`cat $ERRFILE`
            echo "DISCOVER - FAILURE $INFO"
        fi
        
        rm -f $TMPFILE $ERRFILE
    else
        echo "DISCOVER - SUCCESS $INFO"
    fi
}

dynamic_monitor (){
    TMPFILE=".search.$$.$1.$RANDOM"
	ERRFILE=".error.$$.$1.$RANDOM"
	
    grid-info-search -x -LLL -h $2 > $TMPFILE 2> $ERRFILE
        
    if [ $? -eq 0 ]
    then
        HOSTNAME=`grep Mds-Host-hn: $TMPFILE | awk -F": " '{print $2}' | head -1`
        ARCH=`grep Mds-Computer-isa: $TMPFILE | awk -F": " '{print $2}' | head -1`
        OS_NAME=`grep Mds-Os-name: $TMPFILE | awk -F": " '{print $2}' | head -1`
        OS_VERSION=`grep Mds-Os-release: $TMPFILE | awk -F": " '{print $2}' | head -1`
        CPU_MODEL=`grep Mds-Cpu-model: $TMPFILE | awk -F": " '{print $2}' | head -1`
        CPU_MHZ=`grep Mds-Cpu-speedMHz: $TMPFILE | awk -F": " '{print $2}' | head -1`
        CPU_FREE=`grep Mds-Cpu-Total-Free-1minX100: $TMPFILE | awk -F": " '{print $2}' | tail -1`
        CPU_SMP=`grep Mds-Cpu-Smp-size: $TMPFILE | awk -F": " '{print $2}' | tail -1`
        NODECOUNT=`grep Mds-Computer-Total-nodeCount: $TMPFILE | awk -F": " '{print $2}' | tail -1`
        FREE_MEM_MB=`grep Mds-Memory-Ram-Total-freeMB: $TMPFILE | awk -F": " '{print $2}' | head -1`
        SIZE_MEM_MB=`grep Mds-Memory-Ram-Total-sizeMB: $TMPFILE | awk -F": " '{print $2}' | head -1`
        FREE_DISK_MB=`grep Mds-Fs-Total-freeMB: $TMPFILE | awk -F": " '{print $2}' | head -1`
        SIZE_DISK_MB=`grep Mds-Fs-Total-sizeMB: $TMPFILE | awk -F": " '{print $2}' | head -1`
        FORK_NAME=`grep "Mds-Software-deployment: jobmanager" $TMPFILE | awk -F": " '{print $2}' | head -1`
        LRMS_NAME=`grep "Mds-Software-deployment: jobmanager" $TMPFILE | awk -F": " '{print $2}' | tail -1`
        LRMS_TYPE=`grep Mds-Service-Gram-schedulertype: $TMPFILE | awk -F": " '{print $2}' | tail -1`

        grid-info-search -x -LLL -h $2 "(&(objectclass=MdsJobQueue)$QUEUEFILTER)"> $TMPFILE 2> $ERRFILE
    
        if [ $? -eq 0 ]
        then
            QUEUE_NAME=(`grep Mds-Job-Queue-name: $TMPFILE | awk -F": " '{print $2}' | awk -F"@" '{print $1}'`)
            QUEUE_NODECOUNT=(`grep Mds-Computer-Total-nodeCount: $TMPFILE | awk -F": " '{print $2}' | tail +1`)
            QUEUE_FREENODECOUNT=(`grep Mds-Computer-Total-Free-nodeCount: $TMPFILE | awk -F": " '{print $2}'`)
            QUEUE_MAXTIME=(`grep Mds-Gram-Job-Queue-maxtime: $TMPFILE | awk -F": " '{print $2}'`)
            QUEUE_MAXCPUTIME=(`grep Mds-Gram-Job-Queue-maxcputime: $TMPFILE | awk -F": " '{print $2}'`)
            QUEUE_MAXCOUNT=(`grep Mds-Gram-Job-Queue-maxcount: $TMPFILE | awk -F": " '{print $2}'`)
            QUEUE_MAXRUNNINGJOBS=(`grep Mds-Gram-Job-Queue-maxrunningjobs: $TMPFILE | awk -F": " '{print $2}'`)
            QUEUE_MAXJOBSINQUEUE=(`grep Mds-Gram-Job-Queue-maxjobsinqueue: $TMPFILE | awk -F": " '{print $2}'`)
            QUEUE_STATUS=(`grep Mds-Gram-Job-Queue-status: $TMPFILE | awk -F": " '{print $2}'`)
            QUEUE_DISPATCHTYPE=(`grep Mds-Gram-Job-Queue-dispatchtype: $TMPFILE | awk -F": " '{print $2}'`)
            QUEUE_PRIORITY=(`grep Mds-Gram-Job-Queue-priority: $TMPFILE | awk -F": " '{print $2}'`)
            QUEUE_JOBWAIT=(`grep Mds-Gram-Job-Queue-jobwait: $TMPFILE | awk -F": " '{print $2}'`)

	        INFO=`echo "HOSTNAME=\"$HOSTNAME\" ARCH=\"$ARCH\"" \
                "OS_NAME=\"$OS_NAME\" OS_VERSION=\"$OS_VERSION\"" \
                "CPU_MODEL=\"$CPU_MODEL\" CPU_MHZ=$CPU_MHZ CPU_FREE=$CPU_FREE CPU_SMP=$CPU_SMP" \
                "NODECOUNT=$NODECOUNT" \
                "SIZE_MEM_MB=$SIZE_MEM_MB FREE_MEM_MB=$FREE_MEM_MB" \
                "SIZE_DISK_MB=$SIZE_DISK_MB FREE_DISK_MB=$FREE_DISK_MB" \
                "FORK_NAME=\"$FORK_NAME\"" \
                "LRMS_NAME=\"$LRMS_NAME\" LRMS_TYPE=\"$LRMS_TYPE\""`

    	    if [ "$LRMS_TYPE" = "fork" ]
	        then
	            i=0;
	        else
	            i=1;
	        fi

	        for ((j=0; i<${#QUEUE_NAME[@]}; i++,j++))
	        do
	            INFO=`echo "$INFO QUEUE_NAME[$j]=\"${QUEUE_NAME[$i]}\" QUEUE_NODECOUNT[$j]=${QUEUE_NODECOUNT[$i]}" \
                    "QUEUE_FREENODECOUNT[$j]=${QUEUE_FREENODECOUNT[$i]} QUEUE_MAXTIME[$j]=${QUEUE_MAXTIME[$i]}" \
                    "QUEUE_MAXCPUTIME[$j]=${QUEUE_MAXCPUTIME[$i]} QUEUE_MAXCOUNT[$j]=${QUEUE_MAXCOUNT[$i]}" \
                    "QUEUE_MAXRUNNINGJOBS[$j]=${QUEUE_MAXRUNNINGJOBS[$i]} QUEUE_MAXJOBSINQUEUE[$j]=${QUEUE_MAXJOBSINQUEUE[$i]}" \
                    "QUEUE_DISPATCHTYPE[$j]=\"${QUEUE_DISPATCHTYPE[$i]}\" QUEUE_PRIORITY[$j]=\"${QUEUE_PRIORITY[$i]}\"" \
                    "QUEUE_STATUS[$j]=\"${QUEUE_STATUS[$i]}\""`
    	    done

        	echo "MONITOR $1 SUCCESS $INFO"
		else
	    	INFO=`cat $ERRFILE`
	        echo "MONITOR $1 FAILURE $INFO"	
        fi		
    else
    	INFO=`cat $ERRFILE`
        echo "MONITOR $1 FAILURE $INFO"
    fi

    rm -f $TMPFILE $ERRFILE
}

BASE="mds-vo-name=local,o=grid"
HOSTFILTER=""
QUEUEFILTER=""

. gw_im_mad_common.sh
