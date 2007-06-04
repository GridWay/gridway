#!/bin/bash

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
    exec nice -n $PRIORITY java -DGLOBUS_LOCATION=$GLOBUS_LOCATION -Djava.endorsed.dirs=$GLOBUS_LOCATION/endorsed GWRftProcessor
else
    exec nice -n $PRIORITY java -DGLOBUS_LOCATION=$GLOBUS_LOCATION -Djava.endorsed.dirs=$GLOBUS_LOCATION/endorsed -DGLOBUS_TCP_PORT_RANGE=$GLOBUS_TCP_PORT_RANGE GWRftProcessor
fi
