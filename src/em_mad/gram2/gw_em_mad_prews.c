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

#include "gw_em_mad_prews.h"

extern struct mad_s mad;

int main (int argc, char **argv )
{
    int  rc;
    char action[20];
    char jid_s[20];
    int  jid = 0;
    char contact[500];
    char rsl_file[1024];
    int  status = -1;
    char info[500];
    int end = 0;
    fd_set in_pipes;
    int j;
    char c;
    char str[4096];
    struct timeval tv;
    int timer_interval = 300;
    time_t last_timer = 0;
    time_t the_time;
    
    struct timeval t1,t2;
    double waited;

    setbuf(stdout,NULL);

    rc = globus_module_activate(GLOBUS_COMMON_MODULE);
    
    if ( rc != GLOBUS_SUCCESS )
    	return -1;
    
    waited = 0;
        
    while (!end)
    {
        FD_ZERO(&in_pipes);
        FD_SET (0,&in_pipes);

        tv.tv_sec  = 0;
        tv.tv_usec = 1000;
        
		gettimeofday(&t1, NULL);
		
        rc = select(1, &in_pipes, NULL, NULL, &tv);

		gettimeofday(&t2, NULL);
   		
   		waited += ((t2.tv_sec - t1.tv_sec)*1000000) + (t2.tv_usec - t1.tv_usec);
	
	    if ( waited > 999 )
        {
        	globus_poll();
        	waited = 0;
        }          
        	
        if (rc == -1)
        {
            exit(-1);
        }
        else if (rc == 1)
        {
            j = 0;

            do
            {
                rc = read(0, (void *) &c, sizeof(char));
                str[j++] = c;
            }
            while ( rc > 0 && c != '\n' );

            str[j] = '\0';

            if (rc <= 0)
                exit(-1);
        
            rc = sscanf(str, "%s %s %s %[^\n]", action, jid_s, contact, 
                    rsl_file);
            
            if (rc != 4 )
            {
                printf("FAILURE Not all four arguments defined\n");
                continue;
            }

            jid = atoi(jid_s);

            if (strcmp(action, "INIT") == 0 )
            {
                status = gw_em_mad_init(jid, info);
            }
            else if (strcmp(action, "SUBMIT") == 0 )
            {
                status = gw_em_mad_submit(jid, contact, rsl_file, info);
            }
            else if (strcmp(action, "RECOVER") == 0 )
            {
                status = gw_em_mad_recover(jid, contact, info);
            }
            else if (strcmp(action, "CANCEL") == 0 )
            {
                status = gw_em_mad_cancel(jid, info);
            }
            else if (strcmp(action, "POLL") == 0 )
            {
                status = gw_em_mad_poll(jid, info);
            }
            else if (strcmp(action, "FINALIZE") == 0 )
            {
                status = gw_em_mad_finalize(info);
                end = 1;

		    	return 0;            
            }

            if (status != 0)
                printf("%s %d FAILURE %s\n", action, jid, info);
        }
  
        the_time = time(NULL);
        
        if (the_time - last_timer >=  timer_interval)
        {
            last_timer = the_time;

			if (mad.initialized == 1)
			{
            	status = gw_em_mad_check_credentials(info);

            	if (status != 0)
                	printf("%s %d FAILURE %s\n", action, jid, info);
			}
        }
    }

    return 0;
}
