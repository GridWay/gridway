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
#include "gw_scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

static void gw_scheduler_init (gw_scheduler_t * sched);

static FILE *fd_log;

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
    
    char    str[320];
    
    char    action[10];
    
	char    id1[7];
    char    id2[7];
    char    xfr[10];
    char    sus[80];
    char    exe[10];
    char    tsk[10];
    
    int                   hid, uid, jid, aid;
    int                   uslots, ajobs, nice;
    int                   rjobs,tasks;
    gw_migration_reason_t reason;
    float                 txfr,texe,tsus;
        
    char *  GW_LOCATION;
    char *  log;
    char *  error_str;
    int     length;
        
    gw_scheduler_t sched;  	
  	
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
	    log    = (char *) malloc (sizeof(char)*(length + 15));
	    
        sprintf(log, "%s/var/sched.log", GW_LOCATION);
        
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
  	
  	gw_scheduler_init(&sched);

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

        rc = sscanf(str,"%s %s %s %s %s %s %[^\n]",action,id1,id2,xfr,sus,exe,tsk);

#ifdef GWSCHEDDEBUG
        gw_scheduler_print('D',"Message received from gwd \"%s %s %s %s %s %s %s\"\n",
            action,id1,id2,xfr,sus,exe,tsk);
#endif        
        if (strcmp(action, "INIT") == 0 )
        {
        	if (error == 0)
        	    printf("INIT - SUCCESS -\n");
        	else
        	{
        		printf("INIT - FAILURE %s\n",error_str);
        	    end = 1;	
        	}
        }
        else if (strcmp(action, "FINALIZE") == 0 )
        {
        	printf("FINALIZE - SUCCESS -");
        	end = 1;
        }
        else if (strcmp(action, "HOST_MONITOR") == 0 )
        {
        /* Add or update a given host format is:
         * HOST_MONITOR HID USLOTS RJOBS NAME - 
         */
            hid    = atoi(id1);
            uslots = atoi(id2);
            rjobs  = atoi(xfr);
            
            gw_scheduler_add_host(&sched,hid,uslots,rjobs,sus);
        }
        else if (strcmp(action, "USER_ADD") == 0 )
        {
        /* Add a user:
         * USER_ADD UID ASLOTS RSLOTS NAME - 
         */        	
        	uid   = atoi(id1);
        	ajobs = atoi(id2);
        	rjobs = atoi(xfr);
        	
        	gw_scheduler_add_user(&sched,uid,ajobs,rjobs,sus);
        }
        else if (strcmp(action, "USER_DEL") == 0 )
        {
        /* Remove an user
         * USER_DEL UID - - - - 
         */        	
        	uid   = atoi(id1);
        	        	
        	gw_scheduler_del_user(&sched,uid);
        }        
        else if (strcmp(action, "JOB_DEL") == 0 )
        {
        /* Remove an job
         * JOB_DEL JID - - - - 
         */        	
        	jid   = atoi(id1);
        	
        	gw_scheduler_job_del(&sched,jid);
        }                
        else if (strcmp(action, "TASK_DEL") == 0 )
        {
        /* Remove an job
         * TASK_DEL AID - - - - 
         */        	
        	aid   = atoi(id1);
        	
        	gw_scheduler_array_del(&sched,aid,1);
        }                     
        else if (strcmp(action, "JOB_FAILED") == 0 )
        {
        /* A job has failed, update user host statistics
         * JOB_FAILED HID UID REASON - -
         */
          	hid    = atoi(id1);
          	uid    = atoi(id2);
          	reason = (gw_migration_reason_t) atoi(xfr);
          	
          	gw_scheduler_job_failed(&sched,hid, uid,reason);
        }                
        else if (strcmp(action, "JOB_SUCCESS") == 0 )        
        {
        /* A job has been successfully executed, update user host statistics
         * JOB_SUCCESS HID UID XFR SUS EXE 
         */
          	hid  = atoi(id1);
          	uid  = atoi(id2);
          	
          	txfr = atof(xfr);
          	tsus = atof(sus);
          	texe = atof(exe);
            
            gw_scheduler_job_success(&sched,hid,uid,txfr,tsus,texe);

        }
        else if (strcmp(action, "JOB_SCHEDULE") == 0 )        
        {
        /* A job need schedule add it to the job list
         * JOB_SCHEDULE JID AID REASON NICE UID
         */
          	jid    = atoi(id1);
          	aid    = atoi(id2);          	
          	reason = (gw_migration_reason_t) atoi(xfr);
          	nice   = atoi(sus);
          	uid    = atoi(exe);
            
            gw_scheduler_job_add(&sched,jid,aid,reason,nice,uid);
        }
        else if (strcmp(action, "ARRAY_SCHEDULE") == 0 )        
        {
        /* A job need schedule add it to the job list
         * ARRAY_SCHEDULE JID AID REASON NICE UID TASKS
         */
          	jid    = atoi(id1);
          	aid    = atoi(id2);          	
          	reason = (gw_migration_reason_t) atoi(xfr);
          	nice   = atoi(sus);
          	uid    = atoi(exe);
          	tasks  = atoi(tsk);
            
            gw_scheduler_array_add(&sched,jid,aid,reason,nice,uid,tasks);
        }    
        else if (strcmp(action, "SCHEDULE") == 0 )
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
        	
       		    scheduler(&sched,user_arg);
        	}	
       		
       		printf("SCHEDULE_END - SUCCESS -\n");
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

    va_start (ap, str_format);

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
                                
inline void gw_scheduler_ctime(time_t the_time, char *str)
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
