#!/bin/bash

## RPM/DEB installation
#JAVA_EXT="${GW_LOCATION}/lib/java-ext/gridway-bes/"

## Usual installation
JAVA_EXT="/usr/share/gridsam-2.3.0-client/"

if [ -z "${GW_LOCATION}" ]; then
    echo "Please, set GW_LOCATION variable."
    exit -1
fi

CLASSPATH="${GW_LOCATION}/lib/gw_em_mad_bes.jar:`find ${JAVA_EXT}/lib /usr/share/java -name xbean.jar -o -name xmlbeans.jar | head -n 1`:`find /usr/share/java -name jaxrpc.jar`:${JAVA_EXT}/lib/gridsam-schema-2.3.0.jar:`find /usr/share/java -name axis.jar`:`find /usr/share/java -name commons-logging.jar`:`find /usr/share/java -name commons-discovery.jar`:`find /usr/share/java -name wsdl4j.jar`:${JAVA_EXT}/lib/omii-security-utils-1.3.jar:`find ${JAVA_EXT}/lib /usr/share/java -name wss4j.jar | head -n 1`:`find ${JAVA_EXT}/lib /usr/share/java -name xml-security.jar -o -name xmlsec.jar | head -n 1`:`find /usr/share/java -name log4j.jar`:${GW_LOCATION}/etc"
. $GW_LOCATION/bin/gw_mad_common.sh

setup_globus
cd_var
mad_debug
check_proxy

exec nice -n $PRIORITY java -classpath $CLASSPATH -Djava.endorsed.dirs=$JAVA_EXT/endorsed -Daxis.ClientConfigFile=$GW_LOCATION/etc/client-config.wsdd GW_em_mad_bes
