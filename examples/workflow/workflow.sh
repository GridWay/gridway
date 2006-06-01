#!/bin/sh

A_ID=`gwsubmit -v -t A.jt | cut -f2 -d':' | cut -f2 -d' '`
B_ID=`gwsubmit -v -t B.jt -d "$A_ID" | cut -f2 -d':' | cut -f2 -d' '`
C_ID=`gwsubmit -v -t C.jt -d "$A_ID" | cut -f2 -d':' | cut -f2 -d' '`
D_ID=`gwsubmit -v -t D.jt -d "$B_ID $C_ID"| cut -f2 -d':' | cut -f2 -d' '`

gwwait $D_ID

echo "Random number `cat out.A`"
echo "Workflow computation `cat out.workflow`"
