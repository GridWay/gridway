#!/bin/bash

if [ -z "${GW_LOCATION}" ]; then
    echo "Please, set GW_LOCATION variable."
    exit -1
fi

. $GW_LOCATION/bin/gw_mad_common.sh

setup_globus
cd_var
mad_debug
check_proxy

if [ -z "${GLOBUS_TCP_PORT_RANGE}" ]; then
    exec nice -n $PRIORITY java -DGLOBUS_LOCATION=$GLOBUS_LOCATION -DX509_USER_PROXY=$X509_USER_PROXY -Djava.endorsed.dirs=$GLOBUS_LOCATION/endorsed -classpath $CLASSPATH:$GW_LOCATION/lib/gw_em_mad_mds4.jar GW_mad_mds4 $*
else
    exec nice -n $PRIORITY java -DGLOBUS_LOCATION=$GLOBUS_LOCATION -DX509_USER_PROXY=$X509_USER_PROXY -Djava.endorsed.dirs=$GLOBUS_LOCATION/endorsed -DGLOBUS_TCP_PORT_RANGE=$GLOBUS_TCP_PORT_RANGE -classpath $CLASSPATH:$GW_LOCATION/lib/gw_em_mad_mds4.jar GW_mad_mds4 $*
fi
