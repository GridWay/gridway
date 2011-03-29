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
#include "gw_scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>

static void gw_scheduler_init (gw_scheduler_t * sched);

static FILE *fd_log;

#define GW_SCHED_FIELD_WIDTH 80
#define GW_SCHED_MSG_WIDTH   480

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_scheduler_loop(gw_scheduler_function_t scheduler, void *user_arg)
{
	int     end   = 0;
	int     error = 0;
	
    fd_set  in_pipes;
    int     rc,j,i;
    char    c;
    
    char    str[GW_SCHED_MSG_WIDTH];
    
    char    act[GW_SCHED_FIELD_WIDTH];    
	char    id1[GW_SCHED_FIELD_WIDTH];
    char    id2[GW_SCHED_FIELD_WIDTH];
    char    at1[GW_SCHED_FIELD_WIDTH];
    char    at2[GW_SCHED_FIELD_WIDTH];
    char    at3[GW_SCHED_FIELD_WIDTH];  
    
    int     hid, uid, jid, aid, np;
    int     uslots, ajobs, fixed_priority;
    int     rjobs;
    float   txfr,texe,tsus;
        
    char *  GW_LOCATION;
    char *  log;
    char *  conf;
    char *  error_str;
    char *  name;
    
    int     length;
    time_t  the_time, deadline;
    
    gw_scheduler_t sched;
    
    gw_client_t *         gw_session = NULL;
    gw_return_code_t      gwrc;
	gw_migration_reason_t reason;
	gw_msg_job_t          job_status;

    /* ------------------------------- */
    /*  Init environment ang log file  */
    /* ------------------------------- */

    GW_LOCATION = getenv("GW_LOCATION");
    
    if(GW_LOCATION == NULL)
    {
    	error = 1;
    	error_str = strdup("GW_LOCATION environment variable is undefined.");
    	
    	gw_scheduler_print('E',"%s\n",error_str);
    }
    else
    {
        length = strlen(GW_LOCATION);
        
	    log =(char *) malloc(sizeof(char)*(length + sizeof(GW_VAR_DIR) + 12));
	    conf=(char *) malloc(sizeof(char)*(length + sizeof(GW_ETC_DIR) + 13));
	    
        sprintf(log, "%s/" GW_VAR_DIR "/sched.log", GW_LOCATION);
        sprintf(conf,"%s/" GW_ETC_DIR "/sched.conf",GW_LOCATION);
        
    	rc = truncate(log, 0);
	
        fd_log = fopen(log,"a");
            
        if (fd_log == NULL)
        {
            error     = 1;
            error_str = strdup(strerror(errno));	
                
     	    gw_scheduler_print('E',"Could not open file %s - %s\n",
	                log,error_str);                
        }
        else
            setbuf(fd_log,NULL);
                
        free(log);
    }
          	
  	setbuf(stdout,NULL);  	

    /* ----------------------------------- */
    /*  Scheduler Initialization           */
    /* ----------------------------------- */
  	
  	gw_scheduler_init(&sched);
  	
  	gw_sch_conf_init(&(sched.sch_conf));

    rc = gw_sch_loadconf(&(sched.sch_conf), conf);
    
    if (rc != 0 )
    {
   	    gw_scheduler_print('E',"Parsing scheduler configuration file %s, using defaults.\n",conf);
    }
    
    free(conf);
    
    the_time = time(NULL);
    
	sched.next_user_window = the_time + 
                            (time_t) (sched.sch_conf.window_size * 86400);
                            
	sched.next_host_window = the_time + (time_t) 86400;
                               
    gw_scheduler_print('I',"Scheduler successfully started.\n");

    /* ----------------------------------- */
    /*  Scheduler Loop                     */
    /* ----------------------------------- */
	
    while (!end)
    {
        FD_ZERO(&in_pipes);
        FD_SET (0,&in_pipes);                         

        rc = select(1, &in_pipes, NULL, NULL, NULL);

        if (rc == -1)
        {
            end = 1;
            gw_scheduler_print('E',"Error in select() - %s\n",strerror(errno));
        }
                               
        j = 0;

        do
        {
            rc = read(0, (void *) &c, sizeof(char));
            str[j++] = c;
            
        }while ( rc > 0 && c != '\n' );

        str[j] = '\0';    

        if (rc <= 0)
        {
            end = 1;
        }

        rc = sscanf(str,"%s %s %s %s %s %[^\n]",act,id1,id2,at1,at2,at3);

#ifdef GWSCHEDDEBUG
        gw_scheduler_print('D',"Message received from gwd \"%s %s %s %s %s %s\"\n",
            act,id1,id2,at1,at2,at3);
#endif        
        if (strcmp(act, "INIT") == 0 )
        {        	
        	if (error == 0)
        	    printf("INIT - SUCCESS -\n");
        	else
        	{
        		printf("INIT - FAILURE %s\n",error_str);
        	    end = 1;	
        	}
        }
        else if (strcmp(act, "FINALIZE") == 0 )
        {
       		gw_client_finalize();
       		
        	printf("FINALIZE - SUCCESS -");
        	end = 1;
        }
        else if (strcmp(act, "HOST_MONITOR") == 0 )
        {
        /* Add or update a given host:
         * HOST_MONITOR HID USLOTS RJOBS NAME - 
         */
            hid    = atoi(id1);
            uslots = atoi(id2);
            rjobs  = atoi(at1);
            name   = at2;
            
            gw_scheduler_add_host(&sched,hid,uslots,rjobs,name);
        }
        else if (strcmp(act, "USER_ADD") == 0 )
        {
        /* Add a user:
         * USER_ADD UID ASLOTS RSLOTS NAME - 
         */        	
        	uid   = atoi(id1);
        	ajobs = atoi(id2);
        	rjobs = atoi(at1);
        	name  = at2;
        	
        	gw_scheduler_add_user(&sched,uid,ajobs,rjobs,name);
        }
        else if (strcmp(act, "USER_DEL") == 0 )
        {
        /* Remove an user
         * USER_DEL UID - - - - 
         */        	
        	uid   = atoi(id1);
        	        	
        	gw_scheduler_del_user(&sched,uid);
        }        
        else if (strcmp(act, "JOB_DEL") == 0 )
        {
        /* Remove an job
         * JOB_DEL JID - - - - 
         */        	
        	jid   = atoi(id1);
        	
        	gw_scheduler_job_del(&sched,jid,0);
        }          
        else if (strcmp(act, "JOB_FAILED") == 0 )
        {
        /* A job has failed, update user host statistics
         * JOB_FAILED HID UID REASON - -
         */
          	hid    = atoi(id1);
          	uid    = atoi(id2);
          	reason = (gw_migration_reason_t) atoi(at1);
          	
          	gw_scheduler_job_failed(&sched,hid,uid,reason);
        }                
        else if (strcmp(act, "JOB_SUCCESS") == 0 )        
        {
        /* A job has been successfully executed, update user host statistics
         * JOB_SUCCESS HID UID XFR SUS EXE 
         */
          	hid  = atoi(id1);
          	uid  = atoi(id2);
          	
          	txfr = atof(at1);
          	tsus = atof(at2);
          	texe = atof(at3);
            
            gw_scheduler_job_success(&sched,hid,uid,txfr,tsus,texe);
        }
        else if (strcmp(act, "JOB_SCHEDULE") == 0 )        
        {        	
        /* A job need schedule add it to the job list
         * JOB_SCHEDULE JID AID UID REASON -
         */
          	jid    = atoi(id1);
          	aid    = atoi(id2);
         	uid    = atoi(at1);
          	reason = (gw_migration_reason_t) atoi(at2);


        	if (gw_session == NULL)
        	{
        	    gw_session = gw_client_init();
        	    
				if (gw_session == NULL)
				{
					gw_scheduler_print('E',"Error creating a GW session.\n");
				}
        	}

          	gwrc = gw_client_job_status(jid, &job_status);
          	
          	if ( gwrc == GW_RC_SUCCESS )
          	{
          		fixed_priority = job_status.fixed_priority;
          		np             = job_status.np;
                deadline       = job_status.deadline;
          	}
          	else
          	{
         		gw_scheduler_print('E',"Error getting job information, will use default values.\n");
          		fixed_priority = 0;
          		np             = 1;
                deadline       = 0;
          	}
          	
          	gw_scheduler_job_add(&sched,jid,aid,np,reason,
                    fixed_priority,uid,deadline);	
        }
        else if (strcmp(act, "SCHEDULE") == 0 )
        {
#ifdef GWSCHEDDEBUG
           gw_scheduler_print('D',"JOBS:%i HOSTS:%i USERS:%i\n",
               sched.num_jobs,sched.num_hosts,sched.num_users);
#endif        	
        	if (    (sched.num_hosts > 0) 
        	     && (sched.num_users > 0) 
        	     && (sched.num_jobs  > 0))
        	{
        	    for (i=0;i<sched.num_hosts;i++)
        	        sched.hosts[i].dispatched = 0;

        	    for (i=0;i<sched.num_users;i++)
        	        sched.users[i].dispatched = 0;        	
        	
        	    gw_scheduler_matching_arrays(&sched);
        	    
        	    if (sched.sch_conf.disable == 0)
        	       gw_scheduler_job_policies (&sched);
        	
       		    scheduler(&sched,user_arg);
        	}
        	
            the_time = time(NULL);
            
        	if ( the_time > sched.next_user_window)
        	{
#ifdef GWSCHEDDEBUG
                gw_scheduler_print('D',"Updating window shares.\n");
#endif
        		sched.next_user_window = time(NULL) + 
       		                     (time_t) (sched.sch_conf.window_size * 86400);
       		                        
        		gw_scheduler_user_update_windows(&sched);
        	}
#ifdef HAVE_LIBDB        	
        	if ( the_time > sched.next_host_window)
        	{
#ifdef GWSCHEDDEBUG
                gw_scheduler_print('D',"Updating host usage.\n");
#endif	
        		sched.next_host_window = time(NULL) + (time_t) 86400;
       		                     
				gw_scheduler_update_usage_host(&sched);
        	}
#endif       		
       		printf("SCHEDULE_END - SUCCESS -\n");
        }
        else
        {
            gw_scheduler_print('E',"Unknown action from core %s\n.",act);        	
        }
    }
    
    if (error == 0)
        fclose(fd_log);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

static void gw_scheduler_init (gw_scheduler_t * sched)
{  
    sched->num_users = 0;
    sched->users     = NULL;
  
    sched->num_hosts = 0;
    sched->hosts     = NULL;

    sched->num_jobs  = 0;
    sched->jobs      = NULL;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_scheduler_print (const char mode, const char *str_format,...)
{
    va_list ap;
    time_t  the_time;
    
    char str[26];

    va_start(ap, str_format);

    if (fd_log != NULL)
    {
        the_time = time(NULL);

#ifdef GWSOLARIS
        ctime_r(&(the_time),str,sizeof(char)*26);
#else
        ctime_r(&(the_time),str);
#endif

        str[24]='\0';

        fprintf(fd_log,"%s [%c]: ", str, mode);
        vfprintf(fd_log,str_format,ap);
    }
        
    return;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
                                
void gw_scheduler_ctime(time_t the_time, char *str)
{
	int i;
	
#ifdef GWSOLARIS
        ctime_r(&(the_time),str,sizeof(char)*26);
#else
        ctime_r(&(the_time),str);
#endif

	for (i=0;i<8;i++)
		str[i]=str[i+11];
		
	str[8]='\0';
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
