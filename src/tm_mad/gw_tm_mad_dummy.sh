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

. $GLOBUS_LOCATION/etc/globus-user-env.sh

nice -n $PRIORITY $GW_LOCATION/bin/gw_tm_mad_dummy.bin $*
