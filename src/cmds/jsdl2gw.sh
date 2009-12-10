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
# ---------------------------------------------------------------------------

if [ -z "${GW_LOCATION}" ]; then
    echo "Please, set GW_LOCATION variable."
    exit -1
fi

if [ "x$1" = "x-h" ]; then
  echo "USAGE"
  echo " jsdl2gw [-h] input_jsdl [output_gwjt]"
  echo
  echo "SYNOPSIS"
  echo "   Converts a jsdl document into a GridWay job template."
  echo "   If no output file is defined, it defaults to the standard output."
  echo "   This enables the use of pipes with gwsubmit in the following fashion:"
  echo
  echo "          jsdl2gw jsdl-job.xml | gwsubmit"
  echo
  echo "OPTIONS"
  echo "    -h           print this help"
  echo "    input_jsdl   read the jsdl document from the input_jsdl"
  echo "    output_gwjt  store the GridWay Job Template specification in the output_gwjt.jt file"
  exit 0
fi

export CLASSPATH=$GW_LOCATION/lib/gw_jsdl.jar:$CLASSPATH

java JSDLParser $1 $2
