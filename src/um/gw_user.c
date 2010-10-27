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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>


#include "gw_em_mad.h"
#include "gw_user.h"
#include "gw_log.h"
#include "gw_rm_msg.h"

int gw_user_init(gw_user_t *user, const char *name, const char *proxy_path)
{

    int i,j;
    int rc;
    int proxy_var;
    FILE *file;
    char proxy_command[GW_MSG_STRING_LONG], *globus_location;
    char dn[GW_MSG_STRING_LONG], *pline;
 
    if ( user == NULL)
        return -1;
        
    user->name = strdup(name);

    if (user->name == NULL)
        return -1;

    if (strcmp(proxy_path, "") == 0)
    {
#ifdef GWUSERDEBUG
        gw_log_print("UM",'I',"Unsetting X509_USER_PROXY variable.\n");
#endif

        unsetenv("X509_USER_PROXY");
		proxy_var=0;
    }
    else
    {
#ifdef GWUSERDEBUG
        gw_log_print("UM",'I',"Setting X509_USER_PROXY variable to %s.\n",
                proxy_path);
#endif
        proxy_var=1;
        setenv("X509_USER_PROXY", proxy_path, 1);
    }

    user->proxy_path = strdup(proxy_path);
    globus_location = getenv("GLOBUS_LOCATION");

    if (gw_conf.multiuser == GW_TRUE)
        sprintf(proxy_command, "sudo -H -u %s %s/bin/grid-proxy-info -exists", name, globus_location);
    else
        sprintf(proxy_command, "grid-proxy-info -exists");

    if (system(proxy_command) == 0)
    {
        if (gw_conf.multiuser == GW_TRUE)
            sprintf(proxy_command, "sudo -H -u %s %s/bin/grid-proxy-info -identity", name, globus_location);
        else
            sprintf(proxy_command, "grid-proxy-info -identity");

        gw_log_print("UM",'I',"Executing command %s\n", proxy_command);

        file = popen(proxy_command, "r");

        if (file != NULL)
        {
            while( fgets(dn, sizeof(dn), file) != NULL )
            {
                // Keep looping even if we just expect one line
            }
		
            pclose(file);
	  
            if (dn != NULL){
                pline =  strchr(dn, '\n');
            if (pline != NULL)
                *pline = '\0';

            user->dn = strdup(dn);
            gw_log_print("UM",'I',"User proxy info, %s\n", user->dn);
            }
            else
            {
                 gw_log_print("UM",'I',"Error getting identity of user %s.\n",
                         GWNSTR(name));
                user->dn = strdup("Unknown");
            }
        }
        else
        {
            gw_log_print("UM",'I',"Error executing grid-proxy-info -identity for user %s.\n",
                    GWNSTR(name));
            user->dn = strdup("Unknown");
        }
    }
    else
    {
        gw_log_print("UM",'E',"Error executing grid-proxy-info for user %s. Check sudoers.\n",
                GWNSTR(name));
        user->dn = strdup("Unknown"); 
    }

    user->active_jobs  = 0;
    user->running_jobs = 0;
    user->idle         = 0;
    user->em_mads      = 0;
    user->tm_mads      = 0;
    
    gw_log_print("UM",'I',"Loading execution MADs for user %s (%s).\n",
            GWNSTR(name), GWNSTR(user->dn));
    
    i = 0;
    
    while ( ( i < GW_MAX_MADS ) && (gw_conf.em_mads[i][0] != NULL) )
    {
        rc = gw_em_register_mad(user,
                gw_conf.em_mads[i][GW_MAD_EM_PATH_INDEX],
                gw_conf.em_mads[i][GW_MAD_EM_NAME_INDEX],        
                gw_conf.em_mads[i][GW_MAD_EM_ARGS_INDEX],
                gw_conf.em_mads[i][GW_MAD_EM_RSL_INDEX]);

        if (rc != 0)
        {
            gw_log_print("UM",'E',"Could not load execution MAD %s.\n",
                    gw_conf.em_mads[i][GW_MAD_EM_NAME_INDEX]);
        	
            for (j = 0; j< user->em_mads ; j++)
            {
                gw_log_print("UM",'E',"Removing execution MAD %s.\n",
                             user->em_mad[j].name);
                
                gw_em_mad_finalize (&(user->em_mad[j]));
            }
            
            free(user->name);

            return -1;
        }

        i++;
    }
    
    gw_log_print("UM",'I',"Loading transfer MADs for user %s (%s).\n",
            GWNSTR(name), GWNSTR(user->dn));
    
    i = 0;
    while ( ( i < GW_MAX_MADS ) && (gw_conf.tm_mads[i][0] != NULL) )
    {
        rc = gw_tm_register_mad(user,
                gw_conf.tm_mads[i][GW_MAD_TM_PATH_INDEX],
                gw_conf.tm_mads[i][GW_MAD_TM_NAME_INDEX],
                gw_conf.tm_mads[i][GW_MAD_TM_ARGS_INDEX]);

        if ( rc != 0)
        {
            gw_log_print("UM",'E',"Could not load transfer MAD %s.\n",
                    gw_conf.tm_mads[i][GW_MAD_TM_NAME_INDEX]);
        	
            for (j=0; j< user->tm_mads ; j++)
            {  
                gw_log_print("UM",'E',"Removing transfer MAD %s.\n",
                             user->tm_mad[j].name);
            	          	                        
                gw_tm_mad_finalize (&(user->tm_mad[j]));
            }
            
            for (j=0; j< user->em_mads ; j++)
            {
                gw_log_print("UM",'E',"Removing execution MAD %s.\n",
                             user->em_mad[j].name);
                
                gw_em_mad_finalize (&(user->em_mad[j]));
            }

            free(user->name);

            return -1;
        }        
        i++;
    }

    gw_log_print("UM",'I',"User %s (%s) registered.\n",
            GWNSTR(name), GWNSTR(user->dn));
            
    return 0;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_user_destroy(gw_user_t *user)
{
    int i;
        
    if ( user == NULL )
        return;
    
    gw_log_print("UM",'I',"Removing MADs for user %s (%s).\n",
            GWNSTR(user->name), GWNSTR(user->dn));
            
    if (user->name != NULL )
        free(user->name);

    if (user->proxy_path != NULL )
        free(user->proxy_path);

    if (user->dn != NULL )
        free(user->dn);

    for (i = 0; i< user->em_mads; i++)
        gw_em_mad_finalize(&(user->em_mad[i]));

    for (i = 0; i< user->tm_mads; i++)
        gw_tm_mad_finalize(&(user->tm_mad[i]));

}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int gw_em_register_mad(gw_user_t *  user, 
                       const char * executable,
                       const char * name, 
                       const char * args,
                       const char * mode)
{
    int rc, i;

    /* ----------------------------------------------------- */
    /* 1.- Check if there is space left                      */
    /* ----------------------------------------------------- */
    
    if (user == NULL)
    {
        gw_log_print("UM",'E',"\tCould not load execution MAD, user not defined.\n");
        return -1;
    }
        
    if (user->em_mads == GW_MAX_MADS)
    {
        gw_log_print("UM",'E',"\tCould not load execution MAD, max number of MADs reached.\n");
        return -1;
    }
    else
    {
    	for (i=0;i<user->em_mads;i++)
    	{
            if (strcmp(user->em_mad[i].name,name) == 0)
            {
                gw_log_print ("UM",'W',"\tExecution MAD %s already loaded.\n",
                        GWNSTR(name));
                return 0;    			
            }
    	}
    }
	    
    /* ----------------------------------------------------- */
    /* 2.- Init MAD structure and start the driver           */
    /* ----------------------------------------------------- */    
    
    rc = gw_em_mad_init(&(user->em_mad[user->em_mads]),
                        executable,
                        name,
                        args,
                        mode,
                        user->name);

    if ( rc == 0 )
    {
        user->em_mads++;
        
        gw_log_print("UM",'I',"\tExecution MAD %s loaded (exec:%s, args:%s, mode:%s).\n",
                GWNSTR(name),
                GWNSTR(executable),
                GWNSTR(args),
       	        GWNSTR(mode));
    }
    else
        gw_log_print("UM",'E',"\tCould not load execution MAD %s (Check name/path of %s or live proxy).\n",
                     GWNSTR(name), 
                     GWNSTR(executable));

    return rc;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

gw_em_mad_t* gw_em_get_mad(gw_user_t * user, const char *name)
{
    gw_em_mad_t *mad;
    int i;

    if ( user == NULL )
        return NULL;
        
    mad = NULL;

    for (i = 0; i < user->em_mads; i++)
    {
        if (strcmp(name, user->em_mad[i].name) == 0)
        {
            mad = &(user->em_mad[i]);
            break;
        }
    }

    return(mad);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int gw_tm_register_mad(gw_user_t *  user,
        const char * executable, 
        const char * name, 
        const char * arg)
{
    int           rc,i;
         
    /* ----------------------------------------------------- */
    /* 1.- Check if there is space left                      */
    /* ----------------------------------------------------- */    

    if (user == NULL )
    {
        gw_log_print("UM",'E',"\tCould not load transfer MAD, user not defined.\n");
        return -1;
    }
    
    if (user->tm_mads == GW_MAX_MADS)
    {
        gw_log_print("UM",'E',"\tCould not load transfer MAD, max number of MADs reached.\n");
        return -1;
    }
    else
    {
    	for (i=0;i<user->tm_mads;i++)
    	{
    		if (strcmp(user->tm_mad[i].name,name)==0)
    		{
				gw_log_print ("UM",'W',"\tTransfer MAD %s already loaded.\n",
                              GWNSTR(name));			
    			return 0;    			
    		}
    	}
    }
    
    /* ----------------------------------------------------- */
    /* 2.- Init MAD structure and start the driver           */
    /* ----------------------------------------------------- */    

    rc = gw_tm_mad_init(&(user->tm_mad[user->tm_mads]),
                        executable,
                        name,
                        arg,
                        user->name);

    if ( rc == 0 )
    {        
        user->tm_mads++;

        gw_log_print("UM",'I',"\tTransfer MAD %s loaded (exec: %s, arg: %s).\n",
                     GWNSTR(name),
                     GWNSTR(executable),
                     GWNSTR(arg));
    }
    else
        gw_log_print("UM",'E',"\tCould not load transfer MAD %s (Check name/path of %s).\n",
                     GWNSTR(name), 
                     GWNSTR(executable));

    return rc;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

gw_tm_mad_t* gw_tm_get_mad(gw_user_t * user, const char *name)
{
    gw_tm_mad_t *mad;
    int i;

    if ( user == NULL )
        return NULL;
        
    mad = NULL;

    for (i=0; i < user->tm_mads; i++)
    {
        if (strcmp(name, user->tm_mad[i].name) == 0)
        {
            mad = &(user->tm_mad[i]);
            break;
        }
    }
    
    return(mad);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
