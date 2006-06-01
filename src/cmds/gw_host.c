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
"\n gwhost [-h] [-c delay] [-n] [-m job_id] [host_id]\n\n"
"SYNOPSIS\n"
"  Prints information about all the hosts in the GridWay system (default)\n\n"
"OPTIONS\n"
"  -h           prints this help\n"
"  -c <delay>   this will cause gwhost to print job information every <delay>\n"
"               seconds continuously (similar to top command)\n"
"  -n           do not print the header\n"
"  -m <job_id>  prints hosts matching the requirements of a given job\n"
"  host_id      only monitor this host_id, also prints queue information\n\n"
"FIELD INFORMATION\n"
"  HID          host unique identification assigned by the Gridway system\n"
"  OS           operating system\n"
"  ARCH         architecture\n"
"  MHZ          CPU speed in MHZ\n"
"  %CPU         free CPU ratio\n"
"  MEM(F/T)     system memory: F = Free, T = Total\n"
"  DISK(F/T)    secondary storage: F = Free, T = Total\n"
"  N(U/F/T)     number of slots: U = used by GridWay, F = free, T = total\n"
"  LRMS         local resource management system, the jobmanager name\n"
"  HOSTNAME     FQDN of this host\n\n"
"QUEUE FIELD INFORMATION\n"
"  QUEUENAME    name of this queue\n"
"  SL(F/T)      slots:  F = Free, T = Total\n"
"  WALLT        queue wall time\n"
"  CPUT         queue cpu time\n"
"  COUNT        queue count number\n"
"  MAXR         max. running jobs\n"
"  MAXQ         max. queued jobs\n"
"  STATUS       queue status\n"
"  DISPATCH     queue dispatch type\n"
"  PRIORITY     queue priority\n";
       
const char * susage =
"usage: gwhost [-h] [-c delay] [-n] [-m job_id] [host_id]\n";

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
    int               host_id = -1;
    int               job_id  = -1;    
  	char              opt;
  	int               c = 0, n = 0, m = 0;
    int               delay = 0;
  	gw_client_t *     gw_session;
	gw_msg_host_t     host_status;
	struct sigaction  act;
  	gw_return_code_t  rc;
  	
	gw_msg_match_t *  match_list = NULL;
	int               num_records;
	int               i;	  	

	/* ---------------------------------------------------------------- */
	/* Parse arguments                                                  */
	/* ---------------------------------------------------------------- */
	
    opterr = 0;
    optind = 1;
	
    while((opt = getopt(argc,argv,":nhc:m:"))!= -1)
        switch(opt)
        {
            case 'c': c  = 1;
                delay = atoi(optarg);
                break;
            case 'n': n = 1;
                break;    
            case 'm': m = 1;
                job_id = atoi(optarg);
                break;                
            case 'h':
                printf("%s",usage);
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
		host_id = atoi(argv[optind]);
    
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
	/* Get host or pool status                                          */
	/* ---------------------------------------------------------------- */

	if (m)
	{
		rc = gw_client_match_job(job_id, &match_list, &num_records);
		
    	if (rc == GW_RC_SUCCESS)
        {
        	if (!n)
        		gw_client_print_host_match_header();
			
			for (i=0;i<num_records;i++)
	    		gw_client_print_host_match(&(match_list[i]));
	    	
	    	if (match_list != NULL)
	    		free(match_list);
        }  
	    else
	    {
	    	fprintf(stderr,"FAILED: %s\n",gw_ret_code_string(rc)); 
           	
	        gw_client_finalize();
	        return -1;
	    }  
    	
		return 0;	
	}
	
	do
	{
		if (c)
		{
			cls();
            move(0,0);
        }

	    if (host_id != -1)
	    	rc = gw_client_host_status(host_id, &host_status);
		else	    		    
			rc = gw_client_host_status_all( );

    	if (rc == GW_RC_SUCCESS)
        {
        	if (!n)
        		gw_client_print_host_status_header();

		    if (host_id != -1)
		    {
		    	gw_client_print_host_status(&host_status);
		    	gw_client_print_host_queues(&host_status,!n);
		    }
			else	    		    
		    	gw_client_print_host_pool_status();
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
