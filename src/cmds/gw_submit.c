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

/* ------------------------------------------------------------------------- */
/* GLOBAL VARIABLES                                                          */
/* ------------------------------------------------------------------------- */

const char * usage =
"\n gwsubmit <-t template> [-n tasks] [-h] [-v] [-o] [-d \"id1 id2 ...\"]\n\n"
"SYNOPSIS\n"
"  Submit a job or an array job (if the number of tasks is defined) to gwd\n\n"
"OPTIONS\n"
"  -h                prints this help\n"
"  -t <template>     the template file describing the job\n"
"  -n <tasks>        submit an array job with the given number of tasks\n"
"                    all the jobs in the array will use the same template\n"
"  -v                print to stdout the job ids returned by gwd.\n" 
"  -o                hold job on submission.\n" 
"  -d \"id1 id2...\" job dependencies. Submit the job on hold state, and\n"
"                    release it once jobs with id1,id2,.. have finished\n";

const char * susage =
"usage: gwsubmit <-t template> [-n tasks] [-h] [-v] [-o] [-d \"id1 id2...\"]\n";

extern char *optarg;
extern int   optind, opterr, optopt;

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

int main(int argc, char **argv)
{
    int              job_id;
    int              array_id;
    int *            job_ids;
    int              tasks;
    char *           template;
    char *           deps_str;
    char *           num;
    int              deps[GW_JT_DEPS];
    char             opt;
    char             rpath[PATH_MAX];
    int              t = 0, v = 0, n = 0, o = 0, d = 0;
    int              i;
    gw_client_t *    gw_session;
    gw_job_state_t   init_state;

    gw_return_code_t rc;

    /* ---------------------------------------------------------------- */
    /* Parse arguments                                                  */
    /* ---------------------------------------------------------------- */
    
    opterr = 0;
    optind = 1;
    
    if(argc < 2)
    {
        fprintf(stderr,"usage: %s\n", susage);
        exit(1);
    }

    while((opt = getopt(argc, argv, ":vhot:n:d:")) != -1)
        switch(opt)
        {
            case 't': t  = 1;
                template = optarg;
                break;
            case 'o': o = 1;
                break;    
            case 'n': n = 1;
                tasks   = atoi(optarg);
                break;
            case 'v': v = 1;
                break;          
            case 'd': d = 1;
                deps_str= strdup(optarg);
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

    if ( !t )
    {
        printf("%s", susage);
        exit(1);
    }
        
    /* ---------------------------------------------------------------- */
    /* Get template                                                     */
    /* ---------------------------------------------------------------- */

    if(realpath(template,rpath) == NULL)
    {
        perror("invalid template path");
        return -1;
    }  

    if (o)
    	init_state = GW_JOB_STATE_HOLD;
    else
        init_state = GW_JOB_STATE_PENDING;
           
    if (d)
    {
   		i = 0;
    	deps[0] = -1;	
   		num = strtok (deps_str," ");
			
		while ( (num != NULL) && (i<(GW_JT_DEPS -1)) )
		{
			deps[i++] = atoi(num);
    		num = strtok (NULL," ");
		}
			
		deps[i] = -1;
		free(deps_str);    	
    }
    else
    	deps[0] = -1;       
           

    /* ---------------------------------------------------------------- */
    /* Connect to GWD                                                   */
    /* ---------------------------------------------------------------- */

    gw_session = gw_client_init();
    
    if ( gw_session == NULL )
    {
        fprintf(stderr,"Could not connect to gwd\n");
        return (-1);
    }

    /* ---------------------------------------------------------------- */
    /* Submit the job                                                   */
    /* ---------------------------------------------------------------- */
    
    if (!n) 
    {                
        rc = gw_client_job_submit(rpath,init_state,&job_id,deps);
        
        if (rc == GW_RC_SUCCESS)
        {
            if (v)
                printf("JOB ID: %d\n",job_id);
            
            gw_client_finalize();            
            return 0;
        }  
        else
        {
            fprintf(stderr,"FAILED: %s\n",gw_ret_code_string(rc)); 
               
            gw_client_finalize();
            return -1;
        }  
    }  
    
    rc = gw_client_array_submit(rpath,tasks,init_state,&array_id,&job_ids,deps);
    
    if (rc == GW_RC_SUCCESS)
    {
        if (v)
        {
            printf("ARRAY ID: %d\n\n",array_id);
            printf("%-4s %-4s","TASK","JOB");
            printf("\n");
               
            for (i=0;i<tasks;i++)
                printf("%-4d %-4d\n",i,job_ids[i]);
            printf("\n");
        }
        
        free(job_ids);
        
        gw_client_finalize();    
        return 0;
    }
    else
    {
        fprintf(stderr,"FAILED: %s\n",gw_ret_code_string(rc));
         
        gw_client_finalize();
        return -1;
    }  
}
