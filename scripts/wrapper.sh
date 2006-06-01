#!/bin/bash -xv

# -------------------------------------------------------------------------- 
# Copyright 2002-2006 GridWay Team, Distributed Systems Architecture         
# Group, Universidad Complutense de Madrid                                   
#                                                                            
# Licensed under the Apache License, Version 2.0 (the "License"); you may    
# not use this file except in compliance with the License. You may obtain    
# a copy of the License at                                                   
#                                                                            
# http://www.apache.org/licenses/LICENSE-2.0                                 
#                                                                            
# Unless required by applicable law or agreed to in writing, software        
# distributed under the License is distributed on an "AS IS" BASIS,          
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   
# See the License for the specific language governing permissions and        
# limitations under the License.                                             
# -------------------------------------------------------------------------- 

#-------------------------------------------------------------------------------
#   Wrapping script for the GW submission tool
#-------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
#          Check environment and remote job directory on remote machine
#-------------------------------------------------------------------------------

setup(){

    if [ -z "${GW_JOB_ID}" \
            -o -z "${GW_USER}" \
            -o -z "${GW_HOSTNAME}" \
            -o -z "${GLOBUS_LOCATION}" ]; then
         echo "failed."
         exit 1
    fi

    printf "`date`: Checking remote job home... "

    RMT_JOB_HOME=${HOME}/.gw_${GW_USER}_${GW_JOB_ID}

    if [ ! -d ${RMT_JOB_HOME} ]
    then
        echo "failed (will perform explicit file staging)."
        
        printf "`date`: Checking Globus programs... "

        if [ -x "${GLOBUS_LOCATION}/bin/globus-url-copy" ]; then
            GLOBUS_CP=${GLOBUS_LOCATION}/bin/globus-url-copy
        else
            echo "failed."
            exit 1
        fi

        echo "done."
        
        printf "`date`: Creating remote job home... "

        mkdir -p ${RMT_JOB_HOME}

        echo "done."
        
        printf "`date`:Staging job environment file... "

		if [ -z "${GW_SE_HOSTNAME}" ]; then
	        SRC_URL="gsiftp://${GW_HOSTNAME}/~/.gw_${GW_USER}_${GW_JOB_ID}/job.env"
        else
	        SRC_URL="gsiftp://${GW_SE_HOSTNAME}/~/.gw_${GW_USER}_${GW_JOB_ID}/job.env"
        fi
	        
        DST_URL="file:${RMT_JOB_HOME}/job.env"
 
        ${GLOBUS_CP} ${SRC_URL} ${DST_URL}

        if [ $? -ne 0 ]; then
	        echo "failed."
	        exit 1;
        else
    	    echo "done."
        fi        
        
        STAGE=1    
    else
        echo "done."

        STAGE=0
    fi

    printf "`date`: Checking job environment... "

    # Warning! Check it well!
    cd ${RMT_JOB_HOME}
    
	source ./job.env
	
    if [ -z "${GW_RESTARTED}" -o -z "${GW_EXECUTABLE}" ]; then
         echo "failed."
         exit 1
    fi

    echo "done."
}


