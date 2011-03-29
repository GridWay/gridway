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

#include "gw_tm_ftp_transfer.h"

extern gw_tm_ftp_transfer_t ** gw_tm_ftp_xfr_pool;

int main (int argc, char **argv )
{
    char   str[4096];
    int    rc,j;
    char   action[20];
    char   xfr_id_s[20];
    int    xfr_id;
    char   cp_xfr_id_s[20];
    int    cp_xfr_id;
    char   src_url[1024];
    char   dst_url[1024];
    char   modex;
    int    end = 0;   

    struct timeval t1,t2;
    double waited;
    
	struct timeval tv;
    fd_set in_pipes;
    char   c;
	
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
	        }while ( rc > 0 && c != '\n' );
	        
	        str[j] = '\0';    

        	if (rc <= 0)
        	{
            	exit(-1);
        	}
        	
        	rc = sscanf(str,"%s %s %s %c %s %s",
                         action,
                         xfr_id_s,
                         cp_xfr_id_s,
                         &modex,
                         src_url,
                         dst_url);
            
        	if (rc != 6 )
        	{
            	continue;
        	}
        
        	xfr_id = atoi(xfr_id_s);

        	if (strcmp(action, "INIT") == 0 )
        	{
            	rc = gw_tm_ftp_mad_init(xfr_id);
            	
            	if (rc == 1)
            		printf("INIT - - FAILURE Error Activating FTP & GASS modules\n");
            	else
            		printf("INIT - - SUCCESS -\n");
        	}
        	else if (strcmp(action, "START") == 0 )
        	{
        		rc = gw_tm_ftp_add_xfr(xfr_id);
        		
            	if (rc == 1)
            		printf("START %i - FAILURE already started or invalid id\n",xfr_id);
            	else
            		printf("START %i - SUCCESS -\n",xfr_id);
        	}
	        else if (strcmp(action, "END") == 0 )
	        {
				rc = gw_tm_ftp_del_xfr(xfr_id);
				
	            if (rc == 1)
            		printf("END %i - FAILURE not started or invalid id\n",xfr_id);
	        }
        	else if (strcmp(action, "RMDIR") == 0 )
        	{
				if (gw_tm_ftp_xfr_pool[xfr_id] == NULL)
				{
					printf("RMDIR %i - FAILURE id does not exists, or invalid\n",xfr_id);
				}
				else
				{
					rc = gw_tm_ftp_transfer_rmdir(xfr_id, src_url);
								
					if ( rc == 1 )
						printf("RMDIR %i - FAILURE error in MLST command\n",xfr_id);
				}
	        }
	        else if (strcmp(action, "EXISTS") == 0 )
    	    {
				if (gw_tm_ftp_xfr_pool[xfr_id] == NULL)
				{
					printf("EXISTS %i - FAILURE id does not exists, or invalid\n",xfr_id);					
				}
				else
				{
					rc = gw_tm_ftp_transfer_exists_dir(xfr_id, src_url);
					
					if ( rc == 1 )
						printf("EXISTS %i - FAILURE error in EXISTS command\n",xfr_id);
				}
        	}
        	else if (strcmp(action, "MKDIR") == 0 )
        	{
 				if (gw_tm_ftp_xfr_pool[xfr_id] == NULL)
				{
					printf("MKDIR %i - FAILURE id does not exists, or invalid\n",xfr_id);
				}
				else
				{
					rc = gw_tm_ftp_transfer_mkdir(xfr_id,src_url);	
					
					if ( rc == 1 )
						printf("MKDIR %i - FAILURE error in MKDIR command\n",xfr_id);
				}
        	}                
        	else if (strcmp(action, "CP") == 0 )
        	{
 				if (gw_tm_ftp_xfr_pool[xfr_id] == NULL)
				{
					printf("CP %i - FAILURE id does not exists, or invalid\n",xfr_id);
				}
				else
				{
					cp_xfr_id = atoi(cp_xfr_id_s);
				
					rc = gw_tm_ftp_transfer_url_to_url(xfr_id,
									                   cp_xfr_id, 
									                   src_url,
									                   dst_url,
									                   modex);	
					if ( rc == 1 )					
						printf("CP %i %i FAILURE error in url2url command (%s->%s)\n",xfr_id,
							cp_xfr_id, src_url, dst_url);
				}
        	}                
        	else if (strcmp(action, "FINALIZE") == 0 )
        	{
            	end    = 1;
        	}
        	else
        	{
            	printf("%s %d - FAILURE not a valid action\n", action, xfr_id);	
        	}
        }                       
    }
    
    printf("FINALIZE - - SUCCESS -\n");
    
    return 0;
}
