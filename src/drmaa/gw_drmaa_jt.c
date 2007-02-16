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

#include "gw_drmaa_jt.h"

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pwd.h>
#include <limits.h>
#include <string.h>

/* -------------------------------------------------------------------------- */

static gw_short_string_t drmaa_gw_template_strs[] = {
   "EXECUTABLE",       
   "ARGUMENTS",
   "ENVIRONMENT",
   "INPUT_FILES",
   "OUTPUT_FILES",
   "RESTART_FILES",
   "STDERR_FILE",
   "STDIN_FILE",
   "STDOUT_FILE",   
   "RANK",   
   "REQUIREMENTS",
   "RESCHEDULE_ON_FAILURE",
   "NUMBER_OF_RETRIES",
   "JOB_NAME",
   "JOB_WD",
   "JS_STATE",
   "DEADLINE",
   "TYPE",
   "NP",
   "----"
};

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
                          
int drmaa_gw_jt_init   (drmaa_job_template_t **jt)
{
    char cwd[PATH_MAX+1];
    
    
	*jt = (drmaa_job_template_t *) malloc (sizeof(drmaa_job_template_t));
	
	if ( *jt == NULL )
		return DRMAA_ERRNO_NO_MEMORY;
	
	pthread_mutex_init(&((*jt)->mutex), (pthread_mutexattr_t *) NULL);
	
	pthread_mutex_lock(&((*jt)->mutex));
		
	(*jt)->executable    = NULL;
	(*jt)->num_arg       = 0;
	(*jt)->arguments     = NULL;

	(*jt)->num_env       = 0;
	(*jt)->environment   = NULL;

	(*jt)->num_ifiles    = 0;
	(*jt)->input_files   = NULL;
		
	(*jt)->num_ofiles    = 0;
	(*jt)->output_files  = NULL;

	(*jt)->num_rfiles    = 0;
	(*jt)->restart_files = NULL;
  
	(*jt)->stdin_jt  = NULL;
	(*jt)->stdout_jt = NULL;
	(*jt)->stderr_jt = NULL;	

	(*jt)->rank         = NULL;
	(*jt)->requirements = NULL;
  	
	(*jt)->reschedule_on_failure = strdup("no");
	(*jt)->number_of_retries     = strdup("3");

	(*jt)->job_name = strdup("job_template");

	if ( getcwd(cwd, PATH_MAX) == NULL )
	{
		pthread_mutex_unlock(&((*jt)->mutex));
        return DRMAA_ERRNO_INTERNAL_ERROR;
	}
	
	(*jt)->job_wd   = strdup(cwd);

	(*jt)->js_state = strdup(DRMAA_SUBMISSION_STATE_ACTIVE);
    
    (*jt)->deadline_time = strdup("00");
    
    (*jt)->type = strdup(DRMAA_GW_TYPE_SINGLE);
    (*jt)->np   = strdup("1");
    

/* Not relevant for the current GridWay implementation, will be ignored */
	
	(*jt)->block_email   = NULL;
	(*jt)->hlimit        = NULL;
	(*jt)->slimit        = NULL;
	(*jt)->category      = NULL;
	(*jt)->join_files    = NULL;
	(*jt)->native        = NULL;
	(*jt)->start         = NULL;
	(*jt)->transfer      = NULL;
	(*jt)->wct_hlimit    = NULL;
	(*jt)->wct_slimit    = NULL;

	(*jt)->num_v_email = 0;
	(*jt)->v_email=NULL;
	
	pthread_mutex_unlock(&((*jt)->mutex));
	
	return DRMAA_ERRNO_SUCCESS; 	
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

inline void drmaa_gw_free_strs(char **strs, int num)
{
	int i;

	for(i=0; i<num; i++)
	{
		if ( strs[i] != NULL )
			free(strs[i]);
	}
	
	free(strs);
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

void drmaa_gw_jt_destroy(drmaa_job_template_t *jt)
{
	if ( jt == NULL )
		return;

	pthread_mutex_lock(&(jt->mutex));
	
	if (jt->executable != NULL)
		free(jt->executable);
	
	if (jt->arguments != NULL)
		drmaa_gw_free_strs(jt->arguments, jt->num_arg);
	
	if (jt->environment != NULL)
		drmaa_gw_free_strs(jt->environment, jt->num_env);

	if (jt->input_files != NULL)
		drmaa_gw_free_strs(jt->input_files, jt->num_ifiles);

	if (jt->output_files != NULL)
		drmaa_gw_free_strs(jt->output_files, jt->num_ofiles);
		
	if (jt->restart_files != NULL)
		drmaa_gw_free_strs(jt->restart_files, jt->num_rfiles);

	if (jt->stdin_jt != NULL)
		free(jt->stdin_jt);

	if (jt->stdout_jt != NULL)
		free(jt->stdout_jt);

	if (jt->stderr_jt != NULL)
		free(jt->stderr_jt);

	if (jt->rank != NULL)
		free(jt->rank);
  
	if (jt->requirements != NULL)
		free(jt->requirements);

	if (jt->reschedule_on_failure != NULL)
		free(jt->reschedule_on_failure);

	if (jt->number_of_retries != NULL)
		free(jt->number_of_retries);
  	
	if (jt->job_name != NULL)
		free(jt->job_name);

	if (jt->job_wd != NULL)
		free(jt->job_wd);
		
	if (jt->js_state != NULL)
		free(jt->js_state);
        
    if ( jt->deadline_time != NULL) 
        free( jt->deadline_time);
        
    if ( jt->type != NULL) 
        free( jt->type);

    if ( jt->np != NULL) 
        free( jt->np);
		
/* Not relevant for the current GridWay implementation, will be ignored */	
	if ( jt->block_email   != NULL) 
		free( jt->block_email);
	if ( jt->hlimit        != NULL) 
		free( jt->hlimit);
	if ( jt->slimit        != NULL) 
		free(jt->slimit );
	if ( jt->category      != NULL) 
		free(jt->category );
	if ( jt->join_files    != NULL) 
		free(jt->join_files );
	if ( jt->native        != NULL) 
		free(jt->native );
	if ( jt->start         != NULL) 
		free(jt->start );
	if ( jt->transfer      != NULL) 
		free(jt->transfer );
	if ( jt->wct_hlimit    != NULL) 
		free(jt->wct_hlimit );
	if ( jt->wct_slimit    != NULL) 
		free(jt->wct_slimit );
	if ( jt->v_email       != NULL) 
		drmaa_gw_free_strs(jt->v_email,jt->num_v_email);

	pthread_mutex_unlock(&(jt->mutex));
	
	pthread_mutex_destroy(&(jt->mutex));
			
	free(jt);
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

void drmaa_gw_jt_get_ptr (drmaa_job_template_t * jt,
                          const char *           name,
                          void **                val,
                          int  **                num_val)
{
	*num_val = NULL;
	*val     = NULL;

	if (strcmp(name, DRMAA_ERROR_PATH) == 0)
	{
		*val = (void * ) &(jt->stderr_jt);
	}
	else if (strcmp(name, DRMAA_INPUT_PATH) == 0)
	{
		*val = (void * ) &(jt->stdin_jt);
	}
	else if (strcmp(name, DRMAA_JOB_NAME) == 0)		
	{
		*val = (void * ) &(jt->job_name);
	}
	else if (strcmp(name, DRMAA_JS_STATE) == 0)		
	{
		*val = (void * ) &(jt->js_state);
	}
	else if (strcmp(name, DRMAA_OUTPUT_PATH) == 0)
	{
		*val = (void * ) &(jt->stdout_jt);
	}
	else if (strcmp(name, DRMAA_REMOTE_COMMAND) == 0)		
	{
		*val = (void * ) &(jt->executable);
	}
	else if (strcmp(name, DRMAA_V_ARGV) == 0)
	{
		*num_val = (int *)  &(jt->num_arg);
		*val     = (void *) &(jt->arguments);
	}
	else if (strcmp(name, DRMAA_V_ENV) == 0)		
	{
		*num_val = (int *)   &(jt->num_env);
		*val     = (void * ) &(jt->environment);
	}
	else if (strcmp(name, DRMAA_WD) == 0)		
	{
		*val = (void * ) &(jt->job_wd);
	}
	else if (strcmp(name, DRMAA_V_GW_INPUT_FILES) == 0)		
	{
		*num_val = (int *)   &(jt->num_ifiles);
		*val     = (void * ) &(jt->input_files);
	}
	else if (strcmp(name, DRMAA_V_GW_OUTPUT_FILES) == 0)		
	{
		*num_val = (int *)   &(jt->num_ofiles);
		*val     = (void * ) &(jt->output_files);
	}
	else if (strcmp(name, DRMAA_V_GW_RESTART_FILES) == 0)		
	{
		*num_val = (int *)   &(jt->num_rfiles);
		*val     = (void * ) &(jt->restart_files);
	}
	else if (strcmp(name, DRMAA_GW_RESCHEDULE_ON_FAILURE) == 0)		
	{
		*val = (void * ) &(jt->reschedule_on_failure);
	}
	else if (strcmp(name, DRMAA_GW_NUMBER_OF_RETRIES) == 0)		
	{
		*val = (void * ) &(jt->number_of_retries);
	}
	else if (strcmp(name, DRMAA_GW_RANK) == 0)		
	{
		*val = (void * ) &(jt->rank);
	}
	else if (strcmp(name, DRMAA_GW_REQUIREMENTS) == 0)		
	{
		*val = (void * ) &(jt->requirements);
	}
    else if (strcmp(name, DRMAA_DEADLINE_TIME) == 0)
    {
        *val = (void * ) &(jt->deadline_time);
    }   
    else if (strcmp(name, DRMAA_GW_TYPE) == 0)
    {
        *val = (void * ) &(jt->type);
    }   
    else if (strcmp(name, DRMAA_GW_NP) == 0)
    {
        *val = (void * ) &(jt->np);
    }       
/* Not relevant for the current GridWay implementation, will be ignored */	
	else if (strcmp(name, DRMAA_BLOCK_EMAIL) == 0)
	{
		*val = (void * ) &(jt->block_email);
	}
	else if (strcmp(name, DRMAA_DURATION_HLIMIT) == 0)
	{
		*val = (void * ) &(jt->hlimit);
	}
	else if (strcmp(name, DRMAA_DURATION_SLIMIT) == 0)
	{
		*val = (void * ) &(jt->slimit);
	}			
	else if (strcmp(name, DRMAA_JOB_CATEGORY) == 0)
	{
		*val = (void * ) &(jt->category);
	}
	else if (strcmp(name, DRMAA_JOIN_FILES) == 0)
	{
		*val = (void * ) &(jt->join_files);
	}			
	else if (strcmp(name, DRMAA_NATIVE_SPECIFICATION) == 0)
	{
		*val = (void * ) &(jt->native);
	}
	else if (strcmp(name, DRMAA_START_TIME) == 0)
	{
		*val = (void * ) &(jt->start);
	}			
	else if (strcmp(name, DRMAA_TRANSFER_FILES) == 0)
	{
		*val = (void * ) &(jt->transfer);
	}
	else if (strcmp(name, DRMAA_V_EMAIL) == 0)
	{
		*num_val = (int *)   &(jt->num_v_email);
		*val     = (void * ) &(jt->v_email);
	}
	else if (strcmp(name, DRMAA_WCT_HLIMIT) == 0)
	{
		*val = (void * ) &(jt->wct_hlimit);
	}
	else if (strcmp(name, DRMAA_WCT_SLIMIT) == 0)
	{
		*val = (void * ) &(jt->wct_slimit);
	}			

}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

int  drmaa_gw_jt_set_vval(drmaa_job_template_t * jt, 
                          const char *           var_name,
                          const char **          values)
{
	char *** jt_values;
	void *   ptr;
	int *    jt_num_values;
	int      i;
	int      len;

	if ((jt == NULL) || (var_name == NULL ) || (values == NULL))
		return DRMAA_ERRNO_INVALID_ARGUMENT;
	
	drmaa_gw_jt_get_ptr (jt,var_name,&ptr,&jt_num_values);
	
	jt_values = (char ***) ptr;
	
	if (( jt_values == NULL ) || ( jt_num_values == NULL))
		return DRMAA_ERRNO_SUCCESS;
		
	if ( *jt_values != NULL )
	{
		for (i=0;i<(*jt_num_values);i++)
		{
			if ( (*jt_values)[i] != NULL )
				free((*jt_values)[i]);
		}
		
		free((*jt_values));	
	}
	
	len = 0;
	i   = 0;
	
	while ( values[i] != NULL )
	{
		len = len + 1;
		i   = i   + 1;
	}
	
	if ( len == 0 )
	{
		*jt_values     = NULL;
		*jt_num_values = 0;
		
		return DRMAA_ERRNO_SUCCESS;
	}
	
	*jt_values = (char **) malloc(len*sizeof(char *));
	
	if (*jt_values == NULL )
	{
		*jt_num_values = 0;
		
		return DRMAA_ERRNO_NO_MEMORY;
	}
	
	for (i=0;i<len;i++)
		(*jt_values)[i] = strdup(values[i]);
		
	 *jt_num_values = len;

	 return DRMAA_ERRNO_SUCCESS;
}                          

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

int  drmaa_gw_jt_set_val (drmaa_job_template_t * jt, 
                          const char *           var_name,
                          const char *           values)
{
	char ** jt_values;
	void *  ptr;
	int *   jt_num_values;
	
	if ((jt == NULL) || (var_name == NULL ) || (values == NULL))
		return DRMAA_ERRNO_INVALID_ARGUMENT;
	
	drmaa_gw_jt_get_ptr (jt,var_name,&ptr,&jt_num_values);
	
	jt_values = (char **) ptr;

	if ( jt_values == NULL )
		return DRMAA_ERRNO_SUCCESS;
	
	if ( *jt_values != NULL )
		free(*jt_values);
		
	*jt_values = strdup(values);

	if ( *jt_values == NULL )
		return DRMAA_ERRNO_NO_MEMORY;
	else
		return DRMAA_ERRNO_SUCCESS;
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

char * drmaa_gw_jt_file  (const drmaa_job_template_t *jt)
{
    char * jt_file;
    char * job_name;
    char * job_wd;
    int    file_len;
   
	if ( jt == NULL )
		return NULL;
    		
	job_name = (char *) (jt->job_name);
	job_wd   = (char *) (jt->job_wd);
	
	if ( (job_name == NULL) || (job_wd == NULL) )
		return NULL;
		    
    file_len = strlen(job_name) + strlen(job_wd) + 2;
    jt_file  = (char *) malloc(file_len*sizeof(char));
    
    if (jt_file == NULL)
    	return NULL;
            	
    sprintf(jt_file,"%s/%s",job_wd,job_name);

    return jt_file;
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

char * drmaa_gw_jt_parse(const char * value)
{
	char *          token_start;
	char *          token_end;
	char *          value_cp;
	char *          tmp;
	int             len;
	struct passwd * pw_ent;
	
	if ( value == NULL )
		return NULL;
		
	value_cp = strdup(value);
	
	token_start = strstr(value_cp,DRMAA_PLACEHOLDER_INCR);
	while ( token_start != NULL )
	{
		*token_start ='\0';
		token_end    = token_start + strlen(DRMAA_PLACEHOLDER_INCR);
		
		len = strlen(value_cp)+strlen(token_end)+strlen(DRMAA_GW_PARAM)+1;
		tmp = (char *) malloc(sizeof(char)*len);
		
		sprintf(tmp,"%s%s%s",value_cp,DRMAA_GW_PARAM,token_end);
		
		free (value_cp);
		value_cp = tmp;
		
		token_start = strstr(value_cp,DRMAA_PLACEHOLDER_INCR);
	}

	token_start = strstr(value_cp,DRMAA_PLACEHOLDER_WD);
	while ( token_start != NULL )
	{
		*token_start ='\0';
		token_end    = token_start + strlen(DRMAA_PLACEHOLDER_WD);
		
		len = strlen(value_cp) + strlen(token_end) + 1;
		tmp = (char *) malloc(sizeof(char)*len);
		
		sprintf(tmp,"%s%s",value_cp,token_end);
		
		free (value_cp);
		value_cp = tmp;
		
		token_start = strstr(value_cp,DRMAA_PLACEHOLDER_WD);
	}

	token_start = strstr(value_cp,DRMAA_PLACEHOLDER_HD);
	
	while ( token_start != NULL )
	{
		*token_start ='\0';
		token_end    = token_start + strlen(DRMAA_PLACEHOLDER_HD);
		pw_ent       = getpwuid(getuid());
		
		len = strlen(value_cp)+strlen(token_end)+strlen(pw_ent->pw_dir)+1;
		tmp = (char *) malloc(sizeof(char)*len);
		
		sprintf(tmp,"%s%s%s",value_cp,pw_ent->pw_dir,token_end);
		
		free (value_cp);
		value_cp = tmp;
		
		token_start = strstr(value_cp,DRMAA_PLACEHOLDER_HD);
	}
	
	return value_cp;
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

inline void gw_drmaa_jt_w_var (FILE *fd, int var, const char *val)
{
	char * val_cp;
	
	if ( val != NULL )
	{
		val_cp  = drmaa_gw_jt_parse(val);
		
		if ( val_cp != NULL )
		{
			fprintf(fd,"%s=%s\n",drmaa_gw_template_strs[var],val_cp);
			free(val_cp);
		}
	}
}	

/* ------------------------------------------------------------------------- */

inline void gw_drmaa_jt_w_vvar(FILE *        fd, 
                               int           var, 
                               char **       val,
                               int           num,
                               const char *  ifs)
{
	int    i;
	char * val_cp;
	
	if ((val != NULL) && (num > 0))
	{
		fprintf(fd,"%s= ",drmaa_gw_template_strs[var]);
		
		for (i=0;i<num-1;i++)
		{
			val_cp = drmaa_gw_jt_parse(val[i]);
			fprintf(fd,"%s%s",val_cp,ifs);
			free(val_cp);
		}
		
		val_cp = drmaa_gw_jt_parse(val[num-1]);
		fprintf(fd,"%s\n",val_cp);
		free(val_cp);
	}
}	

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

static void gw_drmaa_gw_str2drmaa_str(char ** str)
{
	char *filename;
	char *hostname;
	char *cp;
	int length;
	
	if ( str == NULL )
		return;
	
	if (*str == NULL)
		return;
	
	hostname = strdup(*str);
		
	filename = strchr(hostname, ':');
	
	if ( filename == NULL )
	{
		free(hostname);
		return;
	}
	
	*filename='\0';
	filename++;
	
	if (( strcmp(hostname,"gsiftp") == 0 )||
	    ( strcmp(hostname,"file") == 0 )  ||
	    ( strcmp(hostname,"http") == 0 )  ||	   
	    ( strcmp(hostname,"https") == 0 ) ) /* a gw filename */
	{
		free(hostname);
		return;
	}
	else if (strlen(hostname) == 0) /* no hostname */
	{
		cp = *str;				
		*str = strdup(cp+1);
		free(cp);
		free(hostname);
		return;
	}
	else /* hostname use gsiftp:// */
	{
		length = strlen(hostname) + strlen (filename) + 11;
		
		free(*str);
		*str = (char *) malloc (sizeof(char)*length);
		
		snprintf(*str,length,"gsiftp://%s/%s",hostname,filename);
		free(hostname);
		
		return;
	}
}


int    gw_drmaa_jt_write (drmaa_job_template_t * jt)
{
    char *jt_file;    
    FILE *fp;
    char *jt_parse;
    
    if ( jt == NULL )
    	return DRMAA_ERRNO_INVALID_ARGUMENT;
        
    jt_file = drmaa_gw_jt_file(jt);

    if (jt_file == NULL)
        return DRMAA_ERRNO_INTERNAL_ERROR;    	
    
    jt_parse = drmaa_gw_jt_parse(jt_file);
        
    if (jt_parse == NULL)
    {
    	free(jt_file);
        return DRMAA_ERRNO_INTERNAL_ERROR;    	
    }
    
    fp = fopen(jt_parse,"w");

    if ( fp == NULL )
    {
        free(jt_file);
        free(jt_parse);
        return DRMAA_ERRNO_INTERNAL_ERROR;        
    }
    
    fprintf(fp,"#This file was automatically generated by the GridWay DRMAA library\n");
    fprintf(fp,"NAME=%s\n",jt->job_name);

	gw_drmaa_jt_w_var (fp,D_EXECUTABLE, jt->executable);
	gw_drmaa_jt_w_vvar(fp,D_ARGUMENTS , jt->arguments, jt->num_arg," ");
	
	gw_drmaa_jt_w_vvar(fp,D_ENVIRONMENT, jt->environment, jt->num_env,", ");
	
	gw_drmaa_jt_w_vvar(fp,D_INPUT_FILES,  jt->input_files,  jt->num_ifiles,", ");
	gw_drmaa_jt_w_vvar(fp,D_OUTPUT_FILES, jt->output_files, jt->num_ofiles,", ");
	gw_drmaa_jt_w_vvar(fp,D_RESTART_FILES,jt->restart_files,jt->num_rfiles,", ");
	
	gw_drmaa_gw_str2drmaa_str(&(jt->stdout_jt));
	gw_drmaa_jt_w_var (fp,D_STDOUT_FILE, jt->stdout_jt);

	gw_drmaa_gw_str2drmaa_str(&(jt->stdin_jt));	
	gw_drmaa_jt_w_var (fp,D_STDIN_FILE , jt->stdin_jt);
	
	gw_drmaa_gw_str2drmaa_str(&(jt->stderr_jt));	
	gw_drmaa_jt_w_var (fp,D_STDERR_FILE, jt->stderr_jt);
	
	gw_drmaa_jt_w_var (fp,D_RANK        , jt->rank);
	gw_drmaa_jt_w_var (fp,D_REQUIREMENTS, jt->requirements);
	
	gw_drmaa_jt_w_var (fp,D_RESCHEDULE_ON_FAILURE, jt->reschedule_on_failure);
	gw_drmaa_jt_w_var (fp,D_NUMBER_OF_RETRIES    , jt->number_of_retries);

    gw_drmaa_jt_w_var (fp,D_DEADLINE, jt->deadline_time);
    
    gw_drmaa_jt_w_var (fp,D_TYPE, jt->type);
    gw_drmaa_jt_w_var (fp,D_NP  , jt->np);

    fclose(fp);
    
    free(jt_parse);
    free(jt_file);
    
    return DRMAA_ERRNO_SUCCESS;			
}                          
