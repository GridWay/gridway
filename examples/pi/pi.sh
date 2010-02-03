#!/bin/sh
# Submitting the 4 jobs to GridWay and getting the Array Identificator
AID=`gwsubmit -v -t pi.jt -n 4 | head -1 | awk '{print $3}'`

# If succeeded we keep further otherwise inform the user
if [ $? -ne 0 ]; then
	echo "Submission failed!"
	exit 1
fi

# It is time to wait for the collection of 4 jobs to finish, so the user does not receive the shell prompt
gwwait -v -A $AID

# Once the waiting is over, compute the sum of the stdout_files
if [ $? -eq 0 ]
then
	awk 'BEGIN {sum=0} {sum+=$1} END {printf "Pi is %0.10f\n", sum}' stdout_file.*
else
	echo "Some tasks failed! Check the errors with gwhistory or looking to the $GW_LOCATION/var/\$JOB_ID/job.log"
fi
