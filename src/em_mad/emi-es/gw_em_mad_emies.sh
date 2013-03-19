#!/bin/bash

# Help
while getopts h option
    do
        case $option in
        h)   echo -e "USAGE\n gw_em_mad_emies [-h]" \
                     "\n\nSYNOPSIS"\
                     "\n  Execution driver to interface with EMI-ES services. It is not intended to be used from CLI."\
                     "\n\nOPTIONS"\
                     "\n  -h    print this help";
             exit 0;;
        [?]) echo -e "usage: gw_em_mad_emies [-h]";
             exit 1;;
        esac
    done

if [ -z "${GW_LOCATION}" ]; then
    echo "Please, set GW_LOCATION variable."
    exit -1
fi

. $GW_LOCATION/bin/gw_mad_common.sh

setup_globus
mad_debug
check_proxy

exec nice -n $PRIORITY $GW_LOCATION/bin/gw_em_mad_emies.bin $*
