#!/bin/bash

GRIDSAM_HOME=/opt/gridsam-2.3.0-client
if [ -z "${GW_LOCATION}" ]; then
    echo "Please, set GW_LOCATION variable."
    exit -1
fi

. $GW_LOCATION/bin/gw_mad_common.sh

setup_globus
cd_var
mad_debug
check_proxy

exec nice -n $PRIORITY java -classpath $GW_LOCATION/lib/gw_em_mad_bes.jar:/usr/share/java/xmlbeans.jar:/usr/share/java/jaxrpc.jar:$(GRIDSAM_HOME)/lib/gridsam-schema-2.3.0.jar:/usr/share/java/axis.jar:$(GRIDSAM_HOME)/lib/gridsam-core-2.3.0.jar:/usr/share/java/commons-logging.jar:/usr/share/java/commons-discovery.jar:/usr/share/java/wsdl4j.jar:$(GRIDSAM_HOME)/lib/omii-security-utils-1.3.jar:$(GRIDSAM_HOME)/lib/wss4j-1.5.0-itinnov-2.jar:$(GRIDSAM_HOME)/conf:/usr/share/java/xml-security.jar:/usr/share/java/dom4j.jar -Djava.endorsed.dirs=$(GRIDSAM_HOME)/endorsed -Daxis.ClientConfigFile=$(GRIDSAM_HOME)/conf/client-config.wsdd -Dlog4j.configuration=$(GRIDSAM_HOME)/conf/log4j.properties GW_em_mad_bes
