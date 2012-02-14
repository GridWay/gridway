#!/bin/bash

GRIDSAM_HOME="/usr/share/gridsam-2.3.0-client"
if [ -z "${GW_LOCATION}" ]; then
    echo "Please, set GW_LOCATION variable."
    exit -1
fi
CLASSPATH="${GW_LOCATION}/lib/gw_em_mad_bes.jar:`find /usr/share/java -name xbean.jar -o -name xmlbeans.jar`:`find /usr/share/java -name jaxrpc.jar`:${GRIDSAM_HOME}/lib/gridsam-schema-2.3.0.jar:`find /usr/share/java -name axis.jar`:${GRIDSAM_HOME}/lib/gridsam-core-2.3.0.jar:`find /usr/share/java -name commons-logging.jar`:`find /usr/share/java -name commons-discovery.jar`:`find /usr/share/java -name wsdl4j.jar`:${GRIDSAM_HOME}/lib/omii-security-utils-1.3.jar:${GRIDSAM_HOME}/lib/wss4j-1.5.0-itinnov-2.jar:${GRIDSAM_HOME}/conf:`find /usr/share/java -name xml-security.jar`:`find /usr/share/java -name dom4j.jar`:`find /usr/share/java -name log4j.jar`"
. $GW_LOCATION/bin/gw_mad_common.sh

setup_globus
cd_var
mad_debug
check_proxy

exec nice -n $PRIORITY java -classpath $CLASSPATH -Djava.endorsed.dirs=$GRIDSAM_HOME/endorsed -Daxis.ClientConfigFile=$GRIDSAM_HOME/conf/client-config.wsdd -Dlog4j.configuration=$GRIDSAM_HOME/conf/log4j.properties GW_em_mad_bes
