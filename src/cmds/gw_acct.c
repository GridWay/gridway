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
"\n gwacct [-h] [-n] <-u user|-r host>\n\n"
"SYNOPSIS\n"
"  Prints accounting information about users or hosts in the GridWay system\n\n"
"OPTIONS\n"
"  -h        prints this help\n"
"  -n        do not print the header lines\n"
"  -r host   print accounting information for host\n"
"  -u user   print accounting information for user\n\n"
"FIELD INFORMATION\n"
"  HOST/USER host/user usage summary for this user/host\n"
"  XFR       total transfer time on this host (for this user)\n"
"  EXE       total execution time on this host (for this user), without suspension time\n"
"  SUSP      total suspension (queue) time on this host (for this user)\n"

"  TOTS      total executions on this host (for this user). Termination reasons:\n"
"    - SUCC  success\n"
"    - ERR   error\n"
"    - KILL  kill\n"
"    - USER  user requested\n"
"    - SUSP  suspension timeout\n"
"    - DISC  discovery timeout\n"
"    - SELF  self migration\n"
"    - PERF  performance degradation\n"
"    - S/R   stop/resume\n";


const char * susage =
"usage: gwacct [-h] [-n] <-u user|-r host>\n";

extern char *optarg;
extern int   optind, opterr, optopt;

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

void signal_handler (int sig)
{
#ifdef HAVE_LIBDB	
   	gw_acct_db_close();
#endif
    exit(0);
}

int main(int argc, char **argv)
{
  	char               opt;
  	int                n = 0, u = 0, r = 0;
	char *             name = NULL;
	
#ifdef HAVE_LIBDB	
	gw_acct_t          **accts;
    gw_return_code_t   rc;	
	int                num;    
#endif
	
    struct sigaction   act;

  	
	/* ---------------------------------------------------------------- */
	/* Parse arguments                                                  */
	/* ---------------------------------------------------------------- */
	
    opterr = 0;
    optind = 1;
	
    while((opt = getopt(argc,argv,":nhu:r:"))!= -1)
        switch(opt)
        {
            case 'n': n  = 1;
                break;
                
            case 'h':
            	printf("%s", usage);
                exit(0);
                break;
                
            case 'u':
            
            	if ( name != NULL )
            		free(name);
            		
            	name = strdup(optarg);
                u = 1;
                break;

            case 'r':
            
            	if ( name != NULL )
            		free(name);
            
            	name = strdup(optarg);
                r = 1;
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

	if (((u == 1) && (r == 1)) 
	|| ((u==0) && (r == 0)))
	{
		if ( name != NULL )
			free(name);
			
       	printf("%s", susage);
        exit(1);
	}
		

    act.sa_handler = signal_handler;
    act.sa_flags   = SA_RESTART;
    sigemptyset(&act.sa_mask);
        
    sigaction(SIGTERM||SIGINT,&act,NULL);
        
	/* ---------------------------------------------------------------- */
	/* Get acct info                                                    */
	/* ---------------------------------------------------------------- */

#ifdef HAVE_LIBDB
	if ( u == 1 )
		rc = gw_client_user_accts(name, &accts, &num);
	else
		rc = gw_client_host_accts(name, &accts, &num);

   	if (rc == GW_RC_SUCCESS)
    {
    	if (num == 0)
    	{
    		fprintf(stderr,"FAILED: No records found for %s.\n",name);
    		
    		free(name);
    		return -1;    		
    	}
    	
       	if (!n)
       	{
       		if ( u == 1 )
	       		gw_client_print_user_accts_header(name);
       		else
      			gw_client_print_host_accts_header(name);
       	}

		gw_client_print_accts(accts, num);
    }  
    else
    {
    	fprintf(stderr,"FAILED: %s\n",gw_ret_code_string(rc)); 
    	
   		free(name);
        return -1;
    }  
#else
	fprintf(stderr,"FAILED: Berkley Database support not compiled in GridWay\n");
#endif

	free(name);	
	return 0;
}
