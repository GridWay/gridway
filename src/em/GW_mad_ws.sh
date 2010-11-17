#!/bin/sh

# -------------------------------------------------------------------------- 
# Copyright 2002-2010, GridWay Project Leads (GridWay.org)                   
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

JVM=${JAVA_HOME}/bin/java
if [ ! -x ${JVM} ]; then
    JVM=`which java 2>/dev/null`
    if [ "x$JVM" == "x" ]; then
        echo "Cannot find a JVM"
        exit 1
    fi
fi

##
## There should be no need to change anything below this line
##
cat<<EOF
MAD_WS GRAM client application 
(C) 2010 Members of the GridWay project

EOF
## Sanity check

if [ $# == 0 ]; then
    cat <<EOF
Usage: $0 [create | terminate | submit | status | recover | cancel] -p <port>

create     Invokes the GW_mad_ws with INIT operation
terminate  Invokes the GW_mad_ws with FINALIZE operation
submit     Invokes the GW_mad_ws with SUBMIT operation
status     Invokes the GW_mad_ws with POLL operation
recover    Invokes the GW_mad_ws with RECOVER operation
cancel     Invokes the GW_mad_ws with CANCEL operation

-p <port>  Invokes the operation on the specific port.
           e.g.: $0 create -p 8443

If you want to use Proxy based authentication, you should define the 
environment variable X509_USER_PROXY to the name of your proxy file. 

E.g. setenv X509_USER_PROXY /tmp/x509up_u999

EOF
    exit -1
fi

JAVA_CLASS="em_mad.GW_mad_ws"

case $1 in
    create)
	ARGUMENTS="INIT" 
        #JAVA_CLASS="org.glite.ce.creambes.client.CreateActivityClient" 
        shift
        ;;
    status) 
	ARGUMENTS="POLL"
        #JAVA_CLASS="org.glite.ce.creambes.client.GetActivityStatusClient" 
        shift
        ;;
    #attributes) 
        #JAVA_CLASS="org.glite.ce.creambes.client.GetFactoryAttributesClient" 
        #shift
        #;;
    terminate) 
	ARGUMENTS="FINALIZE"
        #JAVA_CLASS="org.glite.ce.creambes.client.TerminateActivitiesClient" 
        shift
        ;;
    #documents)
        #JAVA_CLASS="org.glite.ce.creambes.client.GetActivityDocumentsClient"
        #shift
        #;;
    submit)
	ARGUMENTS="SUBMIT"
	shift
	;;
    recover)
	ARGUMENTS="RECOVER"
	shift
	;;
    cancel)
	ARGUMENTS="CANCEL"
	shift
	;;
    *) 
        echo "Syntax error. Type $0 to get help."
        exit -1
        ;;
esac

AXIS_HOME=${OMII_WORKAREA}/repository/externals/axis/1.4/noarch

CP="$GLOBUS_LOCATION/lib/common/axis.jar:$GLOBUS_LOCATION/lib/common/globus-addressing-1.0.jar:$GLOBUS_LOCATION/lib/common/wsrf_core.jar:$GLOBUS_LOCATION/lib/common/cog-jglobus.jar:$GLOBUS_LOCATION/lib/globus_delegation_service.jar:$GLOBUS_LOCATION/lib/globus_wsrf_gram_client.jar:$GLOBUS_LOCATION/lib/globus_wsrf_gram_utils.jar:$GLOBUS_LOCATION/lib/globus_wsrf_gram_common.jar:$JAVA_HOME/jre/lib/rt.jar"


if [ "x$X509_USER_PROXY" == "x" ]; then
    echo "Using X509 certificate."
    JAVA_OPTS="-Daxis.socketSecureFactory=org.glite.security.trustmanager.axis.AXISSocketFactory -DsslCertFile=$HOME/.globus/usercert.pem -DsslKey=$HOME/.globus/userkey_free.pem"
else
    echo "Using VOMS proxy ${X509_USER_PROXY}."
    JAVA_OPTS="-Daxis.socketSecureFactory=org.glite.security.trustmanager.axis.AXISSocketFactory -DgridProxyFile=${X509_USER_PROXY}"
fi
## Disable fetching of CRL
JAVA_OPTS="$JAVA_OPTS -DcrlEnabled=false -DcrlRequired=false -DsslCAFiles=/etc/grid-security/certificates/*.0"
## Fix the classpath
for item in `ls ${AXIS_HOME}/lib/*.jar`; do
    CP=$CP:$item
done

#JAVA_OPTS="$JAVA_OPTS -Dlog4j.configuration=${OMII_WORKAREA}/cream-bes/log4j.properties"

${JVM} $JAVA_OPTS -classpath $CP $JAVA_CLASS $ARGUMENTS
