/* -------------------------------------------------------------------------- */
/* Copyright 2002-2013, GridWay Project Leads (GridWay.org)                   */
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
#include <string.h>

/* ------------------------------------------------------------------------- */
/* GLOBAL VARIABLES                                                          */
/* ------------------------------------------------------------------------- */

const char * usage =
"\n gwacct [-h] [-nx] [-d n|-w n|-m n|-t s] <-u user|-r host>\n\n"
"SYNOPSIS\n"
"  Prints accounting information about users or hosts in the GridWay system\n\n"
"OPTIONS\n"
"  -h        print this help\n"
"  -n        do not print the header lines\n"
"  -x        xml format\n"
"  -d n      print accounting information from n days ago  (ex: -d 1)\n"
"  -w n      print accounting information from n weeks ago (ex: -w 1)\n"
"  -m n      print accounting information from n months ago (ex: -m 1)\n"
"  -t s      print accounting information from s seconds, where s is an\n"
"            epoch (i.e. -t 1159809792)\n"
"  -u user   print accounting information for user\n\n"
"  -r host   print accounting information for host\n"
"FIELD INFORMATION\n"
"  HOST/USER host/user usage summary for this user/host\n"
"  XFR       total transfer time on this host (for this user)\n"
"  EXE       total execution time on this host (for this user), without\n"
"            suspension time\n"
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
"usage: gwacct [-h] [-nx] [-d n|-w n|-m n|-t s] <-u user|-r host>\n";

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
  	int                n = 0, x = 0, u = 0, r = 0, t = 0;
	char *             hostname = NULL;
	char *             username = NULL;
	char * 			   time_arg = NULL;
	time_t			   from_time = (time_t)NULL;
	
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
	
    while((opt = getopt(argc,argv,":hnxu:r:d:w:m:t:"))!= -1)
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
                
            case 'u':
            
            	if ( username != NULL )
            		free(username);
            		
            	username = strdup(optarg);
                u = 1;               
                break;

            case 'r':
            
            	if ( hostname != NULL )
            		free(hostname);
            
            	hostname = strdup(optarg);
                r = 1;
                break;
                
            case 't': // accounting information since argument (must be an epoch)                      
           
            	time_arg = strdup(optarg);
            	from_time = (time_t)atoi(time_arg);
            	t++;        
                break;       
                
            case 'd': // accounting information since argument (number of days)                      
           
            	time_arg = strdup(optarg);
            	from_time = time(NULL) - atoi(time_arg)*24*60*60;
            	t++;            	
                break;  
                
            case 'w': // accounting information since argument (number of weeks)                      
           
            	time_arg = strdup(optarg);
            	from_time = time(NULL) - atoi(time_arg)*7*24*60*60;
            	t++;            	
                break; 
                
            case 'm': // accounting information since argument (number of 30 day months)                      
           
            	time_arg = strdup(optarg);
            	from_time = time(NULL) - atoi(time_arg)*30*7*24*60*60;
            	t++;           	
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
          	        	

	if (  ((u + r) < 1 )  // User must define either host and/or user search 
	||    (t       > 1))  // Just one time option   
	{
		if ( hostname != NULL )
			free(hostname);
		
		if ( username != NULL )
			free(username);

        if( t>1 )
            printf("You can only specify one time option (d, w, m or t)\n\n");
		
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
    if( (u + r) == 2 ){
	  printf ("ERROR: -u user -r host joint options not yet implemented.\n");
	  exit(1);
	  //      rc = gw_client_host_and_user_accts(hostname, username, &accts, &num, from_time);
	}
    else {
      if ( u == 1 )
		rc = gw_client_user_accts(username, &accts, &num, from_time);
      else
		rc = gw_client_host_accts(hostname, &accts, &num, from_time);
    }
    
   	if (rc == GW_RC_SUCCESS)
    {
    	if (num == 0)
    	{
    		if( (u + r) == 2 )
    		{
				fprintf(stderr,"FAILED: No records found for %s @ %s.\n",username, hostname);
				
				free(username);
				free(hostname);
				return -1;	    			
    		}
    		else
    		{
	       		if ( u == 1 )
	       		{
					fprintf(stderr,"FAILED: No records found for %s.\n",username);
					
					free(username);
					return -1;	       			
	       		}
	       		else
	      		{
					fprintf(stderr,"FAILED: No records found for %s.\n",hostname);
					
					free(hostname);
					return -1;	 	      			
	      		}
   			
    			
    		}    		
    	}
    	
		if (x){
		  char command[]=GW_ACCT_COMMAND_XML;
		  char user_string[]="USER";
		  char host_string[]="HOST";
		  char command_open[GW_ACCT_COMMAND_OPEN_SIZE_XML];
 
		  if ( ( u + r ) == 2 ){
			sprintf (command_open, "%s %sNAME@%sNAME=\"%s@%s\"", command, user_string, host_string, username, hostname);
		    gw_print_xml_header(command_open);		  
		  } 
		  else if ( u == 1 ){
		    sprintf (command_open, "%s %sNAME=\"%s\"", command, user_string, username);
		    gw_print_xml_header(command_open);		  
		  } 
		  else if ( r == 1 ){
		    sprintf (command_open, "%s %sNAME=\"%s\"", command, host_string, hostname);
		    gw_print_xml_header(command_open);		  
		  }
		  gw_client_print_accts_xml( accts, num, u, r );
		  gw_print_xml_footer(command);		  
		}
		else {
		  if (!n)
			{
			  if( (u + r) == 2 )
       			gw_client_print_host_and_user_accts_header(hostname, username, from_time);
			  else
				{
				  if ( u == 1 )
		       		gw_client_print_user_accts_header(username, from_time);
				  else
	      			gw_client_print_host_accts_header(hostname, from_time);
				}
			}

		  gw_client_print_accts(accts, num);
		}
    }  
    else
    {
    	fprintf(stderr,"FAILED: %s\n",gw_ret_code_string(rc)); 

		if( (u + r) == 2 )
		{	
			free(username);
			free(hostname);
		}  	
		else
       		if ( u == 1 )
				free(username);
       		else
				free(hostname);
    	
        return -1;
    }  
#else
	fprintf(stderr,"FAILED: Berkeley Database support not compiled in GridWay\n");
#endif

	if( (u + r) == 2 )	
	{	
		free(username);
		free(hostname);
	}	
	else
   		if ( u == 1 )
			free(username);
   		else
			free(hostname);
			
	return 0;
}
