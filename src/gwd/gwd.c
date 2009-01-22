/* -------------------------------------------------------------------------- */
/* Copyright 2002-2009 GridWay Team, Distributed Systems Architecture         */
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

#include "gw_rm.h"
#include "gw_dm.h"
#include "gw_tm.h"
#include "gw_em.h"
#include "gw_im.h"
#include "gw_um.h"

#include "gw_job_pool.h"
#include "gw_array_pool.h"
#include "gw_host_pool.h"
#include "gw_user_pool.h"
#include "gw_log.h"
#include "gw_conf.h"

#ifdef HAVE_LIBDB
#include "gw_acct.h"
#endif

#include <signal.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <fcntl.h>
#include <stdlib.h>
#include <dirent.h>
#include <ctype.h>
#include <dirent.h>

/* ------------------------------------------------------------------------- */
/* GLOBAL VARIABLES                                                          */
/* ------------------------------------------------------------------------- */

const char * usage =
"\n gwd [-h] [-v] [-m] [-c]\n\n"
"SYNOPSIS\n"
"  Prints information about all the hosts in the GridWay system (default)\n\n"
"OPTIONS\n"
"  -h            prints this help.\n"
"  -v            prints GridWay version and license\n"
"  -m            runs GridWay daemon in multiuser mode\n"
"  -c            clears previous GridWay state (otherwise, it is recovered)\n"
"  -f            run GridWay in foreground mode (no detached)\n";

const char * susage =
"usage: gwd [-h] [-v] [-m] [-c]\n";

/* -------------------------------------------------------------------------- */

gw_dm_t *dm;
gw_rm_t *rm;
gw_tm_t *tm;
gw_em_t *em;
gw_im_t *im;
gw_um_t *um;

char    *lock; 

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  cond  = PTHREAD_COND_INITIALIZER;

/* ------------------------------------------------------------------------- */
/* LOCAL FUNCTIONS                                                           */
/* ------------------------------------------------------------------------- */

static void gw_register_mads();

int gw_clear_state();

