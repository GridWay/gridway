/* -------------------------------------------------------------------------- */
/* Copyright 2002-2010, GridWay Project Leads (GridWay.org)                   */
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
"USAGE\n gwps [-h] [-u user] [-r host] [-A AID] [-s job_state] [-o output_format] [-c delay] [-nfx] [job_id]\n\n"
"SYNOPSIS\n"
"  Prints information about all the jobs in the GridWay system (default)\n\n"
"OPTIONS\n"
"  -h               print this help\n"
"  -u user          monitor only jobs owned by user\n"
"  -r host          monitor only jobs executed in host\n"
"  -A AID           monitor only jobs part of the array AID\n"
"  -s job_state     monitor only jobs in state job_state (see JOB STATES)\n"
"  -o output_format define output information (see FIELD INFORMATION)\n" 
"  -c delay         refresh job information every delay seconds\n"
"  -n               do not print the header\n"
"  -f               full format\n"
"  -x               xml format\n"
"  job_id           only monitor this job_id\n\n"
"FIELD INFORMATION\n"
"  USER     (u)  owner of this job\n"
"  JID      (J)  job unique identification assigned by the Gridway system\n"
"  AID      (i)  array unique identification, only relevant for array jobs\n"
"  TID      (i)  task identification, ranges from 0 to TOTAL_TASKS -1,\n"
"                only relevant for array jobs\n"
"  FP       (p)  fixed priority of the job\n"
"  TYPE     (y)  type of job (simple, multiple or mpi)\n"
"  NP       (n)  number of processors\n"
"  DM       (s)  dispatch Manager state, one of: pend, hold, prol, prew,\n"
"                wrap, epil, canl, stop, migr, done, fail\n"
"  EM       (e)  execution Manager state (Globus state): pend, susp, actv,\n"
"                fail, done\n"
"  RWS      (f)  flags: \n"
"                   - R: times this job has been restarted\n" 
"                   - W: number of processes waiting for this job\n"
"                   - S: re-schedule flag\n"
"  START    (t|T)  the time the job entered the system\n"
"  END      (t|T)  the time the job reached a final state (fail or done)\n"
"  EXEC     (t|T)  total execution time, includes suspension time in the\n"
"                  remote queue system\n"
"  XFER     (t|T)  total file transfer time, includes stage-in and stage-out\n"
"                  phases\n"
"  EXIT     (x)    job exit code\n"
"  TEMPLATE (j)    filename of the job template used for this job\n"
"  HOST     (h)    hostname where the job is being executed\n\n"
"  Note: 't' option only prints time and 'T' also writes the date.\n\n"
"JOB STATES\n"
"  PENDING (i)\n"
"  PROLOG  (p)\n"
"  HOLD    (h)\n"
"  WRAPPER (w)\n"
"  EPILOG  (e)\n"
"  STOP    (s)\n"
"  KILL    (k)\n"
"  MIGRATE (m)\n"
"  ZOMBIE  (z)\n"
"  FAILED  (f)\n";


