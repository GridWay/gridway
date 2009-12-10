/* -------------------------------------------------------------------------- */
/* Copyright 2002-2009 GridWay Team, Distributed Systems Architecture         */
/* Group, Universidad Complutense de Madrid                                   */
/*                                                                            */
/* Licensed under the Apache License, Version 2.0 (the "License"); you may    */
/* not use this file except in compliance with the License. You may obtain    */
/* a copy of the License at                                                   */
/*                                                                            */
/* http://www.apache.org/licenses/LICENSE-2.0                                 */
/*                                                                            */
/* Unless required by applicable law or agreed to in writing, software        */
/* distributed under the License is distributed on an "AS IS" BASIS,          */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   */
/* See the License for the specific language governing permissions and        */
/* limitations under the License.                                             */
/* -------------------------------------------------------------------------- */

#include "gw_client.h"
#include "gw_cmds_common.h"

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <signal.h>

/* ------------------------------------------------------------------------- */
/* GLOBAL VARIABLES                                                          */
/* ------------------------------------------------------------------------- */

const char * usage =
"USAGE\n gwhistory [-h] [-nx] <job_id>\n\n"
"SYNOPSIS\n"
"  Prints information about the execution history of a job\n\n"
"OPTIONS\n"
"  -h        print this help\n"
"  -n        do not print the header lines\n"
"  -x        xml format\n"
"  job_id    job identification as provided by gwps\n\n"
"FIELD INFORMATION\n"
"  HID       host unique identification assigned by the GridWay system\n"
"  START     the time the job start its execution on this host\n"
"  END       the time the job left this host, because it finished or it\n"
"            was migrated\n"
"  PROLOG    total prolog (file stage-in phase) time\n"
"  WRAPPER   total wrapper (execution phase) time\n"
"  EPILOG    total epilog (file stage-out phase) time\n"
"  MIGR      total migration time\n"
"  REASON    the reason why the job left this host\n"
"  QUEUE     name of the queue\n"
"  HOST      FQDN/LRMS of the resource\n";

const char * susage =
"usage: gwhistory [-h] [-nx] <job_id>\n";

extern char *optarg;
extern int   optind, opterr, optopt;

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

void signal_handler (int sig)
{
   	gw_client_finalize();
    exit(0);
}

int main(int argc, char **argv)
{
    int                job_id = -1;
  	char               opt;
  	int                n = 0, x = 0;
  	gw_client_t *      gw_session;
	int                num_records, i;
    gw_msg_history_t * history_list;
    struct sigaction   act;
    gw_return_code_t   rc;
  	
	/* ---------------------------------------------------------------- */
	/* Parse arguments                                                  */
	/* ---------------------------------------------------------------- */
	
    opterr = 0;
    optind = 1;
	
    while((opt = getopt(argc,argv,"nxh"))!= -1)
        switch(opt)
        {
            case 'n': n  = 1;
                break;
            case 'x': x  = 1;
                break;
            case 'h':
            	printf("%s", usage);
                exit(0);
                break;
            case '?':
                fprintf(stderr,"error: invalid option \'%c\'\n",optopt);
            	printf("%s", susage);
                exit(1);
                break;                
            case ':':
                fprintf(stderr,"error: must provide an argument for option \'%c\'\n",
                        optopt);                 
            	printf("%s", susage);
                exit(1);
                break;
          	}

	if( optind < argc)
		job_id = atoi(argv[optind]);
	else
	{
    	printf("%s", susage);
    	exit(1);
	}
	
    
	/* ---------------------------------------------------------------- */
	/* Connect to GWD                                                   */
	/* ---------------------------------------------------------------- */

	gw_session = gw_client_init();
	
	if ( gw_session == NULL )
	{
		fprintf(stderr,"Could not connect to gwd\n");
		return (-1);
	}

    act.sa_handler = signal_handler;
    act.sa_flags   = SA_RESTART;
    sigemptyset(&act.sa_mask);
        
    sigaction(SIGTERM||SIGINT,&act,NULL);
    
    
	/* ---------------------------------------------------------------- */
	/* Get job or pool status                                           */
	/* ---------------------------------------------------------------- */

	rc = gw_client_job_history(job_id, &history_list, &num_records);

   	if (rc == GW_RC_SUCCESS)
    {
	  if (x){
		char command[]=GW_HISTORY_COMMAND_XML;
		char command_open[GW_HISTORY_COMMAND_OPEN_SIZE_XML];

		sprintf (command_open, "%s JOB_ID=\"%i\"", command, job_id);
		gw_print_xml_header(command_open);
		
		for (i=0;i<num_records;i++){
		  gw_client_print_history_xml(&(history_list[i]), i);
		}
		gw_print_xml_footer(command);
	  }
	  else {
		if (!n)
		  gw_client_print_history_header();
		
		for (i=0;i<num_records;i++)
		  gw_client_print_history(&(history_list[i]));
	  }
	  gw_client_finalize();
    }  
    else
    {
    	fprintf(stderr,"FAILED: %s\n",gw_ret_code_string(rc)); 
           	
        gw_client_finalize();
        return -1;
    }  
	
	return 0;
}
