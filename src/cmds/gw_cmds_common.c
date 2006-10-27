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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "gw_client.h"
#include "gw_cmds_common.h"

extern gw_client_t gw_client;

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

char *gw_print_time(time_t t, char *the_time)
{
    struct tm *tm_s;

    if(t<=0)
        return "0:00:00";
    else
    {
        tm_s = gmtime(&t);
        sprintf(the_time,"%1i:%02i:%02i",tm_s->tm_hour,tm_s->tm_min,
                tm_s->tm_sec);
        return(the_time);
    }
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

char *gw_print_time2(time_t t, char *the_time)
{
    struct tm *tm_s;
        
    if(t<=0)
        return "--:--:--";
    else
    {
        tm_s = localtime(&t);
        sprintf(the_time,"%02i:%02i:%02i",tm_s->tm_hour,tm_s->tm_min,
                tm_s->tm_sec);
        return(the_time);
    }
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

char *gw_print_date_and_time(time_t t, char *the_time)
{
    struct tm *tm_s;
            
    if(t<=0)
        return "--/-- --:--:--";
    else
    {
        tm_s = localtime(&t);
        sprintf(the_time,"%02i/%02i %02i:%02i:%02i",tm_s->tm_mday,tm_s->tm_mon+1,tm_s->tm_hour,
                tm_s->tm_min,tm_s->tm_sec);                
        return(the_time);
    }
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

int gw_check_state(char jobstate_from_user, gw_job_state_t current_job_state)
{	
	if(jobstate_from_user == '&')
		return 1;
	
	switch(jobstate_from_user)
	{
		case 'i':
			if( current_job_state == GW_JOB_STATE_PENDING )
				return 1;
			break;
		case 'p':
			if( current_job_state == GW_JOB_STATE_PROLOG )
				return 1;
			break;		
		case 'h':
			if( current_job_state == GW_JOB_STATE_HOLD )
				return 1;
			break;		
		case 'w':
			if( current_job_state == GW_JOB_STATE_WRAPPER )
				return 1;
			break;		
		case 'e':
			if( (current_job_state == GW_JOB_STATE_EPILOG)
			 || (current_job_state == GW_JOB_STATE_EPILOG_STD)
			 || (current_job_state == GW_JOB_STATE_EPILOG_RESTART)
			 || (current_job_state == GW_JOB_STATE_EPILOG_FAIL))
				return 1;
			break;		
		case 's':
			if( (current_job_state == GW_JOB_STATE_STOP_CANCEL)
			 || (current_job_state == GW_JOB_STATE_STOP_EPILOG)
			 || (current_job_state == GW_JOB_STATE_STOPPED))
				return 1;
			break;			
		case 'k':
			if( (current_job_state == GW_JOB_STATE_KILL_CANCEL)
			 || (current_job_state == GW_JOB_STATE_KILL_EPILOG))
				return 1;
			break;		
		case 'm':
			if( (current_job_state == GW_JOB_STATE_MIGR_CANCEL)
			 || (current_job_state == GW_JOB_STATE_MIGR_PROLOG)
			 || (current_job_state == GW_JOB_STATE_MIGR_EPILOG))
				return 1;
			break;			
		case 'z':
			if( current_job_state == GW_JOB_STATE_ZOMBIE )
				return 1;
			break;		
		case 'f':	
			if( current_job_state == GW_JOB_STATE_FAILED )
				return 1;
			break;		
		default:
			return 0;	
	}
	return 0;
}


/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/


void gw_client_print_status_header(char *outoption)
{
    char head_string[200];
    char tmpstr[50];
    
    int i; 
    
    head_string[0]='\0';
    
    if(outoption != NULL)
    {
    	// Always show the JID
    	sprintf(tmpstr,"%-3s ","JID");
    	strncat(head_string,tmpstr,strlen(tmpstr));
    	
    	for(i=0;i<strlen(outoption);i++)
    	{
			switch(outoption[i])
			{
    			case 'e':
    				sprintf(tmpstr,"%-4s ","EM");
    				break;  
    			case 's':
    				sprintf(tmpstr,"%-4s ","DM");
    				break;
    			case 'u':
    				sprintf(tmpstr,"%-12s ","USER");
    				break;   			
    			case 'j':
    				sprintf(tmpstr,"%-15.15s ","TEMPLATE");
    				break;					    			
				case 't':
    				sprintf(tmpstr,"%-8s %-8s %-7s %-7s ","START","END","EXEC","XFER");
    				break;
				case 'T':
    				sprintf(tmpstr,"%-14s %-14s %-7s %-7s ","START","END","EXEC","XFER");
    				break;	    									    							
				case 'h':
    				sprintf(tmpstr,"%-25s ","HOST");
    				break;				
				case 'x':
    				sprintf(tmpstr,"%-4s ","EXIT");
    				break;
				case 'i':
					sprintf(tmpstr,"%-3s %-3s ","AID","TID");
    				break;
				case 'f':
					sprintf(tmpstr,"%-3s ","RWS");
    				break; 				
			}   		 		
    		strncat(head_string,tmpstr,strlen(tmpstr));
    	}
    }    
    else
	    sprintf(head_string,"%-12s %-3s %-3s %-3s %-4s %-4s %-3s %-8s %-8s %-7s %-7s "
	            "%-4s %-15s %-10s", "USER", "JID","AID","TID","DM","EM","RWS","START",
	            "END","EXEC","XFER","EXIT","TEMPLATE","HOST"); 
    bold();
    underline(); 
    printf(head_string);
    restore();
    printf("\n");
    fflush(stdout);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void gw_client_print_status(gw_msg_job_t * msg,char *outoption)
{
    char the_time[15],*template;
    int  i;

    the_time[5]='\0';

    if(strlen(msg->host)==0)
        strcpy(msg->host,"--");

    template = strrchr(msg->file,'/');
        
    if(template!=NULL)
        template++;
    else
        template = msg->file;
        
    if(outoption!=NULL)
    {
    	printf("%-3d ",msg->id);  // always show the job id
    	
    	for(i=0;i<strlen(outoption);i++)
    	{
			switch(outoption[i])
			{
    			case 'e':
    				printf("%-4s ", gw_em_state_string (msg->em_state));	
    				break;  
    			case 's':
				    printf("%-4s ", gw_job_state_string(msg->job_state));
    				break;
    			case 'u':
    				printf("%-12s ",msg->owner);
    				break;   			
    			case 'j':
    				printf("%-15.15s ", template);
    				break;					    			
				case 't':				
				    printf("%-8s ",gw_print_time2(msg->start_time,the_time));
				    printf("%-8s ",gw_print_time2(msg->exit_time ,the_time));
				    printf("%-7s ",gw_print_time (msg->cpu_time  ,the_time)); 
				    printf("%-7s ",gw_print_time (msg->xfr_time  ,the_time)); 
    				break;	
				case 'T':			
				    printf("%-14s ",gw_print_date_and_time(msg->start_time,the_time));
				    printf("%-14s ",gw_print_date_and_time(msg->exit_time ,the_time));
				    printf("%-7s ",gw_print_time (msg->cpu_time  ,the_time)); 
				    printf("%-7s ",gw_print_time (msg->xfr_time  ,the_time));
    				break;    								    							
				case 'h':				
				    printf("%-25s ",msg->host);
    				break;				
				case 'x':
				    if (msg->job_state == GW_JOB_STATE_ZOMBIE)
				        printf("%-4d ",msg->exit_code);
				    else
				        printf("%-4s ", "--");
    				break;
				case 'i':
					if(msg->array_id == -1)
				        printf("--  --  ");
				    else
				        printf("%-3d %-3d ",msg->array_id,msg->task_id);
    				break;
				case 'f':
					printf("%-1d%-1d%-1d ",msg->restarted,msg->client_waiting,msg->reschedule);
    				break; 				
			}   		 		
    	}    
    	printf("\n");	
    }
    else
    {        
		printf("%-12s ",msg->owner);
	                
	    printf("%-3d ",msg->id);
	        
	    if(msg->array_id == -1)
	        printf("--  --  ");
	    else
	        printf("%-3d %-3d ",msg->array_id,msg->task_id);  
	
	    printf("%-4s %-4s ", gw_job_state_string(msg->job_state),
	            gw_em_state_string (msg->em_state));
	    printf("%-1d%-1d%-1d",msg->restarted,msg->client_waiting,msg->reschedule);                    
	    printf(" %-8s",gw_print_time2(msg->start_time,the_time));
	    printf(" %-8s",gw_print_time2(msg->exit_time ,the_time));
	    printf(" %-7s",gw_print_time (msg->cpu_time  ,the_time)); 
	    printf(" %-7s",gw_print_time (msg->xfr_time  ,the_time)); 
	        
	    if (msg->job_state == GW_JOB_STATE_ZOMBIE)
	        printf(" %-4d %-15.15s %-20s\n",msg->exit_code, template, msg->host);
	    else
	        printf(" %-4s %-15.15s %-20s\n", "--", template, msg->host);
    }        
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void gw_client_print_pool_status(char *username, char *hostname, char jobstate, char *outoption, int array_id)
{
    int i;            
           			 		           
	if(username != NULL)
	{
		if(hostname != NULL)
		{
			/* -----   Show jobs for username @ hostname ----- */			
		 	for (i=0;i<gw_client.number_of_jobs;i++)
		 	{
		 		if (gw_client.job_pool[i] != NULL)
		 		{		 		
		    		if ((strcmp(gw_client.job_pool[i]->owner, username) == 0)
		    		&&  (strstr(gw_client.job_pool[i]->host,hostname))!=NULL) 
		    		{
		    			if(gw_check_state(jobstate,gw_client.job_pool[i]->job_state))
		    			{
		    				if(array_id == -1)			 			    		 	
		    					gw_client_print_status(gw_client.job_pool[i],outoption);
		    				else		 
		    					if(gw_client.job_pool[i]->array_id==array_id) 
		    						gw_client_print_status(gw_client.job_pool[i],outoption);
		    			}  		
		    		}			    			
		    		
		 		}	    		
		 	}
		}
	    else 				
	    {
	    	/* -----   Show jobs for username ----- */
		 	for (i=0;i<gw_client.number_of_jobs;i++)
	    		if ((gw_client.job_pool[i] != NULL) 
	    		&&   (strcmp(gw_client.job_pool[i]->owner, username) == 0))	
		    			if(gw_check_state(jobstate,gw_client.job_pool[i]->job_state))
		    			{			 			    		 	
			    			if(array_id == -1)			 			    		 	
			    				gw_client_print_status(gw_client.job_pool[i],outoption);
			    			else		 
			    				if(gw_client.job_pool[i]->array_id==array_id) 
			    					gw_client_print_status(gw_client.job_pool[i],outoption);  		
		    			}
	    				    	
	    }
		
	}
	else
	{ 
		if(hostname != NULL)
		{
			
			/* -----   Show jobs for hostname ----- */			
		 	for (i=0;i<gw_client.number_of_jobs;i++)
		 	{
		 		if (gw_client.job_pool[i] != NULL)
		 		{
		    		if (strstr(gw_client.job_pool[i]->host,hostname)!=NULL)	    		 	
		    			if(gw_check_state(jobstate,gw_client.job_pool[i]->job_state))
		    			{			 			    		 	
		    				if(array_id == -1)			 			    		 	
		    					gw_client_print_status(gw_client.job_pool[i],outoption);
		    				else		 
		    					if(gw_client.job_pool[i]->array_id==array_id) 
		    						gw_client_print_status(gw_client.job_pool[i],outoption);
		    			}  		

		 		}
		 	}			
		}
 		else
 		{
			/* -----   Show all jobs ----- */ 			
 		 	for (i=0;i<gw_client.number_of_jobs;i++)
        		if (gw_client.job_pool[i] != NULL) 	
        			if(gw_check_state(jobstate,gw_client.job_pool[i]->job_state))
        			{			 			    		 	
		    			if(array_id == -1)			 			    		 	
		    				gw_client_print_status(gw_client.job_pool[i],outoption);
		    			else		 
		    				if(gw_client.job_pool[i]->array_id==array_id) 
		    					gw_client_print_status(gw_client.job_pool[i],outoption);
        			}  		
 		}
	}
	
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void gw_client_print_history_header()
{
    char head_string[200];
    
    sprintf(head_string,"%-3s %-8s %-8s %-7s %-7s %-7s %-7s %-6s %-8s %-20s",
    "HID","START","END","PROLOG","WRAPPER","EPILOG","MIGR","REASON","QUEUE","HOST");
                        
    bold();
    underline(); 
    printf(head_string);
    restore();
    printf("\n");
    fflush(stdout);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void gw_client_print_history(gw_msg_history_t *msg_history)
{
    char the_time2[9];
    char the_time [8];    
    time_t prolog;
    time_t epilog;
    time_t wrapper;
    time_t migration;
    
    the_time2[8]='\0';
    the_time [7]='\0';    
        
    if ((msg_history->prolog_etime != 0) && (msg_history->prolog_stime != 0))
        prolog = msg_history->prolog_etime - msg_history->prolog_stime;
    else if ((msg_history->prolog_etime == 0) && (msg_history->prolog_stime != 0))
        prolog = time(NULL) - msg_history->prolog_stime;
    else 
        prolog = 0;    
        
    if ((msg_history->wrapper_etime != 0) && (msg_history->wrapper_stime != 0))
        wrapper = msg_history->wrapper_etime - msg_history->wrapper_stime;
    else if ((msg_history->wrapper_etime == 0) && (msg_history->wrapper_stime != 0))
        wrapper = time(NULL) - msg_history->wrapper_stime;
    else
        wrapper = 0;
        
    if ((msg_history->epilog_etime != 0) && (msg_history->epilog_stime != 0))
        epilog = msg_history->epilog_etime - msg_history->epilog_stime;
    else if ((msg_history->epilog_etime == 0) && (msg_history->epilog_stime != 0))
        epilog = time(NULL) - msg_history->epilog_stime;
    else
        epilog = 0;
        
    if ((msg_history->migration_etime != 0) && (msg_history->migration_stime != 0))
        migration = msg_history->migration_etime - msg_history->migration_stime;
    else if ((msg_history->migration_etime == 0) && (msg_history->migration_stime != 0))
        migration = time(NULL) - msg_history->migration_stime;
    else
        migration = 0;

    printf("%-3i ",  msg_history->host_id);
    printf("%-8s ",  gw_print_time2(msg_history->start_time,the_time2));  
    printf("%-8s ",  gw_print_time2(msg_history->exit_time ,the_time2));    
    printf("%-7s ",  gw_print_time (prolog ,the_time)); 
    printf("%-7s ",  gw_print_time (wrapper ,the_time)); 
    printf("%-7s ",  gw_print_time (epilog ,the_time)); 
    printf("%-7s ",  gw_print_time (migration ,the_time));  
    printf("%-6s ", gw_reason_string(msg_history->reason));
    printf("%-8s ",  msg_history->queue);    
    printf("%-20s\n", msg_history->em_rc);
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

void gw_client_print_host_status_header()
{
    char head_string[200];
    
    sprintf(head_string,"%-3s %-5s %-15s %-5s %4s %4s %9s %13s %12s %-20s %-20s",
    "HID","NICE","OS","ARCH","MHZ","%%CPU","MEM(F/T)","DISK(F/T)","N(U/F/T)","LRMS","HOSTNAME");
    
    bold();
    underline(); 
    printf(head_string);
    restore();
    printf("\n");
    fflush(stdout);
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

void gw_client_print_host_status(gw_msg_host_t * msg)
{
    char buffer[25];
    char *tmp;
    int freenodecount;
    int i;
    
    printf("%-3i ",msg->host_id);
    printf("%-5i ",msg->nice);    
    
    sprintf(buffer,"%s%s",msg->os_name, msg->os_version);
    printf("%-15.15s ",buffer);
    
    printf("%-5.5s ",msg->arch);
    printf("%4i ",msg->cpu_mhz);
    printf("%4i ",msg->cpu_free);

    sprintf(buffer,"%i/%i",msg->free_mem_mb,msg->size_mem_mb);    
    tmp = buffer;
    while (*tmp ==' ')
        tmp++;
    printf("%9s ",tmp);

    sprintf(buffer,"%i/%i",msg->free_disk_mb,msg->size_disk_mb);
    tmp = buffer;
    while (*tmp ==' ')
        tmp++;
    printf("%13s ",tmp);    

    freenodecount = 0;
    for (i=0; i<msg->number_of_queues; i++)
    {
        if (msg->queue_freenodecount[i] > freenodecount)
            freenodecount = msg->queue_freenodecount[i];
    }
    
    sprintf(buffer,"%i/%i/%i",msg->running_jobs,freenodecount,msg->nodecount);
    tmp = buffer;
    while (*tmp ==' ')
        tmp++;
    printf("%12s ",tmp);

    printf("%-20s ",msg->lrms_name);
    
    printf("%-20s\n",msg->hostname);    
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

void gw_client_print_host_queues(gw_msg_host_t * msg, gw_boolean_t header)
{
    char head_string[200];
    char buffer[25];
    char *tmp;    
    int  i;
    
    sprintf(head_string,"\n%-20s %-7s %-5s %-5s %-5s %-5s %-5s %-8s %-10s %-8s",
                "QUEUENAME","SL(F/T)","WALLT","CPUT","COUNT","MAXR","MAXQ",
                "STATUS","DISPATCH","PRIORITY");
    
    bold();
    underline(); 
    if (header == GW_TRUE)
        printf(head_string);
    restore();
    printf("\n");
    fflush(stdout);
    
    for (i=0;i<msg->number_of_queues;i++)
    {
        printf("%-20.20s ",msg->queue_name[i]);
        
        sprintf(buffer,"%3i/%-3i",msg->queue_freenodecount[i],
                        msg->queue_nodecount[i]);
        tmp = buffer;
        while (*tmp ==' ')
            tmp++;
        printf("%-7s ",tmp);

        printf("%-5i ",msg->queue_maxtime[i]);
        printf("%-5i ",msg->queue_maxcputime[i]);
        printf("%-5i ",msg->queue_maxcount[i]);
        printf("%-5i ",msg->queue_maxrunningjobs[i]);
        printf("%-5i ",msg->queue_maxjobsinqueue[i]);
        printf("%-8.8s ",msg->queue_status[i]);
        printf("%-10.10s ",msg->queue_dispatchtype[i]);
        printf("%-8s\n",msg->queue_priority[i]);
    }
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

void gw_client_print_host_pool_status()
{
    int i;
    
    for (i=0;i<gw_client.number_of_hosts;i++)
        if (gw_client.host_pool[i] != NULL)
                gw_client_print_host_status(gw_client.host_pool[i]);
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

void gw_client_print_host_match_header()
{
    char head_string[200];

    sprintf(head_string,"%-3s %-10s %-5s %-5s %-5s %-20s", 
        "HID","QNAME","RANK","NICE","SLOTS","HOSTNAME"); 
    bold();
    underline(); 
    printf(head_string);
    restore();
    printf("\n");
    fflush(stdout);

}

void gw_client_print_host_match(gw_msg_match_t *match_list)
{
    int j;
    
    for (j=0;j< match_list->number_of_queues;j++)
    	if (match_list->match[j] == GW_TRUE )
        {
        	printf("%-3i " ,match_list->host_id);
            printf("%-10s ",match_list->queue_name[j]);
            printf("%-5i " ,match_list->rank[j]);
            printf("%-5i " ,match_list->nice);
            printf("%-5i ",match_list->slots[j]);
            printf("%-20s\n",match_list->hostname);            
        }
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void gw_client_print_user_header()
{
    char head_string[200];
    
    sprintf(head_string,"%-3s %-10s %-4s %-3s %-6s %-10s %-5s %-10s %-5s",
    "UID","NAME","JOBS","RUN","IDLE","EM","PID","TM","PID");
                        
    bold();
    underline(); 
    printf(head_string);
    restore();
    printf("\n");
    fflush(stdout);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void gw_client_print_user(gw_msg_user_t *msg_user)
{
	int max;
	int i;
	
    printf("%-3i ", msg_user->user_id);
	printf("%-10s ",msg_user->name);
    printf("%-4i ", msg_user->active_jobs);
    printf("%-3i ", msg_user->running_jobs);
    printf("%-6i ", (int) msg_user->idle);
    
    if (msg_user->num_ems > msg_user->num_tms)
    	max = msg_user->num_ems;
    else
    	max = msg_user->num_tms;
    	    
    for (i=0;i<max;i++)
    {
    	if (i!=0)
    		printf("                               ");
    		
    	if (i<msg_user->num_ems)
    	{
    		printf("%-10s ", msg_user->em_name[i]);
    		printf("%-5i ", msg_user->em_pid[i]);
    	}
    	else
    		printf("               ");

    	if (i<msg_user->num_tms)
    	{
    		printf("%-10s ", msg_user->tm_name[i]);
    		printf("%-5i ", msg_user->tm_pid[i]);
    	}
    	else
    		printf("              ");
    	
    	printf("\n");
    }
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
#ifdef HAVE_LIBDB

void gw_client_print_user_accts_header(const char *user, time_t from_time)
{
    char head_string[200];
    
	printf("\nAccounting statistics for user %s",user);
	
	if( from_time != (time_t)NULL )
		printf(", counting from %s", ctime(&from_time));
	
	printf("\n\n");

    sprintf(head_string,"%-30s %-8s %-8s %-8s %-4s %-4s %-4s %-4s %-4s %-4s %-4s %-4s %-4s %-4s",
    "HOST","XFR","EXE","SUSP","TOTS","SUCC","ERR ","KILL","USER","SUSP","DISC","SELF","PERF","S/R ");
    		
    bold();
    underline(); 
    printf(head_string);
    restore();
    printf("\n");
    fflush(stdout);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void gw_client_print_host_accts_header(const char *host, time_t from_time)
{
    char head_string[200];
    
	printf("\nAccounting statistics for host %s",host);
	
	if( from_time != (time_t)NULL )
		printf(", counting from %s", ctime(&from_time));
	
	printf("\n\n");	

    sprintf(head_string,"%-30s %-8s %-8s %-8s %-4s %-4s %-4s %-4s %-4s %-4s %-4s %-4s %-4s %-4s",
    "USER","XFR","EXE","SUSP","TOTS","SUCC","ERR ","KILL","USER","SUSP","DISC","SELF","PERF","S/R ");
    		
    bold();
    underline(); 
    printf(head_string);
    restore();
    printf("\n");
    fflush(stdout);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void gw_client_print_host_and_user_accts_header(const char *host, const char *user, time_t from_time)
{
    char head_string[200];
    
	printf("\nAccounting statistics for %s @ %s",user,host);
	
	if( from_time != (time_t)NULL )
		printf(", counting from %s", ctime(&from_time));
	
	printf("\n\n");	

    sprintf(head_string,"%-30s %-8s %-8s %-8s %-4s %-4s %-4s %-4s %-4s %-4s %-4s %-4s %-4s %-4s",
    "USER @ HOST","XFR","EXE","SUSP","TOTS","SUCC","ERR ","KILL","USER","SUSP","DISC","SELF","PERF","S/R ");
    		
    bold();
    underline(); 
    printf(head_string);
    restore();
    printf("\n");
    fflush(stdout);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

static void gw_client_tohours(time_t the_time, char *hstr)
{
	div_t r;
    int   hours;
    
	r = div(the_time, 3600);
	hours = r.quot;		
	r = div(r.rem, 60);
	
	snprintf(hstr,9,"%02i:%02i:%02i",hours,r.quot,r.rem);
}


void gw_client_print_accts(gw_acct_t **accts, int num)
{
	int i;
	char strtime[15];
	
	for (i=0; i<num ; i++)
	{
        printf("%-30s ",accts[i]->name);
        
        gw_client_tohours(accts[i]->transfer, strtime);
        printf("%-8s ",strtime);
        
        gw_client_tohours(accts[i]->execution, strtime);
        printf("%-8s ",strtime);        

        gw_client_tohours(accts[i]->suspension, strtime);
        printf("%-8s ",strtime);

        printf("%-4i ",accts[i]->tot);
        printf("%-4i ",accts[i]->succ);
        printf("%-4i ",accts[i]->err);
        printf("%-4i ",accts[i]->kill);        
        printf("%-4i ",accts[i]->user);
        printf("%-4i ",accts[i]->susp);        
        printf("%-4i ",accts[i]->disc);
        printf("%-4i ",accts[i]->self);
        printf("%-4i ",accts[i]->perf);
        printf("%-4i\n",accts[i]->s_r);        
	}
}

#endif

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