const char * susage =
"usage: gwps [-h] [-u user] [-r host] [-A AID] [-s job_state] [-o output_format] [-c delay] [-nfx] [job_id]\n";

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
  	int               c = 0, n = 0, f = 0, x = 0;
    int               delay = 0;
  	gw_client_t *     gw_session;
	gw_msg_job_t      job_status;
	struct sigaction  act;
  	gw_return_code_t  rc;
  	char *			  hostname     = NULL;
  	char *			  username     = NULL;
    char *			  outoption	   = NULL;    	
  	char 			  jobstate	   = '&'; // If not set to other value all job states will be printed
  										  // (see gw_check_state in gw_cmds_common.c)
    int               i;
    int  	          array_id     = -1;  										    										  
										  
  	
	/* ---------------------------------------------------------------- */
	/* Parse arguments                                                  */
	/* ---------------------------------------------------------------- */
	
    opterr = 0;
    optind = 1;
	
    while((opt = getopt(argc, argv, ":nfxhc:u:r:s:o:A:")) != -1)
        switch(opt)
        {
            case 'c': c  = 1;
                delay = atoi(optarg);
                break;
            case 'n': n = 1;
                break;  
            case 'f': f = 1;
                break;  
		    case 'x': x = 1;
                break;  
            case 'u':                     	
            	username = strdup(optarg);         
                break;
            case 'r':                     	
            	hostname = strdup(optarg);        
                break; 
            case 's':                     	
            	jobstate = strdup(optarg)[0];
            	switch(jobstate)
            	{
            		case 'i':
            		case 'p':
            		case 'h':
            		case 'w':
            		case 'e':
            		case 's':
            		case 'k':
            		case 'm':
            		case 'z':
            		case 'f':
            			break;
            		default:
            			printf("ERROR: Job state must be one of {i,p,h,w,e,s,k,m,z,f}\n");
		            	printf("%s", susage);
		                return (-1);             		
            			break;	
            	}
                break; 
            case 'o':
            	outoption = strdup(optarg);
            	
            	int seen_t=0;
            	
            	for(i=0;i<strlen(outoption);i++)
            		switch(outoption[i])
            		{
						case 't':
							if(seen_t)
							{
		            			printf("ERROR: options t and T are mutually exclusive\n");
				            	printf("%s", susage);
				                return (-1);
							}
				            seen_t++;								
							break;
						case 'T':
							if(seen_t)
							{
		            			printf("ERROR: options t and T are mutually exclusive\n");
				            	printf("%s", susage);
				                return (-1);
							}
				            seen_t++;								
							break;           			
            			case 'e':
            			case 's':
            			case 'u':
            			case 'j':
						case 'h':
						case 'x':
						case 'i':  
						case 'f':
						case 'p':
                        case 'J':
                        case 'y':
                        case 'n':
            				break;
	            		default:
	            			printf("ERROR: Output format must be constructed with {e,s,u,j,t,h,x,i,p,J,y,n}\n");
			            	printf("%s", susage);
			                return (-1);             		
	            			break;
            		}        
                break; 
            case 'A':
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
                	
	if ( optind < argc)
		job_id = atoi(argv[optind]);


	/* ---------------------------------------------------------------- */
	/* Connect to GWD                                                   */
	/* ---------------------------------------------------------------- */

	gw_session = gw_client_init();
	
	if ( gw_session == NULL )
	{
		if(username != NULL)
			free(username);
		if(hostname != NULL)
			free(hostname);	
		if(outoption != NULL)
			free(outoption);					
			
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

	    if (job_id != -1){
		  rc = gw_client_job_status(job_id, &job_status);
		}
		else {
		  rc = gw_client_job_status_all( );
		}

    	if (rc == GW_RC_SUCCESS)
        {
		    if (f)
            {
		        if (job_id != -1)
		        	gw_client_print_status_full(&job_status);
    			else	   	    
	    	    	gw_client_print_pool_status_full(username, hostname, jobstate, array_id);
            }
			else if (x)
			{
			  char command[]=GW_PS_COMMAND_XML;
			  char command_open[GW_PS_COMMAND_OPEN_SIZE_XML];

			  if ( username == NULL && hostname == NULL ){
				sprintf (command_open, "%s", command);
			  } 
			  else if ( username != NULL && hostname == NULL ){
				sprintf (command_open, "%s USERNAME=\"%s\"", command, username);
			  } 
			  else if ( username == NULL && hostname != NULL ){
				sprintf (command_open, "%s HOSTNAME=\"%s\"", command, hostname);
			  } 
			  else if ( username != NULL && hostname != NULL ){
				sprintf (command_open, "%s USERNAME=\"%s\" HOSTNAME=\"%s\"", command, username, hostname);
			  }
			  gw_print_xml_header(command_open);		  
			  if (job_id != -1){
				gw_client_print_status_xml(&job_status);
			  }
			  else {
				gw_client_print_pool_status_xml(username, hostname, jobstate, array_id);
			  }
			  gw_print_xml_footer(command);
			}
            else
			{
            	if (!n)
            		gw_client_print_status_header(outoption);

		        if (job_id != -1)
		        	gw_client_print_status(&job_status, outoption);
    			else	   	    
	    	    	gw_client_print_pool_status(username, hostname, jobstate, outoption, array_id);
            }
        }  
	    else
	    {
			if(username != NULL)
				free(username);
			if(hostname != NULL)
				free(hostname);	 
		    if(outoption != NULL)
			    free(outoption);					
							   	
	    	fprintf(stderr,"FAILED: %s\n",gw_ret_code_string(rc)); 
           	
	        gw_client_finalize();
	        return -1;
	    }  

		sleep(delay);	    
		
	} while(c);
	
	if(username != NULL)
		free(username);
	if(hostname != NULL)
		free(hostname);
	if(outoption != NULL)
		free(outoption);					
	
	return 0;
}

