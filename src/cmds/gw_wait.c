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

/* ------------------------------------------------------------------------- */
/* GLOBAL VARIABLES                                                          */
/* ------------------------------------------------------------------------- */

const char * usage =
"USAGE\n gwwait [-h] [-a] [-v] [-k] [-t timeout] <job_id [job_id2 ...]|-A array_id>\n\n"
"SYNOPSIS\n"
"  Waits for a job\n\n"
"OPTIONS\n"
"  -h             print this help\n"
"  -a             any. Return when the first job of the list or array finishes\n"
"  -v             print job exit code\n"
"  -k             keep jobs. They remain in fail or done states in the\n"
"                 GridWay system.\n"
"                 By default, jobs are killed and their resources freed\n"
"  -t timeout     do not wait more than timeout seconds. A negative value\n"
"                 means waiting for ever (default)\n"
"                 By default, jobs are killed and their resources freed\n"
"  job_id [job_id2 ...] job identification or list of jobs ids as provided by gwps\n"
"  -A array_id    array identification as provided by gwps\n";


const char * susage =
"usage: gwwait [-h] [-a] [-v] [-k] [-t timeout] <job_id ...| -A array_id>\n";

extern char *optarg;
extern int   optind, opterr, optopt;
extern gw_client_t gw_client;


#define MAX_JOBS_WAIT 1000

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
    int                job_id[MAX_JOBS_WAIT];
    int                i;
    int                number_of_jobs;
    int                array_id = -1;
    int                error = 0;
  	char               opt;
  	int                exit_code;
  	int *              exit_codes;  	
  	int                a = 0, A = 0, v = 0, k = 0;
  	signed long        timeout = -1;
  	gw_client_t *      gw_session;
	gw_boolean_t       any = GW_FALSE;
	
    struct sigaction   act;
    gw_return_code_t   rc;
  	
	/* ---------------------------------------------------------------- */
	/* Parse arguments                                                  */
	/* ---------------------------------------------------------------- */
	
    opterr = 0;
    optind = 1;
	
    while((opt = getopt(argc,argv,":hkavt:A:"))!= -1)
        switch(opt)
        {
            case 'h':
            	printf("%s", usage);
                exit(0);
                break;
            case 'v': v = 1;
                break;                
            case 'a': a = 1;
            	any = GW_TRUE;
                break;    
            case 'A': A = 1;
	            array_id = atoi(optarg);
                break;
            case 't':
                timeout = atoi(optarg);               
                break;
            case 'k': k = 1;
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
	/* Set job id array                                                 */
	/* ---------------------------------------------------------------- */
    
    if (!A)
    {
		if (optind < argc)
		{
			number_of_jobs = 0;
			while ( optind < argc )
			{
				job_id[number_of_jobs++] = atoi(argv[optind++]);
				
				if (number_of_jobs >= (MAX_JOBS_WAIT - 1) )
				{
					fprintf(stderr,"FAILED: Max number of jobs reached\n");
					error = 1;
				}
			}
			job_id[number_of_jobs] = -1;
		}
		else
		{
	    	printf("%s",susage);
	    	error = 1;
		}
    }
    else
    {
		rc = gw_client_job_status_all( );

    	if (rc == GW_RC_SUCCESS)
        {
        	number_of_jobs = 0;
        	
       		for (i=0;i<gw_client.number_of_jobs;i++)
				if (gw_client.job_pool[i] != NULL)
					if (gw_client.job_pool[i]->array_id == array_id)
					{
						job_id[number_of_jobs++] = gw_client.job_pool[i]->id;
						if (number_of_jobs>=(MAX_JOBS_WAIT - 1))
						{
							fprintf(stderr,"FAILED: Max number of jobs reached\n");
							error = 1;
						} 
					}
					
			job_id[number_of_jobs] = -1;
			
			if (number_of_jobs == 0)
			{
				fprintf(stderr,"FAILED: failed bad array id\n");
				error = 1;
			} 	
        }
        else
        {
   	   		fprintf(stderr,"FAILED: %s\n",gw_ret_code_string(rc)); 
        	error = 1;
        }
    }

	if (error)
	{
    	gw_client_finalize();
    	return -1;
	}

	/* ---------------------------------------------------------------- */
	/* Wait for the jobs                                                */
	/* ---------------------------------------------------------------- */
	
	if ( number_of_jobs == 1 )
	{
		rc = gw_client_wait(job_id[0], &exit_code, timeout);
			
		if ( rc == GW_RC_SUCCESS )
		{
			if (v)
			{
				printf("%i\n",exit_code);
			}
			
			if (!k)
			{
				gw_client_job_signal(job_id[0],GW_CLIENT_SIGNAL_KILL,GW_FALSE);
			}
		}
	}
	else
	{
		rc = gw_client_wait_set(job_id,&exit_codes,any,timeout);
			
		if (rc == GW_RC_SUCCESS)
		{
			if (any)
			{
				if (v)
				{
					printf("%i: %i\n",job_id[0],exit_codes[0]);
				}
				
				if (!k)
				{
					gw_client_job_signal(job_id[0],GW_CLIENT_SIGNAL_KILL,GW_FALSE);
				}									
			}
			else
			{
				for (i=0;i<number_of_jobs;i++)
				{
					if (v)
					{
						printf("%-4i: %i\n",job_id[i],exit_codes[i]);
					}
				
					if (!k)
					{
						gw_client_job_signal(job_id[i],GW_CLIENT_SIGNAL_KILL,GW_FALSE);
					}
				}
			}
		}
			
		free(exit_codes);	
	}
	
	gw_client_finalize();
	
	if ( rc != GW_RC_SUCCESS)
	{
   		fprintf(stderr,"FAILED: %s\n",gw_ret_code_string(rc)); 
        return -1;										
	}
	else
		return 0;
}