void gw_recover_state();

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void print_license()
{
    printf("Copyright 2002-2009 GridWay Team, Distributed Systems Architecture\n");
    printf("Group, Universidad Complutense de Madrid\n");
    printf("\n");
    printf(GW_VERSION" is distributed and licensed for use under the terms of the\n"); 
    printf("Apache License, Version 2.0 (http://www.apache.org/licenses/LICENSE-2.0).\n");
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void signal_handler (int sig)
{
    pthread_cond_signal(&cond); 
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gwd_main()
{
    struct sigaction  act;
    
    pthread_attr_t    pattr;
    
    gw_array_pool_t * apool;
    gw_job_pool_t   * jpool;
    gw_host_pool_t  * hpool;
    gw_user_pool_t  * upool;

    int fd;
    
#ifdef HAVE_LIBDB    
	int rc;
#endif
	
    /* ----------------------------------------------------------- */
    /* Close stds, we no longer need them                          */
    /* ----------------------------------------------------------- */

    fd = open("/dev/null", O_RDWR|O_CREAT);
    	
    dup2(fd,0);
    dup2(fd,1);    
    dup2(fd,2);
  
    close(fd);	
    
	fcntl(0,F_SETFD,0); /* Keep them open across exec funcs */
    fcntl(1,F_SETFD,0);
    fcntl(2,F_SETFD,0);
	        
    /* ----------------------------------------------------------- */
    /* Capture SIGTERM signal, and ignore SIGPIPE                  */
    /* ----------------------------------------------------------- */
	
    act.sa_handler = signal_handler;
    act.sa_flags   = SA_RESTART;
    sigemptyset(&act.sa_mask);
        
    sigaction(SIGTERM,&act,NULL);
    
    act.sa_handler = SIG_IGN;

    sigaction(SIGPIPE,&act,NULL);
    
    /* ----------------------------------------------------------- */
    /* Init accounting databases                                   */
    /* ----------------------------------------------------------- */

#ifdef HAVE_LIBDB
	
	rc = gw_acct_db_open(GW_TRUE);
    if (  rc != 0  )
    {
        gw_log_print("GW",'E',"Error initializing accounting databases.\n");
        unlink(lock);
        exit(-1);
    }
    
#endif

    /* ----------------------------------------------------------- */
    /* Start Job, Array, Host pool                                 */
    /* ----------------------------------------------------------- */

    jpool = gw_job_pool_init();       
    if (  jpool == NULL  )
    {
        gw_log_print("GW",'E',"Error initializing job pool.\n");
        unlink(lock);
        exit(-1);
    }
    
    apool = gw_array_pool_init();    
    if (  apool == NULL  )
    {
        gw_log_print("GW",'E',"Error initializing array pool.\n");
        unlink(lock);
        exit(-1);
    }

    hpool = gw_host_pool_init();      
    if (  hpool == NULL  )
    {
        gw_log_print("GW",'E',"Error initializing host pool.\n");
        unlink(lock);
        exit(-1);
    }

    upool = gw_user_pool_init();      
    if (  upool == NULL  )
    {
        gw_log_print("GW",'E',"Error initializing user pool.\n");
        unlink(lock);
        exit(-1);
    }   	   	
    
    /* ----------------------------------------------------------- */
    /* Init GWD modules                                            */
    /* ----------------------------------------------------------- */
    
    dm = gw_dm_init();
    if (dm == NULL)
    {
        gw_log_print("GW",'E',"Error initializing Dispatch Manager.\n");
        unlink(lock);
        exit(-1);
    }
    
    im = gw_im_init();	
    if (im == NULL)
    {
        gw_log_print("GW",'E',"Error initializing Information Manager.\n");
        unlink(lock);
        exit(-1);
    }    

    /* ----------------------------------------------------------- */
    /* Init GWD modules                                            */
    /* ----------------------------------------------------------- */
           
    tm = gw_tm_init();
    if (tm == NULL)
    {
        gw_log_print("GW",'E',"Error initializing Transfer Manager.\n");
        unlink(lock);
        exit(-1);
    }
    
    em = gw_em_init();
    if (em == NULL)
    {
        gw_log_print("GW",'E',"Error initializing Execution Manager\n");
        unlink(lock);
        exit(-1);
    }
        
    um = gw_um_init();	
    if (um == NULL)
    {
        gw_log_print("GW",'E',"Error initializing User Manager\n");
        unlink(lock);
        exit(-1);
    }    

    gw_dm_set_tm_am(&(tm->am));
    gw_dm_set_em_am(&(em->am));
    gw_em_set_dm_am(&(dm->am));
    gw_tm_set_dm_am(&(dm->am));
    gw_im_set_dm_am(&(dm->am));
    
    gw_user_pool_set_mad_pipes(em->um_em_pipe_w, tm->um_tm_pipe_w);    

    /* ------------------------------------------------------------ */
    /* Load MADs                                                    */
    /* ------------------------------------------------------------ */

    gw_register_mads();
 
    /* ------------------------------------------------------------ */
    /* Start RM                                                     */
    /* ------------------------------------------------------------ */
    
    rm = gw_rm_init();
    if (rm == NULL)
    {
        gw_log_print("GW",'E',"Error initializing Request Manager\n");
        unlink(lock);
        exit(-1);
    }    

    gw_dm_set_rm_am(&(rm->am));    
    gw_rm_set_dm_am(&(dm->am));    
    
    /* ----------------------------------------------------------- */
    /* Recover state (Warning! After thread start???)              */
    /* ----------------------------------------------------------- */

    gw_log_print("GW",'I',"Recovering GW state.\n");
    gw_recover_state();

    /* ----------------------------------------------------------- */
    /* Start threads                                               */
    /* ----------------------------------------------------------- */
    
    pthread_attr_init (&pattr);
    pthread_attr_setdetachstate (&pattr, PTHREAD_CREATE_JOINABLE);
        
    pthread_create(&(dm->thread_id),&pattr,(void *)gw_dm_start,(void *)NULL);
    pthread_create(&(tm->thread_id),&pattr,(void *)gw_tm_start,(void *)NULL);
    pthread_create(&(em->thread_id),&pattr,(void *)gw_em_start,(void *)NULL);
    pthread_create(&(im->thread_id),&pattr,(void *)gw_im_start,(void *)NULL);
    pthread_create(&(um->thread_id),&pattr,(void *)gw_um_start,(void *)NULL);    
    pthread_create(&(rm->thread_id),&pattr,(void *)gw_rm_start,(void *)NULL);
    
    pthread_mutex_lock(&mutex);
    pthread_cond_wait(&cond, &mutex);
    
    gw_log_print("GW",'I',"SIGTERM received, finalizing.\n");

    gw_am_trigger(&(rm->am), GW_ACTION_FINALIZE, NULL);
    gw_am_trigger(&(dm->am), GW_ACTION_FINALIZE, NULL);
    gw_am_trigger(&(tm->am), GW_ACTION_FINALIZE, NULL);
    gw_am_trigger(&(em->am), GW_ACTION_FINALIZE, NULL);
    gw_am_trigger(&(im->am), GW_ACTION_FINALIZE, NULL);
    gw_am_trigger(&(um->am), GW_ACTION_FINALIZE, NULL);    
    
    pthread_join(rm->thread_id, NULL);              
    pthread_join(dm->thread_id, NULL);
    pthread_join(tm->thread_id, NULL);
    pthread_join(em->thread_id, NULL);
    pthread_join(im->thread_id, NULL);
    pthread_join(um->thread_id, NULL);

    unlink(lock);
        
	gw_job_pool_finalize  ();
	gw_array_pool_finalize();
	gw_host_pool_finalize ();
	gw_user_pool_finalize ();

#ifdef HAVE_LIBDB
	gw_acct_db_close();
#endif
		
    gw_log_print("GW",'I',"All modules finalized, exiting.\n");    
    
    gw_log_destroy();
    
    exit(0);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int main(int argc, char **argv)
{
  	char  opt;
    int  rc, fd;
    char *GW_LOCATION;
    char *log;
    pid_t pid, sid;
    int   length;
    gw_boolean_t multiuser  = GW_FALSE, clear_state = GW_FALSE;
    gw_boolean_t fg_mode = GW_FALSE;
    
    while((opt = getopt(argc,argv,"vhmcf")) != -1)
        switch(opt)
        {
            case 'v':
                print_license();
                exit(0);
                break;
            case 'h':
                printf("%s", usage);
                exit(0);
                break;
            case 'm':
            	multiuser = GW_TRUE;
            	break;        
            case 'c':
            	clear_state = GW_TRUE;
            	break;       
            case 'f':
            	fg_mode  = GW_TRUE;
            	break; 
            default:
                fprintf(stderr,"error: invalid option \'%c\'\n",optopt);
                printf("%s", susage);
                exit(1);
                break;
      	}
    
    /* ------------------------------------ */
    /*   Get Environment & Load conf files  */
    /* ------------------------------------ */

    GW_LOCATION = getenv("GW_LOCATION");
    
    if(GW_LOCATION == NULL)
    {
        fprintf(stderr,"Error! GW_LOCATION environment variable is"
                " undefined.\n");
        return -1;
    }

	length   = strlen(GW_LOCATION) + sizeof(GW_VAR_DIR);
	log  = (char *) malloc (sizeof(char)*(length + 10));
    lock = (char *) malloc (sizeof(char)*(length + 8));

    sprintf(lock, "%s/" GW_VAR_DIR "/.lock", GW_LOCATION);
    sprintf(log,  "%s/" GW_VAR_DIR "/gwd.log", GW_LOCATION);

    /* --------------------------------- */
    /*   Check if other gwd is running   */
    /* --------------------------------- */

    fd = open(lock, O_CREAT|O_EXCL, 0640);

    if( fd == -1)
    {
		switch(errno)
		{
			case EEXIST:
        		fprintf(stderr,"Error! Lock file %s exists.\n",lock);
				break;
			case EACCES:
				fprintf(stderr, "Error! Can not access %s, "
						"check permissions.\n", lock);
				break;
			default:
				fprintf(stderr, "Error! Can not access %s\n", lock);
		}
		exit(-1);
    }

    close(fd);

    /* ---------------------------- */
    /*   Read Configuration file    */
    /* ---------------------------- */
    
    gw_log_init(log);
    
    gw_conf_init(multiuser);
    
    rc = gw_loadconf();    
    
    if (rc != 0)
    {
        printf("ERROR: Loading gwd configuration file: %s "
               "check $GW_LOCATION/" GW_VAR_DIR "/gwd.log\n",log);
        unlink(lock);
        exit(-1);
    }

    if (clear_state)
    {
        gw_log_print("GW",'I',"Clearing last GW state.\n");
        
        rc = gw_clear_state();
        
        if (rc  == -1)
        {
            printf("ERROR: Removing job dirs, check $GW_LOCATION/" 
                   GW_VAR_DIR "/gwd.log\n");
                   
            unlink(lock);
            exit(-1);
        }
    }

    /* ---------------------------- */
    /*   Fork & exit main process   */
    /* ---------------------------- */
    
    if (fg_mode == GW_TRUE) {
       	sprintf(log,"%s/" GW_VAR_DIR "/",GW_LOCATION);
       	rc = chdir(log);

       	free(log);

       	if (rc != 0)
       	{
       		perror("Error, can not change to dir.");
       		unlink(lock);
       		exit(-1);
       	}

       	gwd_main();            
    } 
    else 
    {
       	pid = fork();

       	switch (pid)
       	{
       		case -1: /* Error */
       			fprintf(stderr,"Error! Unable to fork.\n"); 
       			unlink(lock);
       			free(log);
       			exit(-1);
       			break;

       		case 0: /* Child process */
       			sprintf(log,"%s/" GW_VAR_DIR "/",GW_LOCATION);
       			rc = chdir(log);

       			free(log);

       			if (rc != 0)
       			{
       				perror("Error, can not change to dir.");
       				unlink(lock);
       				exit(-1);
       			}

       			sid = setsid();
       			if (sid == -1)
       			{
       				perror("Error, creating new session");
       				unlink(lock);                
       				return -1;
       			}

       			gwd_main();            
       			break;

       		default: /* Parent process */
       			free(log);
       			break;               
        }
    }
    
    return 0;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_register_mads()
{
	int i;
	int rc;
	
	gw_log_print("GW",'I',"Loading Information Manager MADs.\n");
    i = 0;
    while ( ( i < GW_MAX_MADS ) && (gw_conf.im_mads[i][0] != NULL) )
    {
        rc = gw_im_register_mad(gw_conf.im_mads[i][GW_MAD_IM_PATH_INDEX],
                                gw_conf.im_mads[i][GW_MAD_IM_NAME_INDEX],    	
                                gw_conf.im_mads[i][GW_MAD_IM_ARGS_INDEX]);

        if ( rc != 0)
        {
            fprintf(stderr,"Error in Information MAD (%s) initialization, exiting.",
                           gw_conf.im_mads[i][GW_MAD_IM_NAME_INDEX]);
            unlink(lock);               
            exit(-1);
        }
        i++;
    }	

    gw_log_print("GW",'I',"Loading the scheduler.\n");
    
    rc = gw_dm_register_mad(gw_conf.dm_mad[GW_MAD_DM_PATH_INDEX],
                            gw_conf.dm_mad[GW_MAD_DM_NAME_INDEX],
                            gw_conf.dm_mad[GW_MAD_DM_ARGS_INDEX]);

    if ( rc != 0)
    {
        fprintf(stderr,"Error in Scheduler initialization, exiting.");
        unlink(lock);
        exit(-1);
    }	
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

static int gw_remove_dir(const char *path)
{
  DIR *           dirfp;
  struct dirent * dentry;
  struct stat     buf;
  
  int  rc;
  int  error_rc = 0;
  char * dpath;
  int  length;

  dirfp = opendir(path);

  if ( dirfp == NULL )
  {
    error_rc = -1;
    
    if (errno == ENOTDIR)
    {
      rc = unlink(path);
      
      if (rc != 0)
        gw_log_print("GW",'E',"Error removing file %s: %s\n",
                     path,
                     strerror(errno));
      else 
        error_rc = 0;
    }
    else
    {
      gw_log_print("GW",'E',"Error opening directory %s: %s\n",
                   path,
                   strerror(errno));
    }
    
    return error_rc;
  }
   
  while((dentry = readdir(dirfp))!= NULL)
  {
    
    if ((strcmp(dentry->d_name,".")==0)||(strcmp(dentry->d_name,"..")==0))
      continue;
    
    length = strlen(dentry->d_name)+strlen(path)+2;
    dpath  = malloc( length * sizeof(char));    
    
    sprintf(dpath,"%s/%s",path,dentry->d_name);
    
    rc = stat(dpath, &buf);

    if (rc != 0)
    {
      gw_log_print("GW",'E',"Error removing directory %s: %s\n",
                   dpath,
                   strerror(errno));
                   
      error_rc = -1;
         
      free(dpath);           
      continue;
    }
    
    if (S_ISDIR(buf.st_mode))
    {      
      rc = gw_remove_dir(dpath);
      
      if ( rc == -1 )
        error_rc = -1;
    }
    else
    {
      rc = unlink(dpath);
      
      if ( rc != 0 )
      {
        gw_log_print("GW",'E',"Error removing file %s: %s\n",
                     dpath,
                     strerror(errno));
                     
        error_rc = -1;
      }      
    }
    
    free(dpath);
  }

  closedir(dirfp);
  
  rc = rmdir(path);
  
  if ( rc != 0 )
  {
    gw_log_print("GW",'E',"Error removing directory %s: %s\n",
                 path,
                 strerror(errno));
    
    error_rc = -1;
  }

  return error_rc;
}

/* -------------------------------------------------------------------------- */

int gw_clear_state()
{    
    DIR *          vardir;
    struct dirent *dentry;
    
    char   s_vardir[PATH_MAX+1];
    char   j_vardir[PATH_MAX+1];
    int    rc=0;
    
    sprintf(s_vardir,"%s/%s",gw_conf.gw_location,GW_VAR_DIR);
    
    vardir = opendir(s_vardir);

    if ( vardir == NULL )
    {
        gw_log_print("GW",'E',"Error opening directory %s: %s\n",
                     s_vardir,
                     strerror(errno));                     
        return -1;
    }

    while((dentry = readdir(vardir))!= NULL)
    {
        if (isdigit(dentry->d_name[0]))
        {            
            sprintf(j_vardir,"%s/%s",s_vardir,dentry->d_name);
            
            if ( gw_remove_dir(j_vardir) == -1 )
                rc = -1;
        }
    }
  
    closedir(vardir);    
    return rc;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
   
void gw_recover_state()
{
 	DIR *           dir;
  	char *          name;
  	char *          var_name;
  	int             length;
	int             rc;
	int             job_id;
    int *           deps;

	struct stat     buf;
	struct dirent * pdir;
    
    gw_job_t *      job;
  
  	length   = strlen(gw_conf.gw_location) + 2 + sizeof(GW_VAR_DIR);
  	var_name = malloc(sizeof(char)*length);
  	
  	sprintf(var_name,"%s/%s",gw_conf.gw_location,GW_VAR_DIR);
  	
	dir=opendir(var_name);

	if ( dir == NULL )
	{
		gw_log_print("GW",'E',"Could not open directory %s.\n",var_name);
		
		free(var_name);
    	return;
	}

	while((pdir=readdir(dir))!=NULL)
	{
    	if ((strcmp(pdir->d_name,".")==0)||
        	(strcmp(pdir->d_name,"..")==0))
			continue;
      
    	length = strlen(pdir->d_name)+strlen(var_name)+2;
	    name   = malloc( length * sizeof(char));

	    sprintf(name,"%s/%s",var_name,pdir->d_name);

	    rc = stat(name,&buf);

	    if ( rc == 0 )
	    {
			if (S_ISDIR(buf.st_mode) && isdigit(pdir->d_name[0]))
	        {
				job_id = atoi(pdir->d_name);
	        		          
	          	rc     = gw_job_pool_allocate_by_id (job_id);
		    	deps   = NULL;
		    	
	          	if ( rc == job_id )
	          	{
			        job = gw_job_pool_get(job_id, GW_TRUE);
			        rc  = gw_job_recover(job);			        
			        
			        if (rc == 0)
				        gw_job_pool_dep_cp (job->template.job_deps, &deps);
				        
					pthread_mutex_unlock(&(job->mutex));
					
			        if (rc != 0)
			            gw_job_pool_free(job_id);
			        else 
			        {
			        	if ( deps != NULL )
			        	{
			        		if ( deps[0] != -1 )
				    			gw_job_pool_dep_set(job_id, deps);
	    	
				    		free(deps);
			        	}
			        }				        
	          	}
	        }
	    }
	    
    	free(name);    
	}

    gw_job_pool_dep_consistency();
    
    free(var_name);
    closedir(dir);
}
