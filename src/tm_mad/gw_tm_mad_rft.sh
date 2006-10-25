#!/bin/bash

PRIORITY=19
MADDEBUG=

if [ -z "${GLOBUS_LOCATION}" ]; then
    echo "Please, set GLOBUS_LOCATION variable."
    exit -1
fi

if [ -z "${GW_LOCATION}" ]; then
    echo "Please, set GW_LOCATION variable."
    exit -1
fi

cd ${GW_LOCATION}/var/

if [ -n "${MADDEBUG}" ]; then
    ulimit -c 15000
fi

. $GLOBUS_LOCATION/etc/globus-devel-env.sh 

export CLASSPATH=$CLASSPATH:$GW_LOCATION/bin

if [ -z "${GLOBUS_TCP_PORT_RANGE}" ]; then
    nice -n $PRIORITY java -DGLOBUS_LOCATION=$GLOBUS_LOCATION -Djava.endorsed.dirs=$GLOBUS_LOCATION/endorsed GWRftProcessor
else
    nice -n $PRIORITY java -DGLOBUS_LOCATION=$GLOBUS_LOCATION -Djava.endorsed.dirs=$GLOBUS_LOCATION/endorsed -DGLOBUS_TCP_PORT_RANGE=$GLOBUS_TCP_PORT_RANGE GWRftProcessor
fi