execution(){

    STG_FILES="$GW_EXECUTABLE,$GW_INPUT_FILES,$GW_STDIN_FILE"

    SAVED_IFS=$IFS
    IFS=","

    for FILES in $STG_FILES; do
        if [ -z "$FILES" -o "$FILES" = " " ]; then
          continue
        fi

        SRC_FILE=`echo $FILES | awk '{print $1}'`
        DST_FILE=`echo $FILES | awk '{print $2}'`

        if [ -z "$DST_FILE" ]; then
            DST_FILE=$SRC_FILE
        fi

        DST_FILE=`basename $DST_FILE`

        # Uncompress .zip, .gz, .Z and .tgz files
        case $DST_FILE in
        *.zip)
            UNCOMPRESS="unzip"
            ;;
        *.tgz|*.tar.gz)
            UNCOMPRESS="tar xzf"
            ;;
        *.gz)
            UNCOMPRESS="gunzip"
            ;;
        *.Z)
            UNCOMPRESS="uncompress"
            ;;
        *)
            UNCOMPRESS=""
            ;;
        esac

        if [ -n "$UNCOMPRESS" ]; then
            printf "`date`: Uncompressing file \"$DST_FILE\"... "

            ${UNCOMPRESS} ${RMT_JOB_HOME}/${DST_FILE}
 
            if [ $? -eq 0 ]; then
                echo "done."
            else
                echo "failed."
            fi
        fi
    done

    IFS=$SAVED_IFS

    #
    #   Execution of actual job in background
    #

    printf "`date`: Executing actual job \"$GW_EXECUTABLE $GW_ARGUMENTS\"... "

    export PATH=.:$PATH

    # Warning! Do it in prolog?
    chmod u+x $GW_EXECUTABLE

    # Warning! Increment nice only if jobmanager is fork
    #/usr/bin/nice -20

   if [ -z "${GW_STDIN_FILE}" ]; then
	GW_STDIN_FILE=/dev/null
   fi

 
    ${GW_EXECUTABLE} ${GW_ARGUMENTS} < ${GW_STDIN_FILE} >> stdout.execution 2>> stderr.execution &
    
    JOB_PID=$!

    echo "done."
    echo "`date`: PID of actual job is $JOB_PID."

    #
    #   Execution of monitor in background, if defined
    #

    if [ -f .monitor ]
    then
        printf "`date`: Executing monitor \".monitor $JOB_PID\"... "

        .monitor $JOB_PID >> stdout.monitor 2>> stderr.monitor &
        
        MONITOR_PID=$!

        if [ $? -eq 0 ]
        then   
            echo "done."
        else
            echo "failed."
        fi

        echo "`date`: PID of monitor is $MONITOR_PID."
    fi

    #
    # 	Wait for actual job termination
    #

    printf "`date`: Waiting for actual job to finish... "

    wait $JOB_PID

    EXIT_STATUS=$?

    echo "done."

    kill %1

    #
    #   Check performance degradations or self-migrations
    #

    printf "`date`: Checking performance degradations or self-migrations... "

    if [ -f gw_rank ]
    then
        EXIT_STATUS="S"
    elif [ -f gw_reqs ]
    then
        EXIT_STATUS="S"
    elif [ -f gw_perf ]
    then
        EXIT_STATUS="P"
    fi
    
    echo "done." 

    #
    #   Kill monitor
    #
    if [ -f .monitor \
            -a "${EXIT_STATUS}" != "P" ]
    then
        printf "`date`: Killing monitor... "

        kill %2
        
        if [ $? -eq 0 ]
        then   
            echo "done."
        else
            echo "failed."
        fi
    fi

    echo "EXIT_STATUS=$EXIT_STATUS"

    if [ -f gw_rank ]
    then
        echo "NEW_RANK=\"`cat gw_rank`"\"
    fi

    if [ -f gw_reqs ]
    then
        echo "NEW_REQS=\"`cat gw_reqs`"\"
    fi

    if [ -f gw_perf ]
    then
        echo "PERF_DEGR=\"`cat gw_perf`"\"
    fi
}    


