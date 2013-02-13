/* -------------------------------------------------------------------------- */
/* Copyright 2002-2013, GridWay Project Leads (GridWay.org)                   */
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

#ifdef HAVE_LIBDB

#include "gw_acct.h"
#include "gw_conf.h"
#include "gw_log.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

/* -------------------------------------------------------------------------- */
static gw_acct_db_t gw_acct_db = {NULL,NULL,NULL,NULL,NULL,NULL};
/* -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int gw_acct_db_open(gw_boolean_t server)
{
	int       rc;
	u_int32_t flags;
	u_int32_t env_flags;
	DB        *db;
	DB_ENV    *db_env;
	char      env_dir[PATH_MAX];
	char *    gw_location;
	mode_t    mask;
	
    gw_location = getenv("GW_LOCATION");
    
    if ( gw_location == NULL)
    {
		if (server == GW_TRUE)
           gw_log_print("GW",'E',"Variable GW_LOCATION is not defined.\n");
        else
			fprintf(stderr,"Variable GW_LOCATION is not defined!\n");
			
        return -1;
    }
     
    snprintf(env_dir, PATH_MAX-1, "%s/" GW_VAR_DIR "/acct/", gw_location);
	
	/* ----- Open the environment ------ */
	
	mask = umask(S_IWOTH);
	
	rc = db_env_create(&(gw_acct_db.env_db), 0);
	
	if ( rc != 0 )
	{
		if (server == GW_TRUE)
			gw_log_print("DB",'E',"Error initializing environment structure (%s).\n",
				db_strerror(rc));
		else
			fprintf(stderr,"Error initializing environment structure (%s).\n",
				db_strerror(rc));
				
		return -1;
	}
	db_env = gw_acct_db.env_db;
	
	env_flags = DB_INIT_CDB	| DB_INIT_MPOOL;
	
	if (server == GW_TRUE)
		env_flags = env_flags | DB_CREATE | DB_THREAD;
	
	rc = db_env->open(db_env,env_dir,env_flags,0664);

	if (rc!=0)
	{
		if (server == GW_TRUE)
			gw_log_print("DB",'E',"Error openning environment (%s).\n",db_strerror(rc));
		else
			fprintf(stderr,"Error openning environment (%s).\n",db_strerror(rc));

		return -1;
	}

	umask(mask);
	
	/* ----- Primary Database, acct.db ------ */
	
	rc = db_create(&(gw_acct_db.acct_db), db_env, 0);
	
	if ( rc != 0 )
	{		
		if (server == GW_TRUE)
			gw_log_print("DB",'E',"Error initializing database structure (%s).\n",
					db_strerror(rc));
		else
			fprintf(stderr,"Error initializing database structure (%s).\n",
					db_strerror(rc));

		return -1;
	}
	
	db = gw_acct_db.acct_db;
	
	if (server == GW_TRUE)
	{
		db->set_errcall(db,gw_acct_db_error);
		db->set_errpfx(db,"ACCT-DB");		
		flags = DB_CREATE | DB_THREAD;
	}
	else
		flags = DB_RDONLY;

	rc = db->open(db,
	              NULL, 
	              "acct.db",
                  NULL, 
                  DB_BTREE, 
                  flags, 
                  0644);
	if (rc!=0)
	{
		if (server == GW_TRUE)
			gw_log_print("DB",'E',"Error openning database.\n");
		else
			fprintf(stderr,"Error openning database.\n");

		return -1;
	}

	/* ----- Secondary Database (user index), uidx.db ------ */
	
	rc = db_create(&(gw_acct_db.uinx_db), db_env, 0);
	
	if ( rc != 0 )
	{		
		if (server == GW_TRUE)
			gw_log_print("DB",'E',"Error initializing database structure (%s).\n",
					db_strerror(rc));
		else
			fprintf(stderr,"Error initializing database structure (%s).\n",
					db_strerror(rc));

		return -1;
	}
	
	db = gw_acct_db.uinx_db;
	
	db->set_flags(db,DB_DUPSORT);

	if (server == GW_TRUE)
	{
		db->set_errcall(db,gw_acct_db_error);
		db->set_errpfx(db,"UINX-DB");		
		
		flags = DB_CREATE | DB_THREAD;
	}
	else
		flags = DB_RDONLY;
	
	rc = db->open(db,
	              NULL, 
	              "uinx.db",
                  NULL, 
                  DB_BTREE, 
                  flags, 
                  0644);
	if (rc!=0)
	{
		if (server == GW_TRUE)
			gw_log_print("DB",'E',"Error openning database.\n");
		else
			fprintf(stderr,"Error openning database.\n");

		return -1;
	}

	gw_acct_db.acct_db->associate(gw_acct_db.acct_db,
	                              NULL,
	                              db,
	                              gw_acct_uidx_cb,
	                              DB_CREATE);
	                              
	/* ----- Secondary Database (host index), hidx.db ------ */
		
	rc = db_create(&(gw_acct_db.hinx_db), db_env, 0);

	if ( rc != 0 )
	{		
		if (server == GW_TRUE)
			gw_log_print("DB",'E',"Error initializing database structure (%s).\n",
					db_strerror(rc));
		else
			fprintf(stderr,"Error initializing database structure (%s).\n",
					db_strerror(rc));

		return -1;
	}
	
	db = gw_acct_db.hinx_db;
	
	db->set_flags(db,DB_DUPSORT);

	if (server == GW_TRUE)
	{
		db->set_errcall(db,gw_acct_db_error);
		db->set_errpfx(db,"HINX-DB");
		
		flags = DB_CREATE | DB_THREAD;
	}
	else
		flags = DB_RDONLY;
	
	rc = db->open(db,
	              NULL, 
	              "hinx.db",
                  NULL, 
                  DB_BTREE, 
                  flags, 
                  0644);
	if (rc!=0)
	{
		if (server == GW_TRUE)
			gw_log_print("DB",'E',"Error openning database.\n");
		else
			fprintf(stderr,"Error openning database.\n");

		return -1;
	}

	gw_acct_db.acct_db->associate(gw_acct_db.acct_db,
	                              NULL,
	                              db,
	                              gw_acct_hidx_cb,
	                              DB_CREATE);

	if (server == GW_TRUE)	                              		
		gw_log_print ("DB",'I',"Accounting databases opened.\n");
	
	return 0;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int gw_acct_uidx_cb(DB * uidx_db, const DBT *pkey, const DBT *pdata, DBT *skey)
{
	gw_acct_data_t *datap;
	
	datap = (gw_acct_data_t *) pdata->data;
	
	memset(skey,0,sizeof(DBT));
	
	skey->data = datap->username;
	skey->size = strlen(datap->username) + 1;
		
	return 0;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int gw_acct_hidx_cb(DB * uidx_db, const DBT *pkey, const DBT *pdata, DBT *skey)
{
	gw_acct_data_t *datap;
	
	datap = (gw_acct_data_t *) pdata->data;
	
	memset(skey,0,sizeof(DBT));
	
	skey->data = datap->hostname;
	skey->size = strlen(datap->hostname) + 1;

	return 0;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_acct_db_close()
{
	int rc;
	if (gw_acct_db.acct_db != NULL)
	  {
		rc = gw_acct_db.acct_db->close(gw_acct_db.acct_db,0);
		if ( rc != 0 ){
		  gw_log_print("DB",'E',"Error closing database (%s).\n",db_strerror(rc));
		}
		gw_acct_db.acct_db = NULL;
	  }

	if ( gw_acct_db.ucursor != NULL )
	{
		gw_acct_db.ucursor->c_close(gw_acct_db.ucursor);
		gw_acct_db.ucursor = NULL;
	}
	
	if (gw_acct_db.uinx_db != NULL)
	{
		rc = gw_acct_db.uinx_db->close(gw_acct_db.uinx_db,0);
		
		if ( rc != 0 )
			gw_log_print("DB",'E',"Error closing database (%s).\n",db_strerror(rc));
			
		gw_acct_db.uinx_db = NULL;
	}

	if ( gw_acct_db.hcursor != NULL )
	{
		gw_acct_db.hcursor->c_close(gw_acct_db.hcursor);
		gw_acct_db.hcursor = NULL;
	}
			
	if (gw_acct_db.hinx_db != NULL)
	{
		rc = gw_acct_db.hinx_db->close(gw_acct_db.hinx_db,0);
		
		if ( rc != 0 )
			gw_log_print("DB",'E',"Error closing database (%s).\n",db_strerror(rc));
			
		gw_acct_db.hinx_db = NULL;			
	}		
	
	if ( gw_acct_db.env_db != NULL )
	{
		rc = gw_acct_db.env_db->close(gw_acct_db.env_db,0);

		if ( rc != 0 )
			gw_log_print("DB",'E',"Error closing environment (%s).\n",db_strerror(rc));
			
		gw_acct_db.env_db = NULL;		
	}
					
	gw_log_print ("DB",'I',"Accounting databases closed.\n");
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

static void gw_acct_db_write_record(gw_job_t *job, gw_history_t *record, int number)
{
	DBT            key;
	DBT            data;	
    gw_acct_key_t  gw_key;
	gw_acct_data_t gw_data;
	
	u_int32_t      flags;
	
	int rc,i;
	
	if ((job == NULL) || ( record == NULL ))
		return;

	/* ----- Set the key ------ */
			
	gw_key.start_time = job->start_time;
	gw_key.job_id     = job->id;	
	gw_key.restarted  = number;	
	
	memset(&key,0,sizeof(DBT));
		
	key.data = &gw_key;
	key.size = sizeof(gw_acct_key_t);

	/* ----- Set the data ------ */
		
	gw_data.rank   = record->rank;
	gw_data.reason = record->reason;	
	
	gw_rm_copy_str_short(job->owner,gw_data.username);
	gw_rm_copy_str_host(record->host->hostname,gw_data.hostname);
	
	for (i=0;i<GW_HISTORY_MAX_STATS;i++)
		gw_data.stats[i] = record->stats[i];
		
	memset(&data,0,sizeof(DBT));
	
	data.data = &gw_data;
	data.size = sizeof(gw_acct_data_t);

	/* ----- Write it ------ */
	
    flags = DB_NOOVERWRITE;
    
	rc = gw_acct_db.acct_db->put(gw_acct_db.acct_db,
                                 NULL,
                                 &key,
                                 &data,
                                 flags);
                                 
    if (rc == DB_KEYEXIST)
		gw_log_print("DB",'W',"Duplicate entry in database for job id=%i, stime=%i.\n",
		             job->id,job->start_time);		    	
}

/* -------------------------------------------------------------------------- */

void gw_acct_write_job(gw_job_t *job)
{
	int           i;
	gw_history_t *r;
	
	if ((job==NULL)||(job->history == NULL))
		return;
	
	for (i=0,r=job->history ; r != NULL ; i++,r=r->next)
		gw_acct_db_write_record(job, r, i);	
		
	gw_acct_db.acct_db->sync(gw_acct_db.acct_db,0);
	gw_acct_db.uinx_db->sync(gw_acct_db.uinx_db,0);
	gw_acct_db.hinx_db->sync(gw_acct_db.hinx_db,0);	
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

static inline time_t gw_acct_total_time(time_t ini, time_t end, time_t aend)
{
	time_t tot = 0;
	
	if (ini != 0)
	{
		if (end != 0)
			tot = end - ini;
		else if (aend != 0)
			tot = aend - ini;
	}

	return tot;
}		

/* -------------------------------------------------------------------------- */

int gw_acct_join_search(const char * hostname, 
                        const char * username,
                        gw_acct_t *  accts,
                        time_t       from_time)
{ 
	DBT ukey;
	DBT hkey;	
	
	DBT pkey;	
	DBT pdata;
	
	DBC *cursors[3];
	DBC *jc;
	
	int rc;
	time_t *s;
	
	gw_acct_data_t gw_data;
	gw_acct_key_t  gw_key;

	time_t par;
	
/* ------ Open cursor on secondary databases ------ */	


	rc = gw_acct_db.hinx_db->cursor(gw_acct_db.hinx_db,
	                                NULL,
	                                &(gw_acct_db.hcursor),
	                                0);
	if ( rc != 0 )
		return -1;
		
	rc = gw_acct_db.uinx_db->cursor(gw_acct_db.uinx_db,
	                                NULL,
	                                &(gw_acct_db.ucursor),
	                                0);
	if ( rc != 0 )
		return -1;
		
		
/* ------ Init search keys on secondary databases ------ */
	                           	                           
	memset(&hkey,0,sizeof(DBT));
		
	hkey.data = (void *) hostname;
	hkey.size = strlen(hostname)+1;

	memset(&ukey,0,sizeof(DBT));
		
	ukey.data = (void *) username;
	ukey.size = strlen(username)+1;

	memset(&pkey,0,sizeof(DBT));

	pkey.data  = &gw_key;
	pkey.ulen  = sizeof(gw_acct_key_t);
	pkey.flags = DB_DBT_USERMEM;

	memset(&pdata,0,sizeof(DBT));

	pdata.data  = &gw_data;
	pdata.ulen  = sizeof(gw_acct_data_t);
	pdata.flags = DB_DBT_USERMEM;

/* ------ Set cursors on index databases ------ */

	rc = gw_acct_db.hcursor->c_get(gw_acct_db.hcursor,&hkey,&pdata,DB_SET);

	if ( rc != 0 )
		return -1;
		
	rc = gw_acct_db.ucursor->c_get(gw_acct_db.ucursor,&ukey,&pdata,DB_SET);

	if ( rc != 0 )
		return -1;

	cursors[0] = gw_acct_db.hcursor;
	cursors[1] = gw_acct_db.ucursor;
	cursors[2] = NULL;
	
	rc =gw_acct_db.acct_db->join(gw_acct_db.acct_db,
	                             cursors,
	                             &jc,
	                             0);
	                         
	if ( rc != 0 )
		return -1;
	
	memset((void *) accts, 0, sizeof(gw_acct_t));

	while ( (rc = jc->c_get(jc,&pkey,&pdata,0)) == 0 )
	{	
		s = gw_data.stats;
		
		/* Take into account jobs started after "from_time" */
		if(gw_key.start_time<from_time) 
		    continue;
		
		/* ---- Agregate total active time ---- */
		
		accts->execution += s[ACTIVE_TIME];
		
		/* ---- Agregate total transfer time ---- */		
		par = gw_acct_total_time(s[PROLOG_START_TIME], s[PROLOG_EXIT_TIME], s[EXIT_TIME]);
		
		accts->transfer += par;
		
		par = gw_acct_total_time(s[EPILOG_START_TIME], s[EPILOG_EXIT_TIME], s[EXIT_TIME]);
		
		accts->transfer += par;
		
		/* ---- Agregate total suspension time ---- */
		
		accts->suspension += s[SUSPENSION_TIME];
		
		/* ---- Agregate execution stats ---- */
		
		accts->tot++;

		switch(gw_data.reason)
		{
			case GW_REASON_NONE:
				accts->succ++;
				break;
								
			case GW_REASON_EXECUTION_ERROR:
				accts->err++;
				break;
	
			case GW_REASON_SUSPENSION_TIME:
				accts->susp++;
				break;
				
			case GW_REASON_RESCHEDULING_TIMEOUT:
				accts->disc++;
				break;
				
			case GW_REASON_USER_REQUESTED:				
				accts->user++;
				break;
			
			case GW_REASON_SELF_MIGRATION:
				accts->self++;
				break;
			
			case GW_REASON_PERFORMANCE:
				accts->perf++;
				break;
			
			case GW_REASON_STOP_RESUME:			
				accts->s_r++;
				break;

			case GW_REASON_KILL:
				accts->kill++;
				break;

			default:
				break;
		}
	}

    /* Close cursors */
	jc->c_close(jc);
	    
	gw_acct_db.hcursor->c_close(gw_acct_db.hcursor);
	gw_acct_db.hcursor = NULL;
	
	gw_acct_db.ucursor->c_close(gw_acct_db.ucursor);
	gw_acct_db.ucursor = NULL;
	
	return 0;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int gw_acct_join_search_by_user(const char *  username, 
                                gw_acct_t *** accts,
                                int *         nrecs,
                                time_t        from_time)
{
	char **hosts = NULL;
	char hostname[GW_MSG_STRING_HOST];
	
	DBT hkey;
	DBT pdata;
	
	DBC *cp;
	
	gw_acct_t * tmp;
	
	int i;
	int rc;
	int nhost = 0;
	
	/* Get the hosts from the host database */
	
	rc = gw_acct_db.hinx_db->cursor(gw_acct_db.hinx_db,
	                                NULL,
	                                &(gw_acct_db.hcursor),
	                                0);
	if ( rc != 0 )
	{
		*accts = NULL;
		*nrecs = 0;
		return -1;	
	}
	
	cp = gw_acct_db.hcursor;
	
    /* --- Iterate over the host database to get all hosts ------ */
	                           	                           
	memset(&hkey,0,sizeof(DBT));

	hkey.data  = (void *) hostname;
	hkey.ulen  = sizeof(char) * GW_MSG_STRING_HOST;
	hkey.flags = DB_DBT_USERMEM;
			
	memset(&pdata,0,sizeof(DBT));
	
	while ((rc = cp->c_get(cp,&hkey,&pdata,DB_NEXT_NODUP))==0)
	{
		nhost  = nhost +1;        
		hosts = realloc((hosts),nhost*sizeof(char *));
		hosts[nhost-1] = malloc(sizeof(char) * GW_MSG_STRING_HOST);
		gw_rm_copy_str_host(hkey.data,hosts[nhost-1]);
	}
	
	if ((rc != DB_NOTFOUND) || (hosts == NULL))
	{
		cp->c_close(cp);
		gw_acct_db.hcursor = NULL;
		
		if (hosts != NULL )
		{
			for (i=0;i<nhost;i++)
				free(hosts[i]);
				
			free(hosts);
		}
		
		*accts = NULL;
		*nrecs = 0;
		return -1;		
	}

    *accts = NULL;
	*nrecs = 0;
	
	cp->c_close(cp);
	gw_acct_db.hcursor = NULL;
	
	for ( i=0;i<nhost;i++)
	{
		tmp = (gw_acct_t *) malloc(sizeof(gw_acct_t));
		rc  = gw_acct_join_search(hosts[i], username, tmp, from_time);
		
		if ( rc == 0 )
		{
			gw_rm_copy_str_host(hosts[i],tmp->name);
			
			*nrecs   = *nrecs + 1;
			*accts = realloc(*accts, *nrecs *sizeof(gw_acct_t *));	
			(*accts)[*nrecs-1] = tmp;
		}
		else
			free(tmp);
	}

	if (hosts != NULL )
	{
		for (i=0;i<nhost;i++)
			free(hosts[i]);
				
		free(hosts);
	}
			
	return 0;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int gw_acct_join_search_by_host(const char *  hostname, 
                                gw_acct_t *** accts,
                                int *         nrecs,
                                time_t        from_time)
{
	char **users = NULL;
	char username[GW_MSG_STRING_SHORT];
	
	DBT hkey;
	DBT pdata;
	
	DBC *cp;
	
	gw_acct_t * tmp;
	
	int i;
	int rc;
	int nuser = 0;
	
	/* Get the users from the user database */
	
	rc = gw_acct_db.uinx_db->cursor(gw_acct_db.uinx_db,
	                                NULL,
	                                &(gw_acct_db.ucursor),
	                                0);
	if ( rc != 0 )
	{
		*accts = NULL;
		*nrecs = 0;
		return -1;	
	}
	
	cp = gw_acct_db.ucursor;
	
    /* --- Iterate over the host database to get all hosts ------ */
	                           	                           
	memset(&hkey,0,sizeof(DBT));

	hkey.data  = (void *) username;
	hkey.ulen  = sizeof(char) * GW_MSG_STRING_SHORT;
	hkey.flags = DB_DBT_USERMEM;
			
	memset(&pdata,0,sizeof(DBT));
	
	while ((rc = cp->c_get(cp,&hkey,&pdata,DB_NEXT_NODUP))==0)
	{
		nuser  = nuser +1;        
		users = realloc((users),nuser*sizeof(char *));
		users[nuser-1] = malloc(sizeof(char) * GW_MSG_STRING_SHORT);
		gw_rm_copy_str_short(hkey.data,users[nuser-1]);		
	}
	
	if ((rc != DB_NOTFOUND) || (users == NULL))
	{
		cp->c_close(cp);
		gw_acct_db.ucursor = NULL;
		
		if (users != NULL )
		{
			for (i=0;i<nuser;i++)
				free(users[i]);
				
			free(users);
		}
		
		*accts = NULL;
		*nrecs = 0;
		return -1;		
	}
	
	*nrecs = 0;
    *accts = NULL;
    	
	cp->c_close(cp);
	gw_acct_db.ucursor = NULL;
	
	for ( i=0;i<nuser;i++)
	{
		tmp = (gw_acct_t *) malloc(sizeof(gw_acct_t));
		rc  = gw_acct_join_search(hostname, users[i], tmp, from_time);
		
		if ( rc == 0 )
		{
			gw_rm_copy_str_short(users[i],tmp->name);
			
			*nrecs   = *nrecs + 1;
			*accts = realloc(*accts, *nrecs *sizeof(gw_acct_t *));	
			(*accts)[*nrecs-1] = tmp;
		}
		else
			free(tmp);
	}

	if (users != NULL )
	{
		for (i=0;i<nuser;i++)
			free(users[i]);
			
		free(users);
	}
	
	return 0;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


int gw_acct_join_search_by_host_and_user(const char * hostname, 
										 const char * username,
										 gw_acct_t ***accts,
                                         int *        nrecs,
                                         time_t       from_time)
{	
	gw_acct_t * tmp;
	
	int rc;
	
	*nrecs = 0;
    *accts = NULL;
    // This sum is for the combination of user+@+host
	int user_at_host_size=GW_MSG_STRING_USER_AT_HOST;
	char user_at_host[user_at_host_size];

	tmp = (gw_acct_t *) malloc(sizeof(gw_acct_t));
	rc  = gw_acct_join_search(hostname, username, tmp, from_time);
	if ( rc == 0 )
	  {
	    sprintf(user_at_host,"%s@%s",username,hostname);
	    gw_rm_copy_str_user_at_host(user_at_host,tmp->name);		

		 /* This always is going to be 1 (username @ hostname)*/
		*nrecs = *nrecs + 1; 
		*accts = realloc(*accts, *nrecs *sizeof(gw_acct_t *));	
		(*accts)[*nrecs-1] = tmp;
	}
	else
		free(tmp);
	

	return 0;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gw_acct_db_error(const DB_ENV *dbenv, const char *prefix, const char *msg)
{
	if (prefix != NULL)
	{
		if (msg != NULL)
		{
			gw_log_print("DB",'E',"%s, %s.\n",prefix, msg);
		}
		else
		{
			gw_log_print("DB",'E',"%s, Error not defined by DB library.\n",prefix);
		}		
	}
	else
	{
		if (msg != NULL)
		{
			gw_log_print("DB",'E',"%s.\n",msg);			
		}
		else
		{
			gw_log_print("DB",'E',"Error not defined by DB library.\n");			
		}				
	}	
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

#endif
