#!/bin/bash

export JAR_FILES="/usr/share/java"
export GRIDSAM_HOME="/var/lib/tomcat6/webapps/gridsam"

if [ -z "${GW_LOCATION}" ]; then
    echo "Please, set GW_LOCATION variable."
    exit -1
fi

. $GW_LOCATION/bin/gw_mad_common.sh

setup_globus
cd_var
mad_debug
check_proxy

exec nice -n $PRIORITY java -classpath $GW_LOCATION/lib/gw_em_mad_bes.jar:$JAR_FILES/axis.jar:$JAR_FILES/jaxrpc.jar:$GRIDSAM_HOME/WEB-INF/lib/gridsam-schema-2.3.0.jar:$JAR_FILES/xmlbeans.jar:$JAR_FILES/commons-logging-1.0.4.jar:$JAR_FILES/commons-discovery.jar:$GRIDSAM_HOME/WEB-INF/lib/gridsam-core-2.3.0.jar:$JAR_FILES/wsdl4j.jar:$JAR_FILES/commons-httpclient.jar:$JAR_FILES/commons-codec.jar:$JAR_FILES/dom4j.jar -Djava.endorsed.dirs=$GW_LOCATION/lib/endorsed -Dlog4j.configuration="file://$GRIDSAM_HOME/WEB-INF/classes/log4j.properties" GW_em_mad_bes 
