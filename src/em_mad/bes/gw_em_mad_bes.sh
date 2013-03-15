#!/bin/bash

JAVA_EXT="${GW_LOCATION}/lib/java-ext/gridway-bes/"
#JAVA_EXT="<path_to_gridsam-2.3.0-client>"

# Help
while getopts t:h option
    do
        case $option in
        h)   echo -e "USAGE\n GW_em_mad_bes [-h]" \
                     "\n\nSYNOPSIS"\
                     "\n  Execution driver to interface with BES. It is not intended to be used from CLI."\
                     "\n\nOPTIONS"\
                     "\n  -h    print this help";
             exit 0;;
        [?]) echo -e "usage: GW_em_mad_bes [-h]";
             exit 1;;
        esac
    done

if [ -z "${GW_LOCATION}" ]; then
    echo "Please, set GW_LOCATION variable."
    exit -1
fi

CLASSPATH="${GW_LOCATION}/lib/gw_em_mad_bes.jar:`find ${JAVA_EXT}/lib /usr/share/java -name xbean.jar -o -name xmlbeans.jar | head -n 1`:`find /usr/share/java -name jaxrpc.jar`:${JAVA_EXT}/lib/gridsam-schema-2.3.0.jar:`find /usr/share/java -name axis.jar`:`find /usr/share/java -name commons-logging.jar`:`find /usr/share/java -name commons-discovery.jar`:`find /usr/share/java -name wsdl4j.jar`:${GW_LOCATION}/etc"
. $GW_LOCATION/bin/gw_mad_common.sh

setup_globus
mad_debug
check_proxy

cd $GW_LOCATION/etc
exec nice -n $PRIORITY java -classpath $CLASSPATH -Djava.endorsed.dirs=$JAVA_EXT/endorsed -Daxis.ClientConfigFile=$GW_LOCATION/etc/client-config.wsdd GW_em_mad_bes
