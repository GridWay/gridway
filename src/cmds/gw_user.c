/* -------------------------------------------------------------------------- */
/* Copyright 2002-2011, GridWay Project Leads (GridWay.org)                   */
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
"USAGE\n gwuser [-h] [-nx]\n\n"
"SYNOPSIS\n"
"  Prints information about users in the GridWay system\n\n"
"OPTIONS\n"
"  -h    print this help\n"
"  -n    do not print the header lines\n"
"  -x    xml format\n\n"
"FIELD INFORMATION\n"
"  UID      user unique identification assigned by the GridWay system\n"
"  NAME     name of this user\n"
"  JOBS     number of Jobs in the GridWay system\n"
"  RUN      number of running jobs\n"
"  IDLE     idle time, (time with JOBS = 0)\n"
"  EM       execution manager drivers loaded for this user\n"
"  TM       transfer manager drivers loaded for this user\n"
"  PID      process identification of driver processes\n"
"  IDENTITY distinguished name of the user's proxy\n";

const char * susage =
"usage: gwuser [-h] [-nx]\n";

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
    char               opt;
    int                n = 0, x = 0;
    gw_client_t *      gw_session;
    int                num_users, i;
    gw_msg_user_t *    user_status;
    struct sigaction   act;
    gw_return_code_t   rc;
  	
    /* ---------------------------------------------------------------- */
    /* Parse arguments                                                  */
    /* ---------------------------------------------------------------- */
	
    opterr = 0;
    optind = 1;
	
    while((opt = getopt(argc,argv,"nhx"))!= -1)
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

    rc = gw_client_user_status(&user_status, &num_users);
	
    if (rc == GW_RC_SUCCESS)
    {
	  if (x){
		// A.L: It would be nice to include all this settings for header and footer
		// in the gw_client_print_user_xml function
		char command[]="gwuser";
		int xml_header_flag = 1, xml_footer_flag = 1;
		if ( xml_header_flag ){
		  gw_print_xml_header(command);
		  xml_header_flag = 0;
		}
		for (i=0;i<num_users;i++){
		  gw_client_print_user_xml(&(user_status[i]));
		}
		if ( xml_footer_flag ){
		  gw_print_xml_footer(command);
		  xml_footer_flag = 0;
		}
	  }
	  else {
       	if (!n)
		  gw_client_print_user_header();
		
        for (i=0;i<num_users;i++)
		  gw_client_print_user(&(user_status[i]));
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
