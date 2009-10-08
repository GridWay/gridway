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

		if [ "x$PORTNUMBER" = "x" ]
		then
			INDEX="https://$SERVER/wsrf/services/DefaultIndexService"
		else
			INDEX="https://$SERVER:$PORTNUMBER/wsrf/services/DefaultIndexService"		
		fi        
        
        XPATH='//*[local-name()="MemberServiceEPR"]/*[local-name()="Address" and substring-after(text(),"/wsrf/services/")="DefaultIndexService"]'

        echo "<root>" > $TMPFILE
        nice -n $PRIORITY wsrf-query -a -z none -s "$INDEX" "$XPATH" >> $TMPFILE
        STATUS=$?
        echo "</root>" >> $TMPFILE


        if [ $STATUS -eq 0 ]
        then
            HOSTNAMES=`nice -n $PRIORITY java -classpath $CLASSPATH:$GW_LOCATION/lib/gw_im_mad_ws.jar Mds4QueryParser -l $TMPFILE`

            for HOSTNAME in $HOSTNAMES
            do
                INFO="$INFO $HOSTNAME"
            done
            
            echo "DISCOVER - SUCCESS $SERVER $INFO"
        else
            echo "DISCOVER - FAILURE Can't access $SERVER"
        fi
        
        rm -f $TMPFILE
    else
        echo "DISCOVER - SUCCESS $INFO"
    fi
}

dynamic_monitor (){
	   
    TMPFILE=".search.$$.$1.$RANDOM"

	if [ "x$PORTNUMBER" = "x" ]
	then
	    INDEX="https://$2/wsrf/services/DefaultIndexService"
	else
	    INDEX="https://$2:$PORTNUMBER/wsrf/services/DefaultIndexService"	
	fi
    
    XPATH='/*/*/*[local-name()="MemberServiceEPR"]/*[local-name()="Address" and substring-after(text(),"/wsrf/services/")="ManagedJobFactoryService"]/../..'

    echo "<root>" > $TMPFILE
    nice -n $PRIORITY wsrf-query -a -z none -s "$INDEX" "$XPATH" >> $TMPFILE
    STATUS=$?
    echo "</root>" >> $TMPFILE

    if [ $STATUS -eq 0 ]
    then
        INFO=`nice -n $PRIORITY java -classpath $CLASSPATH:$GW_LOCATION/lib/gw_im_mad_ws.jar Mds4QueryParser -i $2 $TMPFILE "$3"`
        echo "MONITOR $1 SUCCESS $INFO"
    else
        echo "MONITOR $1 FAILURE Can't access $2"
    fi
    
    rm -f $TMPFILE
}

# Common initialization
if [ -z "${GW_LOCATION}" ]; then
    echo "Please, set GW_LOCATION variable."
    exit -1
fi

. $GW_LOCATION/bin/gw_mad_common.sh

setup_globus

. $GW_LOCATION/bin/gw_im_mad_common.sh