transfer_input_files(){

    STG_FILES="$GW_EXECUTABLE,$GW_INPUT_FILES,$GW_STDIN_FILE,.monitor"

    SAVED_IFS=$IFS
    IFS=","

    for FILES in $STG_FILES; do
        if [ -z "$FILES" -o "$FILES" = " " ]; then
          continue
        fi

        SRC_FILE=`echo $FILES | awk '{print $1}'`
        DST_FILE=`echo $FILES | awk '{print $2}'`

        case ${SRC_FILE} in
        gsiftp://*)
            SERVER=`echo $SRC_FILE | awk -F/ '{print $3}'`
            FILE_NAME=`echo $SRC_FILE | awk -F/ '{print $NF}'`
            FILE_PATH=`echo $SRC_FILE | awk -F/ \
                '{ORS=""; for (i=4;i<=NF;i++) print "/" $i; print "\n"}'`
            
            if [ -z "$DST_FILE" ]; then
                DST_FILE=$FILE_NAME
            fi
	
 			if [ -z "${GW_SE_HOSTNAME}" ]; then
	            SRC_URL="gsiftp://${GW_HOSTNAME}/~/.gw_${GW_USER}_${GW_JOB_ID}/${DST_FILE}"
	        else
	            SRC_URL="gsiftp://${GW_SE_HOSTNAME}/~/.gw_${GW_USER}_${GW_JOB_ID}/${DST_FILE}"	        
	        fi
	        
            DST_URL="file:${RMT_JOB_HOME}/${DST_FILE}"

            printf "`date`: Staging remote input file \"${SRC_FILE}\" as \"${DST_FILE}\"... "
             
            ${GLOBUS_CP} ${SRC_URL} ${DST_URL}

            if [ $? -ne 0 ]; then
                echo "failed."
            else
                echo "done."
            fi
            ;;
        
        /*)
            printf "`date`: Skipping staging of absolute file \"$SRC_FILE\"... "
            echo "done."
            ;;
        
        *)
            if [ -z "$DST_FILE" ]; then
                DST_FILE=$SRC_FILE
            fi

  			if [ -z "${GW_SE_HOSTNAME}" ]; then
	            SRC_URL="gsiftp://${GW_HOSTNAME}/~/.gw_${GW_USER}_${GW_JOB_ID}/${DST_FILE}"
	        else
	            SRC_URL="gsiftp://${GW_SE_HOSTNAME}/~/.gw_${GW_USER}_${GW_JOB_ID}/${DST_FILE}"	        
	        fi
            
            DST_URL="file:${RMT_JOB_HOME}/${DST_FILE}"

            printf "`date`: Staging local input file \"${SRC_FILE}\" as \"${DST_FILE}\"... "
             
            ${GLOBUS_CP} ${SRC_URL} ${DST_URL}

            if [ $? -ne 0 ]; then
                echo "failed."
            else
                echo "done."
            fi
            ;;
        esac
	
    done
    
    IFS=$SAVED_IFS
}

transfer_output_files(){

    STG_FILES="$GW_OUTPUT_FILES,$GW_STDOUT_FILE,$GW_STDERR_FILE,stdout.execution,stderr.execution"
    
    if [ -f .monitor ]
    then
        STG_FILES="$STG_FILES,stdout.monitor,stderr.monitor"
    fi
        
    SAVED_IFS=$IFS
    IFS=","

    for FILES in ${STG_FILES}; do
        if [ -z "$FILES" -o "$FILES" = " " ]; then
          continue
        fi

        SRC_FILE=`echo $FILES | awk '{print $1}'`
        DST_FILE=`echo $FILES | awk '{print $2}'`

        if [ -z "$DST_FILE" ]; then
            DST_FILE=$SRC_FILE
        fi
        
        SRC_URL="file:${RMT_JOB_HOME}/${SRC_FILE}"
        
		if [ -z "${GW_SE_HOSTNAME}" ]; then
		    DST_URL="gsiftp://${GW_HOSTNAME}/~/.gw_${GW_USER}_${GW_JOB_ID}/${SRC_FILE}"
	    else
		    DST_URL="gsiftp://${GW_SE_HOSTNAME}/~/.gw_${GW_USER}_${GW_JOB_ID}/${SRC_FILE}"
	    fi        
            
        if [ -f ${RMT_JOB_HOME}/${SRC_FILE} ]; then
            printf "`date`: Staging output file \"${SRC_FILE}\"... "
   
            ${GLOBUS_CP} ${SRC_URL} ${DST_URL}

            if [ $? -ne 0 ]; then
                echo "failed."
            else
                echo "done."
            fi
        else
            printf "`date`: Skipping staging of non-existent output file \"${SRC_FILE}\"... "
            echo "done."
        fi
    done
        
    IFS=$SAVED_IFS
}

#-------------------------------------------------------------------------------
#                       WRAPPER SCRIPT
#-------------------------------------------------------------------------------

SECONDS=0

setup

if [ $STAGE -eq 1 ]; then
  transfer_input_files
fi

execution

if [ $STAGE -eq 1 ]; then
  transfer_output_files
fi

echo "REAL_TIME=$SECONDS"
