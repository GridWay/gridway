#!/bin/sh

if [ "x$GW_LOCATION" == "x" ]; then
  echo GW_LOCATION not set.
  exit -1
fi

cp -R bin $GW_LOCATION
cp -R libexec $GW_LOCATION
