/* -------------------------------------------------------------------------- */
/* Copyright 2002-2006 GridWay Team, Distributed Systems Architecture         */
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
"\n gwps [-h] [-c delay] [-n] [job_id]\n\n"
"SYNOPSIS\n"
"  Prints information about all the jobs in the GridWay system (default)\n\n"
"OPTIONS\n"
"  -h         prints this help\n"
"  -c <delay> this will cause gwps to print job information every <delay>\n"
"             seconds continuously (similar to top command)\n"
"  -n         do not print the header\n"
"  job_id     only monitor this job_id\n\n"
"FIELD INFORMATION\n"
"  USER       owner of this job\n"
"  JID        job unique identification assigned by the Gridway system\n"
"  AID        array unique identification, only relevant for array jobs\n"
"  TID        task identification, ranges from 0 to TOTAL_TASKS -1, only relevant for array jobs\n"
"  DM         dispatch Manager state, one of: pend, hold, prol, prew, wrap, epil, canl, stop, migr, done, fail\n"
"  EM         execution Manager state (Globus state): pend, susp, actv, fail, done\n"
"  RWS        flags: \n"
"               - R: times this job has been restarted\n" 
"               - W: number of processes waiting for this job\n"
"               - S: re-schedule flag\n"
"  START      the time the job entered the system\n"
"  END        the time the job reached a final state (fail or done)\n"
"  EXEC       total execution time, includes suspension time in the remote queue system\n"
"  XFER       total file transfer time, includes stage-in and stage-out phases\n"
"  EXIT       job exit code\n"
"  TEMPLATE   filename of the job template used for this job\n"
"  HOST       hostname where the job is being executed\n";


const char * susage =
"usage: gwps [-h] [-c delay] [-n] [job_id]\n";

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
    int               job_id = -1;
  	char              opt;
  	int               c = 0, n = 0;
    int               delay = 0;
  	gw_client_t *     gw_session;
	gw_msg_job_t      job_status;
	struct sigaction  act;
  	gw_return_code_t  rc;
  	
	/* ---------------------------------------------------------------- */
	/* Parse arguments                                                  */
	/* ---------------------------------------------------------------- */
	
    opterr = 0;
    optind = 1;
	
    while((opt = getopt(argc, argv, ":nhc:")) != -1)
        switch(opt)
        {
            case 'c': c  = 1;
                delay = atoi(optarg);
                break;
            case 'n': n = 1;
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
      	
	if ( optind < argc)
		job_id = atoi(argv[optind]);
    
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

	do
	{
		if (c)
		{
			cls();
            move(0,0);
        }

	    if (job_id != -1)
	    	rc = gw_client_job_status(job_id, &job_status);
		else	    		    
			rc = gw_client_job_status_all( );

    	if (rc == GW_RC_SUCCESS)
        {
        	if (!n)
        		gw_client_print_status_header();

		    if (job_id != -1)
		    	gw_client_print_status(&job_status);
			else	    		    
		    	gw_client_print_pool_status();
        }  
	    else
	    {
	    	fprintf(stderr,"FAILED: %s\n",gw_ret_code_string(rc)); 
           	
	        gw_client_finalize();
	        return -1;
	    }  

		sleep(delay);	    
		
	} while(c);
	
	return 0;
}

