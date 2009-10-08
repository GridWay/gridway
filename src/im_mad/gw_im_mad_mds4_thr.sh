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

# Common initialization
if [ -z "${GW_LOCATION}" ]; then
    echo "Please, set GW_LOCATION variable."
    exit -1
fi

. $GW_LOCATION/bin/gw_mad_common.sh

setup_globus
cd_var
mad_debug

if [ -z "${GLOBUS_TCP_PORT_RANGE}" ]; then
    exec nice -n $PRIORITY java -DGW_LOCATION=$GW_LOCATION -DGLOBUS_LOCATION=$GLOBUS_LOCATION -DX509_USER_PROXY=$X509_USER_PROXY -Djava.endorsed.dirs=$GLOBUS_LOCATION/endorsed -classpath $GW_LOCATION/lib/gw_im_mad_ws.jar:$CLASSPATH GW_im_mad_mds4 $*
else
    exec nice -n $PRIORITY java -DGW_LOCATION=$GW_LOCATION -DGLOBUS_LOCATION=$GLOBUS_LOCATION -DX509_USER_PROXY=$X509_USER_PROXY -Djava.endorsed.dirs=$GLOBUS_LOCATION/endorsed -DGLOBUS_TCP_PORT_RANGE=$GLOBUS_TCP_PORT_RANGE -classpath $GW_LOCATION/lib/gw_im_mad_ws.jar:$CLASSPATH GW_im_mad_mds4 $*
fi

