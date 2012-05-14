#!/bin/bash

# Help
while getopts d:t:h option
    do
        case $option in
        h)   echo -e "USAGE\n gw_em_mad_cream [-h] [-d] [-t]" \
                     "\n\nSYNOPSIS"\
                     "\n  Execution driver to interface with CREAM services. It is not intended to be used from CLI."\
                     "\n\nOPTIONS"\
                     "\n  -h    print this help"\
                     "\n  -d    delegation id"\
                     "\n  -t    credentials refresh time";
             exit 0;;
        [?]) echo -e "usage: gw_em_mad_cream [-h] [-d] [-t]";
             exit 1;;
        esac
    done

if [ -z "${GW_LOCATION}" ]; then
    echo "Please, set GW_LOCATION variable."
    exit -1
fi

. $GW_LOCATION/bin/gw_mad_common.sh

setup_globus
cd_var
mad_debug
check_proxy

exec nice -n $PRIORITY $GW_LOCATION/bin/gw_em_mad_cream.bin $*
