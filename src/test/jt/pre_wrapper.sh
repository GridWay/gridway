#!/bin/sh

echo "PreWrapper Token - for GW Test" > wrappertoken

if cmp prewrappertoken wrappertoken 2>/dev/null; then
   exit 0
else
   exit 1
fi
