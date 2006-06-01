#!/bin/sh

ulimit -c 15000
cd /tmp

if [ -z "${GLOBUS_LOCATION}" ]; then
    echo "Please, set GLOBUS_LOCATION variable."
    exit -1
fi

if [ -z "${GW_LOCATION}" ]; then
    echo "Please, set GW_LOCATION variable."
    exit -1
fi

. $GLOBUS_LOCATION/etc/globus-user-env.sh

$GW_LOCATION/bin/gw_em_mad_prews.bin $*
