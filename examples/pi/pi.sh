#!/bin/sh

AID=`gwsubmit -v -t pi.job -n 4 | head -1 | awk '{print $3}'`

if [ $? -ne 0 ]
then
    echo "Submission failed!"
    exit 1
fi

gwwait -v -A $AID

if [ $? -eq 0 ]
then
    awk 'BEGIN {sum=0} {sum+=$1} END {printf "Pi is %0.12g\n", sum}' stdout_file.*
else
    echo "Some tasks failed!"
fi

gwkill -A $AID
