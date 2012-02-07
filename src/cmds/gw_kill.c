/* -------------------------------------------------------------------------- */
/* Copyright 2002-2012, GridWay Project Leads (GridWay.org)                   */
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
"USAGE\n gwkill [-h] [-a] [-k|-t|-o|-s|-r|-l|-9] <job_id [job_id2 ...]|-A array_id>\n\n"
"SYNOPSIS\n"
"  Sends a signal to a job\n\n"
"OPTIONS\n"
"  -h            print this help\n"
"  -a            asynchronous signal, only relevant for KILL and STOP\n"
"  -k            kill (default, if no signal specified)\n"
"  -t            stop\n"
"  -r            resume\n"
"  -o            hold\n"
"  -l            release\n"
"  -s            re-schedule\n"
"  -9            hard kill, removes the job from the system without synchronizing"
"                remote job execution or cleaning remote host\n"
"  job_id [job_id2 ...] job identification as provided by gwps\n"
" -A <array_id>  array identification as provided by gwps\n";

const char * susage =
"usage: gwkill [-h] [-a] [-k | -t | -o | -s | -r | -l | -9] <job_id | -A array_id>\n";

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
    int                job_id   = -1;
    int                array_id = -1;
  	char               opt;
  	int                a = 0, k = 0, t = 0, o = 0, s = 0, r = 0, l = 0, A = 0, hard=0;
  	gw_client_t *      gw_session;
	gw_boolean_t       blocking = GW_TRUE;
	gw_client_signal_t signal = GW_CLIENT_SIGNAL_KILL;
	
    struct sigaction   act;
    gw_return_code_t   rc;
    int mrc;
  	
	/* ---------------------------------------------------------------- */
	/* Parse arguments                                                  */
	/* ---------------------------------------------------------------- */
	
    opterr = 0;
    optind = 1;
	
    while((opt = getopt(argc, argv, ":haktosrl9A:")) != -1)
        switch(opt)
        {
            case 'a': a = 1; blocking = GW_FALSE;
                break;    
            case 't': t = 1; signal   = GW_CLIENT_SIGNAL_STOP;
                break;
            case 'o': o = 1; signal   = GW_CLIENT_SIGNAL_HOLD;
                break;      
            case 's': s = 1; signal   = GW_CLIENT_SIGNAL_RESCHEDULE;
                break;
            case 'r': r = 1; signal   = GW_CLIENT_SIGNAL_RESUME;
                break;                 
            case 'l': l = 1; signal   = GW_CLIENT_SIGNAL_RELEASE;
                break;
            case 'k': k = 1; signal   = GW_CLIENT_SIGNAL_KILL;
                break;
            case '9': hard = 1; signal   = GW_CLIENT_SIGNAL_KILL_HARD;
                break;                
            case 'A': A = 1;
	            array_id = atoi(optarg);
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
		
    if ( t+o+s+r+l+k+hard > 1 )
    {
    	printf("%s", susage);
    	exit(1);
    }
    
    if (!A)
    {
		if (!(optind < argc))
		{
	    	printf("%s", susage);
	    	return -1;
		}
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
    
    if (A)
    {
    	rc = gw_client_array_signal (array_id, signal, blocking);
    	
		if ( rc != GW_RC_SUCCESS)
		{
	   		fprintf(stderr,"FAILED: failed could not signal one or more jobs!\n"); 
	        mrc = -1;										
		}
		else
			mrc =  0;    	
    }
	else
	{
		for (;optind<argc;optind++)
		{			
			job_id = atoi(argv[optind]);
			
			rc = gw_client_job_signal (job_id, signal, blocking);
			
			if ( rc != GW_RC_SUCCESS)
			{
		   		fprintf(stderr,"FAILED: %s (job %i)\n",
		   			gw_ret_code_string(rc),job_id); 
		   		mrc =  -1;
			}
		}
	}

	gw_client_finalize();
	
	return mrc;
}
