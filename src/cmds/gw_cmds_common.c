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
    char *out_options;
    
    int i; 

    if(outoption != NULL)
        out_options=outoption;
    else
        out_options=GW_PS_DEFAULT_OPTIONS;

    head_string[0]='\0';
    
    for(i=0;i<strlen(out_options);i++)
    {
        switch(out_options[i])
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
                sprintf(tmpstr,"%-15.15s ","NAME");
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
            case 'p':
                sprintf(tmpstr,"%-2s ","FP");
                break;
            case 'J':
                sprintf(tmpstr,"%-3s ","JID");
                break;
            case 'y':
                sprintf(tmpstr,"%-8s ","TYPE");
                break;
            case 'n':
                sprintf(tmpstr,"%-2s ","NP");
                break;
        }   		 		
        strncat(head_string,tmpstr,strlen(tmpstr));
    }
 
    bold();
    underline(); 
    printf("%s",head_string);
    restore();
    printf("\n");
    fflush(stdout);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void gw_client_print_status(gw_msg_job_t * msg,char *outoption)
{
    char the_time[15];
    char buf[25];
    char *out_options;
    int  i;

    the_time[5]='\0';

    if(strlen(msg->host)==0)
        strcpy(msg->host,"--");

    if(outoption!=NULL)
        out_options=outoption;
    else
        out_options=GW_PS_DEFAULT_OPTIONS;
   	
    for(i=0;i<strlen(out_options);i++)
    {
        switch(out_options[i])
        {
            case 'e':
                printf("%-4s ", gw_em_state_string (msg->em_state));	
                break;  
            case 's':
                printf("%-4s ", gw_job_state_string(msg->job_state));
                break;
            case 'u':
                sprintf(buf, "%s:%i", msg->owner,msg->uid);
                printf("%-12s ", buf);
                break;
            case 'j':
                printf("%-15.15s ", msg->name);
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
            case 'p':
                printf("%02u ",msg->fixed_priority);
                break;
            case 'J':
                printf("%-3d ",msg->id);
                break;
            case 'y':
                printf("%-8s ",gw_template_jobtype_string(msg->type));
                break;
            case 'n':
                printf("%-2d ",msg->np);
                break;
        }   		 		
    }    
    printf("\n");	
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
			/* -----   Show jobs for username@hostname ----- */			
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

void gw_client_print_status_full(gw_msg_job_t * msg)
{
    char the_time[15];

    the_time[5]='\0';

    printf("JOB_ID=%d\n",msg->id);
    printf("NAME=%s\n",msg->name);
    printf("USER=%s\n",msg->owner);
    printf("UID=%d\n",msg->uid);

    if(msg->array_id != -1)
    {
	    printf("ARRAY_ID=%d\n",msg->array_id);
	    printf("TASK_ID=%d\n",msg->task_id);
    }
           
    if(strlen(msg->host) > 0)
	    printf("HOST=%s\n",msg->host);
        
	printf("FIXED_PRIORITY=%u\n",msg->fixed_priority);
	printf("DEADLINE=%s\n",gw_template_deadline_string(msg->deadline));

	printf("TYPE=%s\n",gw_template_jobtype_string(msg->type));
	printf("NP=%d\n",msg->np);
		
	printf("JOB_STATE=%s\n", gw_job_state_string(msg->job_state));
    
	printf("EM_STATE=%s\n", gw_em_state_string (msg->em_state));
	printf("RESTARTED=%d\n",msg->restarted);
    printf("CLIENT_WAITING=%d\n",msg->client_waiting);
    printf("RESCHEDULE=%d\n",msg->reschedule);

	printf("START_TIME=%s\n",gw_print_time2(msg->start_time,the_time));
	printf("EXIT_TIME=%s\n",gw_print_time2(msg->exit_time,the_time));
	printf("EXEC_TIME=%s\n",gw_print_time (msg->cpu_time,the_time)); 
	printf("XFR_TIME=%s\n",gw_print_time (msg->xfr_time,the_time)); 
	        
	if (msg->job_state == GW_JOB_STATE_ZOMBIE)
	    printf("EXIT_CODE=%d\n",msg->exit_code);
    
    printf("\n");
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void gw_client_print_status_xml(gw_msg_job_t * msg, int xml_header_flag, int xml_footer_flag)
{
    char the_time[15];

    the_time[5]='\0';
	const char command[]="gwps";
    if (xml_header_flag) gw_print_xml_header(command);
 
    printf("\t<JOB JOB_ID=\"%d\">\n",msg->id);
    printf("\t\t<NAME>%s</NAME>\n",msg->name);
    printf("\t\t<USER>%s</USER>\n",msg->owner);
    printf("\t\t<UID>%d</UID>\n",msg->uid);

    if(msg->array_id != -1)
    {
	    printf("\t\t<ARRAY_ID>%d</ARRAY_ID>\n",msg->array_id);
	    printf("\t\t<TASK_ID>%d</TASK_ID>\n",msg->task_id);
    }
           
    if(strlen(msg->host) > 0)
	    printf("\t\t<HOST>%s</HOST>\n",msg->host);
        
	printf("\t\t<FIXED_PRIORITY>%u</FIXED_PRIORITY>\n",msg->fixed_priority);
	printf("\t\t<DEADLINE>%s</DEADLINE>\n",gw_template_deadline_string(msg->deadline));

	printf("\t\t<TYPE>%s</TYPE>\n",gw_template_jobtype_string(msg->type));
	printf("\t\t<NP>%d</NP>\n",msg->np);
		
	printf("\t\t<JOB_STATE>%s</JOB_STATE>\n", gw_job_state_string(msg->job_state));
    
	printf("\t\t<EM_STATE>%s</EM_STATE>\n", gw_em_state_string (msg->em_state));
	printf("\t\t<RESTARTED>%d</RESTARTED>\n",msg->restarted);
    printf("\t\t<CLIENT_WAITING>%d</CLIENT_WAITING>\n",msg->client_waiting);
    printf("\t\t<RESCHEDULE>%d</RESCHEDULE>\n",msg->reschedule);

	printf("\t\t<START_TIME>%s</START_TIME>\n",gw_print_time2(msg->start_time,the_time));
	printf("\t\t<EXIT_TIME>%s</EXIT_TIME>\n",gw_print_time2(msg->exit_time,the_time));
	printf("\t\t<EXEC_TIME>%s</EXEC_TIME>\n",gw_print_time (msg->cpu_time,the_time)); 
	printf("\t\t<XFR_TIME>%s</XFR_TIME>\n",gw_print_time (msg->xfr_time,the_time)); 
	        
	if (msg->job_state == GW_JOB_STATE_ZOMBIE)
	    printf("\t\t<EXIT_CODE>%d</EXIT_CODE>\n",msg->exit_code);
    
	printf("\t</JOB>\n");
    if (xml_footer_flag) gw_print_xml_footer(command);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void gw_client_print_pool_status_full(char *username, char *hostname, char jobstate, int array_id)
{
  int i;            
  
  if(username != NULL)
	{
	  if(hostname != NULL)
		{
		  /* -----   Show jobs for username@hostname ----- */			
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
							gw_client_print_status_full(gw_client.job_pool[i]);
						  else		 
							if(gw_client.job_pool[i]->array_id==array_id) 
							  gw_client_print_status_full(gw_client.job_pool[i]);
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
					gw_client_print_status_full(gw_client.job_pool[i]);
				  else		 
					if(gw_client.job_pool[i]->array_id==array_id) 
					  gw_client_print_status_full(gw_client.job_pool[i]);
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
		    					gw_client_print_status_full(gw_client.job_pool[i]);
		    				else		 
		    					if(gw_client.job_pool[i]->array_id==array_id) 
		    						gw_client_print_status_full(gw_client.job_pool[i]);
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
		    				gw_client_print_status_full(gw_client.job_pool[i]);
		    			else		 
		    				if(gw_client.job_pool[i]->array_id==array_id) 
		    					gw_client_print_status_full(gw_client.job_pool[i]);
        			}  		
 		}
	}
	
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void gw_client_print_pool_status_xml(char *username, char *hostname, char jobstate, int array_id)
{
  int i;            
  int xml_header_flag = 1, xml_footer_flag = 0;
   const char command[] = "gwps";
	if ( xml_header_flag ){
	  gw_print_xml_header(command);
	  xml_header_flag = 0;
	}
  if(username != NULL)
	{
	  if(hostname != NULL)
		{
		  /* -----   Show jobs for username@hostname ----- */			
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
							gw_client_print_status_xml(gw_client.job_pool[i],xml_header_flag,xml_footer_flag);
						  else		 
							if(gw_client.job_pool[i]->array_id==array_id) 
							  gw_client_print_status_xml(gw_client.job_pool[i],xml_header_flag,xml_footer_flag);
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
					gw_client_print_status_xml(gw_client.job_pool[i],xml_header_flag,xml_footer_flag);
				  else		 
					if(gw_client.job_pool[i]->array_id==array_id) 
					  gw_client_print_status_xml(gw_client.job_pool[i],xml_header_flag,xml_footer_flag);
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
		    					gw_client_print_status_xml(gw_client.job_pool[i],xml_header_flag,xml_footer_flag);
		    				else		 
		    					if(gw_client.job_pool[i]->array_id==array_id) 
		    						gw_client_print_status_xml(gw_client.job_pool[i],xml_header_flag,xml_footer_flag);
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
		    				gw_client_print_status_xml(gw_client.job_pool[i],xml_header_flag,xml_footer_flag);
		    			else		 
		    				if(gw_client.job_pool[i]->array_id==array_id) 
		    					gw_client_print_status_xml(gw_client.job_pool[i],xml_header_flag,xml_footer_flag);
        			}  		
 		}
	}
  if ( ! xml_footer_flag ){
	gw_print_xml_footer(command);
	xml_footer_flag = 1;
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
    printf("%s",head_string);
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

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void gw_client_print_history_xml(gw_msg_history_t * msg_history)
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
	
	printf("\t<HISTORY HOST_ID=\"%i\">\n", msg_history->host_id);
	printf("\t\t<START_TIME>%s</START_TIME>\n",gw_print_time2(msg_history->start_time,the_time2));
	printf("\t\t<EXIT_TIME>%s</EXIT_TIME>\n", gw_print_time2(msg_history->exit_time ,the_time2));
	printf("\t\t<PROLOG>%s</PROLOG>\n", gw_print_time (prolog ,the_time));
	printf("\t\t<WRAPPER>%s</WRAPPER>\n", gw_print_time (wrapper ,the_time));
	printf("\t\t<EPILOG>%s</EPILOG>\n", gw_print_time (epilog ,the_time));
	printf("\t\t<MIGRATION>%s</MIGRATION>\n",  gw_print_time (migration ,the_time));  
	printf("\t\t<REASON>%s</REASON>\n", gw_reason_string(msg_history->reason));
	printf("\t\t<QUEUE>%s</QUEUE>\n",  msg_history->queue);    
	printf("\t\t<HOST>%s</HOST>\n", msg_history->em_rc);
	printf("\t</HISTORY>\n");

}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

void gw_client_print_host_status_header()
{
    char head_string[200];
    
    sprintf(head_string,"%-3s %-5s %-15s %-5s %4s %4s %9s %13s %12s %-20s %-20s",
    "HID","PRIO","OS","ARCH","MHZ","%%CPU","MEM(F/T)","DISK(F/T)","N(U/F/T)","LRMS","HOSTNAME");
    
    bold();
    underline(); 
    printf("%s",head_string);
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
    printf("%-5i ",msg->fixed_priority);    
    
    snprintf(buffer,sizeof(char)*25,"%s%s",msg->os_name, msg->os_version);
    printf("%-15.15s ",buffer);
    
    printf("%-5.5s ",msg->arch);
    printf("%4i ",msg->cpu_mhz);
    printf("%4i ",msg->cpu_free);

    snprintf(buffer,sizeof(char)*25,"%i/%i",msg->free_mem_mb,msg->size_mem_mb);    
    tmp = buffer;
    while (*tmp ==' ')
        tmp++;
    printf("%9s ",tmp);

    snprintf(buffer,sizeof(char)*25,"%i/%i",msg->free_disk_mb,msg->size_disk_mb);
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
    
    snprintf(buffer,sizeof(char)*25,"%i/%i/%i",msg->used_slots,freenodecount,msg->nodecount);
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
        printf("%s",head_string);
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

void gw_client_print_host_pool_status_full()
{
    int i;
    
    for (i=0;i<gw_client.number_of_hosts;i++)
        if (gw_client.host_pool[i] != NULL)
                gw_client_print_host_status_full(gw_client.host_pool[i]);
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

void gw_client_print_host_pool_status_xml()
{
  int i;
  int xml_header_flag = 1, xml_footer_flag = 0;
  const char command[] = "gwhost";
  for (i=0;i<gw_client.number_of_hosts;i++){
	if ( xml_header_flag ){
	  gw_print_xml_header(command);
	  xml_header_flag = 0;
	}
	if (gw_client.host_pool[i] != NULL)
	  gw_client_print_host_status_xml(gw_client.host_pool[i], xml_header_flag, xml_footer_flag);
  }
	if ( ! xml_footer_flag ){
	  gw_print_xml_footer(command);
	  xml_footer_flag = 1;
	}
}
  
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

void gw_client_print_host_match_header()
{
    char head_string[200];

    sprintf(head_string,"%-3s %-10s %-5s %-5s %-5s %-20s", 
        "HID","QNAME","RANK","PRIO","SLOTS","HOSTNAME"); 
    bold();
    underline(); 
    printf("%s",head_string);
    restore();
    printf("\n");
    fflush(stdout);

}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

void gw_client_print_host_match(gw_msg_match_t *match_list)
{
    int j;
    
    for (j=0;j< match_list->number_of_queues;j++)
    	if (match_list->match[j] == GW_TRUE )
        {
        	printf("%-3i " ,match_list->host_id);
            printf("%-10s ",match_list->queue_name[j]);
            printf("%-5i " ,match_list->rank[j]);
            printf("%-5i " ,match_list->fixed_priority);
            printf("%-5i ",match_list->slots[j]);
            printf("%-20s\n",match_list->hostname);            
        }
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

void gw_client_print_host_match_xml(gw_msg_match_t *match_list)
{
    int j;
    int host_id=-1;
	int is_first_queue_in_host=1, was_last_queue_in_host=0;
	int did_it_print_at_least_one_queue = 0;

	// We have to order the queues by host, so let's redo the loop
    for (j=0;j< match_list->number_of_queues;j++){
    	if (match_list->match[j] == GW_TRUE )
        {

		  if ( host_id != -1 && host_id != match_list->host_id ){
			// Let's close the HOST tag
			was_last_queue_in_host=1;
		  }

		  if ( host_id == -1 || host_id != match_list->host_id ){
			is_first_queue_in_host=1;
		  }

		  // By default, while going through the first queue, info about the host is printed.
		  if ( is_first_queue_in_host ) {
			printf("\t<HOST HOST_ID=\"%i\">\n", match_list->host_id);
			printf("\t\t<HOSTNAME>%s</HOSTNAME>\n", match_list->hostname);            
			printf("\t\t<FIXED_PRIORITY>%i</FIXED_PRIORITY>\n", match_list->fixed_priority);
		  }
		  if ( was_last_queue_in_host ) {
			printf("\t</HOST>\n");
		  }

		  printf("\t\t<QUEUE QUEUE_ID=\"%i\">\n", j);
		  printf("\t\t\t<QUEUE_NAME>%s</QUEUE_NAME>\n", match_list->queue_name[j]);
		  printf("\t\t\t<RANK>%i</RANK>\n" , match_list->rank[j]);
		  printf("\t\t\t<SLOTS>%i</SLOTS>\n", match_list->slots[j]);
		  printf("\t\t</QUEUE>\n");

		  is_first_queue_in_host = 0;
		  was_last_queue_in_host = 0;
		  if (! did_it_print_at_least_one_queue){
			did_it_print_at_least_one_queue = 1;
		  }
        }
		host_id = match_list->host_id;
	}
	was_last_queue_in_host = 1;
	if ( was_last_queue_in_host && did_it_print_at_least_one_queue ) {
	  printf("\t</HOST>\n");
	}
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

void gw_client_print_host_status_full(gw_msg_host_t * msg)
{
    int freenodecount;
    int i;
    
    printf("HOST_ID=%i\n",msg->host_id);
    printf("HOSTNAME=%s\n",msg->hostname);

    printf("FIXED_PRIORITY=%i\n",msg->fixed_priority);    
    
    printf("OS_NAME=%s\n", msg->os_name);
    printf("OS_VERSION=%s\n",msg->os_version);
    
    printf("ARCH=%s\n",msg->arch);
    printf("CPU_MHZ=%i\n",msg->cpu_mhz);
    printf("CPU_FREE=%i\n",msg->cpu_free);

    printf("FREE_MEM_MB=%d\n", msg->free_mem_mb);
    printf("SIZE_MEM_MB=%d\n", msg->size_mem_mb);
    printf("FREE_DISK_MB=%d\n", msg->free_disk_mb);
    printf("SIZE_DISK_MB=%d\n", msg->size_disk_mb);

    freenodecount = 0;
    for (i=0; i<msg->number_of_queues; i++)
    {
        if (msg->queue_freenodecount[i] > freenodecount)
            freenodecount = msg->queue_freenodecount[i];
    }
    
    printf("RUNNING_JOBS=%i\n",msg->running_jobs);
    printf("USED_SLOTS=%i\n",msg->used_slots);
    printf("FREENODECOUNT=%i\n",freenodecount);
    printf("NODECOUNT=%i\n",msg->nodecount);

    printf("LRMS_NAME=%s\n",msg->lrms_name);
    printf("LRMS_TYPE=%s\n",msg->lrms_type);
    
    for (i=0;i<msg->number_of_queues;i++)
    {
        printf("QUEUE_NAME[%i]=%s\n",i,msg->queue_name[i]);
 
        printf("QUEUE_FREENODECOUNT[%i]=%i\n",i,msg->queue_freenodecount[i]);
        printf("QUEUE_NODECOUNT[%i]=%i\n",i,msg->queue_nodecount[i]);

        printf("QUEUE_MAXTIME[%i]=%i\n",i,msg->queue_maxtime[i]);
        printf("QUEUE_MAXCPUTIME[%i]=%i\n",i,msg->queue_maxcputime[i]);
        printf("QUEUE_MAXCOUNT[%i]=%i\n",i,msg->queue_maxcount[i]);
        printf("QUEUE_MAXRUNNINGJOBS[%i]=%i\n",i,msg->queue_maxrunningjobs[i]);
        printf("QUEUE_MAXJOBSINQUEUE[%i]=%i\n",i,msg->queue_maxjobsinqueue[i]);
        printf("QUEUE_STATUS[%i]=%s\n",i,msg->queue_status[i]);
        printf("QUEUE_DISPATCHTYPE[%i]=%s\n",i,msg->queue_dispatchtype[i]);
        printf("QUEUE_PRIORITY[%i]=%s\n",i,msg->queue_priority[i]);
    }

    for (i=0;i<msg->number_of_int_vars;i++)
       	printf("%s=%i\n", msg->gen_var_int_name[i],msg->gen_var_int_value[i]);

    for (i=0;i<msg->number_of_str_vars;i++)
       	printf("%s=%s\n", msg->gen_var_str_name[i],msg->gen_var_str_value[i]);
    
    printf("\n");
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

void gw_client_print_host_status_xml(gw_msg_host_t * msg, int xml_header_flag, int xml_footer_flag)
{
    int freenodecount;
    int i;
	const char command[]="gwhost";
    if (xml_header_flag) gw_print_xml_header(command);
    printf("\t<HOST HOST_ID=\"%i\">\n",msg->host_id);
    printf("\t\t<HOSTNAME>%s</HOSTNAME>\n",msg->hostname);

    printf("\t\t<FIXED_PRIORITY>%i</FIXED_PRIORITY>\n",msg->fixed_priority);    
    
    printf("\t\t<OS_NAME>%s</OS_NAME>\n", msg->os_name);
    printf("\t\t<OS_VERSION>%s</OS_VERSION>\n",msg->os_version);
    
    printf("\t\t<ARCH>%s</ARCH>\n",msg->arch);
    printf("\t\t<CPU_MHZ>%i</CPU_MHZ>\n",msg->cpu_mhz);
    printf("\t\t<CPU_FREE>%i</CPU_FREE>\n",msg->cpu_free);

    printf("\t\t<FREE_MEM_MB>%d</FREE_MEM_MB>\n", msg->free_mem_mb);
    printf("\t\t<SIZE_MEM_MB>%d</SIZE_MEM_MB>\n", msg->size_mem_mb);
    printf("\t\t<FREE_DISK_MB>%d</FREE_DISK_MB>\n", msg->free_disk_mb);
    printf("\t\t<SIZE_DISK_MB>%d</SIZE_DISK_MB>\n", msg->size_disk_mb);

    freenodecount = 0;
    for (i=0; i<msg->number_of_queues; i++)
    {
        if (msg->queue_freenodecount[i] > freenodecount)
            freenodecount = msg->queue_freenodecount[i];
    }
    
    printf("\t\t<RUNNING_JOBS>%i</RUNNING_JOBS>\n",msg->running_jobs);
    printf("\t\t<USED_SLOTS>%i</USED_SLOTS>\n",msg->used_slots);
    printf("\t\t<FREENODECOUNT>%i</FREENODECOUNT>\n",freenodecount);
    printf("\t\t<NODECOUNT>%i</NODECOUNT>\n",msg->nodecount);

    printf("\t\t<LRMS_NAME>%s</LRMS_NAME>\n",msg->lrms_name);
    printf("\t\t<LRMS_TYPE>%s</LRMS_TYPE>\n",msg->lrms_type);

    
    for (i=0;i<msg->number_of_queues;i++)
	  {
		int j;
		printf("\t\t<QUEUE QUEUE_ID=\"%i\">\n",i);
        printf("\t\t\t<QUEUE_NAME>%s</QUEUE_NAME>\n",msg->queue_name[i]);
		printf("\t\t\t<QUEUE_FREENODECOUNT>%i</QUEUE_FREENODECOUNT>\n",msg->queue_freenodecount[i]);
        printf("\t\t\t<QUEUE_NODECOUNT>%i</QUEUE_NODECOUNT>\n",msg->queue_nodecount[i]);
        printf("\t\t\t<QUEUE_MAXTIME>%i</QUEUE_MAXTIME>\n",msg->queue_maxtime[i]);
        printf("\t\t\t<QUEUE_MAXCPUTIME>%i</QUEUE_MAXCPUTIME>\n",msg->queue_maxcputime[i]);
        printf("\t\t\t<QUEUE_MAXCOUNT>%i</QUEUE_MAXCOUNT>\n",msg->queue_maxcount[i]);
        printf("\t\t\t<QUEUE_MAXRUNNINGJOBS>%i</QUEUE_MAXRUNNINGJOBS>\n",msg->queue_maxrunningjobs[i]);
        printf("\t\t\t<QUEUE_MAXJOBSINQUEUE>%i</QUEUE_MAXJOBSINQUEUE>\n",msg->queue_maxjobsinqueue[i]);
        printf("\t\t\t<QUEUE_STATUS>%s</QUEUE_STATUS>\n",msg->queue_status[i]);
        printf("\t\t\t<QUEUE_DISPATCHTYPE>%s</QUEUE_DISPATCHTYPE>\n",msg->queue_dispatchtype[i]);
        printf("\t\t\t<QUEUE_PRIORITY>%s</QUEUE_PRIORITY>\n",msg->queue_priority[i]);
		// AL: Apparently we can bring the gen_var_str_name here, getting rid of [],
		// since the information appearing here is, at the moment, QUEUE_ACCESS
		for (j=0;j<msg->number_of_str_vars;j++) {
		  if ( j == i ) {
			char * beginning_of_bracket = strrchr(msg->gen_var_str_name[j],'[');
			char var_str_name[beginning_of_bracket-msg->gen_var_str_name[j]];
			strncpy ( var_str_name,msg->gen_var_str_name[j],beginning_of_bracket-msg->gen_var_str_name[j]);
			var_str_name[beginning_of_bracket-msg->gen_var_str_name[j]]='\0';
			//	    printf ("Last occurence of '[' found at %d \n", beginning_of_bracket-msg->gen_var_str_name[j]+1);
			printf("\t\t\t<%s>%s</%s>\n", var_str_name, msg->gen_var_str_value[j], var_str_name);
		  }
		}
		printf("\t\t</QUEUE>\n");
	  }
    /*
	  for (i=0;i<msg->number_of_int_vars;i++)
	  printf("\t\t<%s>%i</%s>\n", msg->gen_var_int_name[i],msg->gen_var_int_value[i]);
	  
	  for (i=0;i<msg->number_of_str_vars;i++)
	  printf("\t\t\t<%s>%s</%s>\n", msg->gen_var_str_name[i],msg->gen_var_str_value[i]);
    */    
    printf("\t</HOST>\n");
    if (xml_footer_flag) gw_print_xml_footer(command);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void gw_client_print_user_header()
{
    char head_string[200];
    
    sprintf(head_string,"%-3s %-10s %-4s %-3s %-6s %-40s",
    "UID","NAME","JOBS","RUN","IDLE","IDENTITY");
                        
    bold();
    underline(); 
    printf("%s",head_string);
    restore();
    printf("\n");
    fflush(stdout);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void gw_client_print_user(gw_msg_user_t *msg_user)
{
    int i;
	
    printf("%-3i ", msg_user->user_id);
    printf("%-10s ", msg_user->name);
    printf("%-4i ", msg_user->active_jobs);
    printf("%-3i ", msg_user->running_jobs);
    printf("%-6i ", (int) msg_user->idle);
    printf("%s\n", msg_user->dn);
    
    printf("    EM MADs: ");

    for (i=0;i<msg_user->num_ems;i++)
    {
        if (i != 0)
            printf(", ");

    	printf("%s (PID=%i)", msg_user->em_name[i],
                msg_user->em_pid[i]);
    }

    printf("\n    TM MADs: ");

    for (i=0;i<msg_user->num_tms;i++)
    {
        if (i != 0)
            printf(", ");
    	printf("%s (PID=%i)", msg_user->tm_name[i],
                msg_user->tm_pid[i]);
    }

    printf("\n");
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void gw_client_print_user_xml(gw_msg_user_t *msg_user)
{
    int i;
	
    printf("\t<USER USER_ID=\"%i\">\n", msg_user->user_id);
    printf("\t\t<USERNAME>%s</USERNAME>\n", msg_user->name);
    printf("\t\t<ACTIVE_JOBS>%i</ACTIVE_JOBS>\n", msg_user->active_jobs);
    printf("\t\t<RUNNING_JOBS>%i</RUNNING_JOBS>\n", msg_user->running_jobs);
    printf("\t\t<IDLE>%i</IDLE>\n", (int) msg_user->idle);
    printf("\t\t<IDENTITY>%s</IDENTITY>\n", msg_user->dn);
    for (i=0;i<msg_user->num_ems;i++)
    {
	  printf("\t\t<EM_MAD MAD_ID=\"%i\">\n",i);
	  printf("\t\t\t<NAME>%s</NAME>\n", msg_user->em_name[i]);
	  printf("\t\t\t<PID>%i</PID>\n", msg_user->em_pid[i]);
      printf("\t\t</EM_MAD>\n");
    }
    for (i=0;i<msg_user->num_tms;i++)
    {
 	  printf("\t\t<TM_MAD MAD_ID=\"%i\">\n",i);
	  printf("\t\t\t<NAME>%s</NAME>\n", msg_user->tm_name[i]);
	  printf("\t\t\t<PID>%i</PID>\n", msg_user->tm_pid[i]);
      printf("\t\t</TM_MAD>\n");
    }
    printf("\t</USER>\n");
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
 
 void gw_print_xml_header(const char *command)
{
   printf("<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n");
   printf("<%s>\n",command);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
 
 void gw_print_xml_footer(const char *command)
{
  printf("</%s>\n",command);
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
    printf("%s",head_string);
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
    printf("%s",head_string);
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
    
	printf("\nAccounting statistics for %s@%s",user,host);
	
	if( from_time != (time_t)NULL )
		printf(", counting from %s", ctime(&from_time));
	
	printf("\n\n");	

    sprintf(head_string,"%-30s %-8s %-8s %-8s %-4s %-4s %-4s %-4s %-4s %-4s %-4s %-4s %-4s %-4s",
    "USER@HOST","XFR","EXE","SUSP","TOTS","SUCC","ERR ","KILL","USER","SUSP","DISC","SELF","PERF","S/R ");
    		
    bold();
    underline(); 
    printf("%s",head_string);
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

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

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

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void gw_client_print_accts_xml(gw_acct_t **accts, int num, const int u, const int r)
{
	int i;
	char strtime[15];

	int max_user_or_host_len=24;
	char user_or_host[max_user_or_host_len];
	char opposite_to_user_or_host[max_user_or_host_len];
	char tab[2];


	for (i=0; i<num ; i++)
	{
	  if ( ( u + r ) == 2 ){
	    sprintf (tab,"");
	  }
	  else if ( u == 1 ){
	    sprintf (user_or_host, "USER");
	    sprintf (opposite_to_user_or_host, "HOST");
	    sprintf (tab,"\t");
	    printf("%s<%s %sNAME=\"%s\">\n", tab, opposite_to_user_or_host, opposite_to_user_or_host, accts[i]->name);
	  } 
	  else if ( r == 1 ){
	    sprintf (user_or_host, "HOST");
	    sprintf (opposite_to_user_or_host, "USER");
	    sprintf (tab,"\t");
	    printf("%s<%s %sNAME=\"%s\">\n", tab, opposite_to_user_or_host, opposite_to_user_or_host, accts[i]->name);
	  }
        
	  gw_client_tohours(accts[i]->transfer, strtime);
	  printf("%s\t<TRANSFER>%s</TRANSFER>\n", tab, strtime);
	  
	  gw_client_tohours(accts[i]->execution, strtime);
	  printf("%s\t<EXECUTION>%s</EXECUTION>\n", tab, strtime);
	  
	  gw_client_tohours(accts[i]->suspension, strtime);
	  printf("%s\t<SUSPENSION>%s</SUSPENSION>\n", tab, strtime);
	  
	  printf("%s\t<TOTS>%i<TOTS>\n", tab, accts[i]->tot);
	  printf("%s\t<SUCC>%i</SUCC>\n", tab, accts[i]->succ);
	  printf("%s\t<ERR>%i</ERR>\n", tab, accts[i]->err);
	  printf("%s\t<KILL>%i</KILL>\n", tab, accts[i]->kill);
	  printf("%s\t<USER>%i</USER>\n", tab, accts[i]->user);
	  printf("%s\t<SUSP>%i</SUSP>\n", tab, accts[i]->susp);
	  printf("%s\t<DISC>%i</DISC>\n", tab, accts[i]->disc);
	  printf("%s\t<SELF>%i</SELF>\n", tab, accts[i]->self);
	  printf("%s\t<PERF>%i</PERF>\n", tab, accts[i]->perf);
	  printf("%s\t<S/R>%i</S/R>\n", tab, accts[i]->s_r);

	  if ( ( u + r ) != 2 ){
	    printf("%s</%s>\n", tab, opposite_to_user_or_host);
	  }
	}
}

#endif

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
