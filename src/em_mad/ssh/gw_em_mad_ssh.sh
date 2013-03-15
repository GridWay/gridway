#!/bin/bash

if [ -z "${GW_LOCATION}" ]; then
    echo "Please, set GW_LOCATION variable."
    exit -1
fi

. $GW_LOCATION/bin/gw_mad_common.sh

setup_globus
mad_debug

ruby $GW_LOCATION/libexec/ruby/gw_em_mad_ssh.rb
