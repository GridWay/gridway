#!/bin/sh -x

IDENTIFIER="__IDENTIFIER__"
STDOUT="__STDOUT__"
STDERR="__STDERR__"
COMMAND="__COMMAND__"

export GLOBUS_LOCATION="/tmp"
__ENVIRONMENT__

STATUS_FILE="\$HOME/.gw_ssh_status"
touch \${STATUS_FILE}

TIME=`date +%s`
echo "\${TIME}:\${IDENTIFIER}:start:\${COMMAND}" >> \${STATUS_FILE}

\${COMMAND} 2>\${STDERR} >\${STDOUT}

TIME=`date +%s`
echo "\${TIME}:\${IDENTIFIER}:end:\$?" >> \${STATUS_FILE}
