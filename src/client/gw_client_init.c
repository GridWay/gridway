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
#include <pwd.h>
#include <unistd.h>
#include <netdb.h>

#include "gw_client.h"
#include "gw_rm_msg.h"
#include "gw_rm.h"
#include "gw_file_parser.h"

gw_client_t gw_client={PTHREAD_MUTEX_INITIALIZER,NULL,-1,"",GW_FALSE,0,NULL,0,NULL};

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

gw_client_t* gw_client_init()
{
	char *          GW_LOCATION;
    char *          conf_file;
    char *          port_file;
    char *          number_of_jobs_s;
    char *          number_of_hosts_s;
    int             rc; 
    int             i;
    FILE *          fd;
	struct passwd * pw_ent;
	
	pthread_mutex_lock(&gw_client.mutex);

	if ( gw_client.initialize == GW_TRUE)
	{
		pthread_mutex_unlock(&gw_client.mutex);
		return &(gw_client);	
	}
	
	/* ---------------------------------------------- */
	/* Set owner for this session                     */
	/* ---------------------------------------------- */
	
	pw_ent = getpwuid(getuid());
	
	if (pw_ent != NULL)
	{
		if (pw_ent->pw_name != NULL)
		{
		    gw_client.owner = strdup(pw_ent->pw_name);		    
		}
		else
		{
			pthread_mutex_unlock(&gw_client.mutex);
			return NULL;
		}
	}
	else
	{
		pthread_mutex_unlock(&gw_client.mutex);
		return NULL;
	}
	
	/* ---------------------------------------------- */
	/* Open gwd.conf to get number of jobs & hosts    */
	/* ---------------------------------------------- */

    GW_LOCATION = getenv("GW_LOCATION");   
    
    if (GW_LOCATION == NULL )
    {
    	fprintf(stderr,"GW_LOCATION is not defined\n");
    	
    	pthread_mutex_unlock(&gw_client.mutex);
        return NULL;
    }
    
    conf_file = malloc(sizeof(char)*(strlen(GW_LOCATION)+14));
    port_file = malloc(sizeof(char)*(strlen(GW_LOCATION)+14));
    
    sprintf(conf_file,"%s/etc/gwd.conf",GW_LOCATION);
    sprintf(port_file,"%s/var/gwd.port",GW_LOCATION);

	/* ------------- TCP PORT ------------- */
	
	fd = fopen(port_file,"r");
	if (fd == NULL)
	{
    	fprintf(stderr,"Error openning gwd.port file (%s)\n",port_file);
    	free(conf_file);
    	free(port_file);
    	
    	pthread_mutex_unlock(&gw_client.mutex);
        return NULL;		
	} 
	fscanf(fd,"%s %i",gw_client.hostname, &gw_client.gwd_port);
	fclose(fd);
	
	free(port_file);

	/* ------------- Number of jobs ------------- */
	     
    rc = gw_parse_file(conf_file,"NUMBER_OF_JOBS",&number_of_jobs_s);

    if ( (rc != -1) && (number_of_jobs_s != NULL) )
    {
    	gw_client.number_of_jobs = atoi(number_of_jobs_s);
    	gw_client.job_pool = (gw_msg_job_t **) malloc( sizeof(gw_msg_job_t *) *
    	                                             gw_client.number_of_jobs);    	                    
        for (i = 0 ; i < gw_client.number_of_jobs; i++)
        	gw_client.job_pool[i] = NULL;
    }
    else
    {
    	fprintf(stderr,"Error parsing gwd.conf. Cannot find NUMBER_OF_HOSTS\n");
    	free(conf_file);
    	
    	pthread_mutex_unlock(&gw_client.mutex);
        return NULL;
    }
    
    free(number_of_jobs_s);

	/* ------------- Number of hosts ------------- */
	     
    rc = gw_parse_file(conf_file,"NUMBER_OF_HOSTS",&number_of_hosts_s);

    if ( (rc != -1) && (number_of_hosts_s != NULL) )
    {
    	gw_client.number_of_hosts = atoi(number_of_hosts_s);
    	gw_client.host_pool = (gw_msg_host_t **) malloc( sizeof(gw_msg_host_t *) *
    	                                             gw_client.number_of_hosts);
    	                                             
        for (i = 0 ; i < gw_client.number_of_hosts; i++)
        	gw_client.host_pool[i] = NULL;
    }
    else
    {
    	fprintf(stderr,"Error parsing gwd.conf (Cannot find NUMBER_OF_HOSTS)\n");
    	free(conf_file);
    	
    	pthread_mutex_unlock(&gw_client.mutex);
        return NULL;
    }
    
    free(number_of_hosts_s);		
    free(conf_file);
		    
    gw_client.initialize = GW_TRUE;
    
	pthread_mutex_unlock(&gw_client.mutex);
	    
    return &(gw_client);    
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

void gw_client_finalize()
{
	int      i;

	pthread_mutex_lock(&gw_client.mutex);
		
	if ( gw_client.initialize == GW_FALSE )
	{
    	pthread_mutex_unlock(&gw_client.mutex);
		return;
	}
			
    for (i = 0 ; i < gw_client.number_of_jobs; i++)
    	if ( gw_client.job_pool[i] != NULL )
       		free(gw_client.job_pool[i]);

    for (i = 0 ; i < gw_client.number_of_hosts; i++)
    	if ( gw_client.host_pool[i] != NULL )
       		free(gw_client.host_pool[i]);
	
	free(gw_client.job_pool);
        
    gw_client.initialize = GW_FALSE;

	pthread_mutex_unlock(&gw_client.mutex);    
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

int gw_client_connect()
{
	int                fd;
    struct sockaddr_in gw_addr;
	struct hostent *   host;
	int                rc;
	
	pthread_mutex_lock(&gw_client.mutex);
	
	/* ---------------------------------------------- */
	/*                Connect to gwd                  */
	/* ---------------------------------------------- */
			
	fd = socket(AF_INET, SOCK_STREAM, 0);
    
    if (fd == -1) 
    {
        perror("socket()");
        
    	pthread_mutex_unlock(&gw_client.mutex);
        return -1;
    }
    
	host = gethostbyname(gw_client.hostname);
	if (host == NULL)
	{
		
#ifdef GWSOLARIS
        fprintf(stderr,"host information for %s not found.\n",gw_client.hostname);
#else
        herror("gethostbyname() ");
#endif				
		pthread_mutex_unlock(&gw_client.mutex);		
		return -1;
	}
	
    gw_addr.sin_family      = AF_INET;
    gw_addr.sin_port        = htons(gw_client.gwd_port);
    memcpy(&(gw_addr.sin_addr.s_addr), host->h_addr_list[0], host->h_length);

	rc = connect(fd, (struct sockaddr *)&gw_addr, sizeof(struct sockaddr));

    if ( rc == -1) 
    {
        perror("connect()");
        
		pthread_mutex_unlock(&gw_client.mutex);        
        return -1;
    }
	
	pthread_mutex_unlock(&gw_client.mutex);
	
	return fd;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void gw_client_disconnect(int fd)
{
	gw_msg_t msg;
	int      length;
	
	pthread_mutex_lock(&gw_client.mutex);

    msg.msg_type = GW_MSG_DISENGAGE;
    length       = sizeof(gw_msg_t);
    
    send(fd,(void *) &msg,length,0);
    
    close(fd);

	pthread_mutex_unlock(&gw_client.mutex);	
}
