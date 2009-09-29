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

#include "drmaa.h"
#include "gw_drmaa_jt.h"
#include "gw_client.h"

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pwd.h>
#include <pthread.h>

/* ************************************************************************** *
 *                DRMAA DATA TYPES                                            *
 * ************************************************************************** */

struct drmaa_attr_names_s 
{
    int  number_names;    
    int	 current;
    char **names;
};

struct drmaa_attr_values_s
{
    int  number_values;
    int  current;
    char **values;
};

struct drmaa_job_ids_s
{
    int  number_ids;
    int  current;
    char **job_ids;
};

#define DRMAA_GW_SESSION_JOBS 2048

typedef struct drmaa_gw_session_s
{
	pthread_mutex_t mutex;
	
    pid_t session_id;
 
    int   number_of_jobs;
    int   job_ids[DRMAA_GW_SESSION_JOBS];
    
}drmaa_gw_session_t;

int    gw_drmaa_rm_session_id (int jid);


/* ************************************************************************** *
 *                               MODULE GLOBALS                               *
 * ************************************************************************** */

drmaa_gw_session_t drmaa_gw_session={PTHREAD_MUTEX_INITIALIZER, -1};

static gw_short_string_t drmaa_gw_error_strs[] = {
	"Success",
	"Unexpected Error",
	"Could not contact with GWD, check your proxy!",
	"Permision denied",
	"Invalid Argument",
	"No active session",
	"Not enough memory",
	"Invalid contact string",
	"Default contact string error",
	"Unable to initialize GWD",
	"A DRMAA session was already initialized",
	"Could not close connection with GWD",
	"Invalid attribute format",
	"Invalid attribut value",
	"Conflicting attribute values",
	"Try later (max. number of jobs reached)...",
	"Permission denied",
	"Invalid Job ID, it does not exsist",
	"Could not resume job: wrong job state",
	"Could not suspend job: wrong job state",
	"Could not hold job: wrong job state",
	"Could not release job: wrong job state",
	"Time out exceeded",
	"RUSAGE not available",
	"No more elements",
	"*UNKNOWN ERROR*"
};

static gw_short_string_t drmaa_gw_signal_strs[] = {
       "UNKNOWN",
       "SIGHUP",
       "SIGINT",
       "SIGQUIT",
       "SIGILL",
       "SIGTRAP",
       "SIGABRT",
       "SIGBUS",
       "SIGFPE",
       "SIGKILL",
       "SIGUSR1",
       "SIGSEGV",
       "SIGUSR2",
       "SIGPIPE",
       "SIGALRM",
       "SIGTERM",
	   "UNKNOWN"
};

/* ************************************************************************** *
 * ************************************************************************** *
 *             MODULE FUNCTION IMPLEMENTATION (PUBLIC INTERFACE)              *
 * ************************************************************************** *
 * ************************************************************************** */

/* ************************************************************************** *
 * SECTION 2. string list helper functions                                    *
 * ************************************************************************** */

int drmaa_get_next_attr_name(drmaa_attr_names_t* values, char *value,
            size_t value_len)
{    
    if ( values == NULL )
        return DRMAA_ERRNO_INVALID_ARGUMENT;
    
    pthread_mutex_lock(&(drmaa_gw_session.mutex));
    
    if (drmaa_gw_session.session_id == -1)
    {
   	    pthread_mutex_unlock(&(drmaa_gw_session.mutex));
    	return DRMAA_ERRNO_NO_ACTIVE_SESSION;
    }
    
    pthread_mutex_unlock(&(drmaa_gw_session.mutex));
    
    if ( values->current >= values->number_names )
    {
        values->current = 0;    
        return DRMAA_ERRNO_NO_MORE_ELEMENTS;
    }
    
    snprintf(value,value_len,"%s", values->names[values->current]);

    values->current++;
    
    return DRMAA_ERRNO_SUCCESS;
}            

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */

int drmaa_get_next_attr_value(drmaa_attr_values_t* values, char *value,
            size_t value_len)
{    
    if ( values == NULL )
        return DRMAA_ERRNO_INVALID_ARGUMENT;
    
    pthread_mutex_lock(&(drmaa_gw_session.mutex));
    
    if (drmaa_gw_session.session_id == -1)
    {
   	    pthread_mutex_unlock(&(drmaa_gw_session.mutex));
    	return DRMAA_ERRNO_NO_ACTIVE_SESSION;
    }
    
    pthread_mutex_unlock(&(drmaa_gw_session.mutex));
    
    if ( values->current >= values->number_values )
    {
        values->current = 0;    
        return DRMAA_ERRNO_NO_MORE_ELEMENTS;
    }
    
    snprintf(value,value_len,"%s", values->values[values->current]);

    values->current++;
    
    return DRMAA_ERRNO_SUCCESS;    
}            

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
 
int drmaa_get_next_job_id(drmaa_job_ids_t* values, char *value, size_t
            value_len)
{    
	if ( values == NULL )
        return DRMAA_ERRNO_INVALID_ARGUMENT;
    
    pthread_mutex_lock(&(drmaa_gw_session.mutex));
    
    if (drmaa_gw_session.session_id == -1)
    {
   	    pthread_mutex_unlock(&(drmaa_gw_session.mutex));
    	return DRMAA_ERRNO_NO_ACTIVE_SESSION;
    }
    
    pthread_mutex_unlock(&(drmaa_gw_session.mutex));
	    
    if ( values->current >= values->number_ids )
    {
        values->current = 0;    
        return DRMAA_ERRNO_NO_MORE_ELEMENTS;
    }
    
    snprintf(value,value_len,"%s", values->job_ids[values->current]);

    values->current++;
    
    return DRMAA_ERRNO_SUCCESS;    
}            
 
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
 
int drmaa_get_num_attr_names(drmaa_attr_names_t* values, size_t *size)
{    
	if ( values == NULL )
        return DRMAA_ERRNO_INVALID_ARGUMENT;
    
    pthread_mutex_lock(&(drmaa_gw_session.mutex));
    
    if (drmaa_gw_session.session_id == -1)
    {
   	    pthread_mutex_unlock(&(drmaa_gw_session.mutex));
    	return DRMAA_ERRNO_NO_ACTIVE_SESSION;
    }
    
    pthread_mutex_unlock(&(drmaa_gw_session.mutex));
    		    
    *size = values->number_names;
    
    return DRMAA_ERRNO_SUCCESS;    
}            
           
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
 
int drmaa_get_num_attr_values(drmaa_attr_values_t* values, size_t *size)
{    
	if ( values == NULL )
        return DRMAA_ERRNO_INVALID_ARGUMENT;
    
    pthread_mutex_lock(&(drmaa_gw_session.mutex));
    
    if (drmaa_gw_session.session_id == -1)
    {
   	    pthread_mutex_unlock(&(drmaa_gw_session.mutex));
    	return DRMAA_ERRNO_NO_ACTIVE_SESSION;
    }
    
    pthread_mutex_unlock(&(drmaa_gw_session.mutex));
    
    *size = values->number_values;
    
    return DRMAA_ERRNO_SUCCESS;    
}            

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
 
int drmaa_get_num_job_ids(drmaa_job_ids_t* values, size_t *size)
{    
	if ( values == NULL )
        return DRMAA_ERRNO_INVALID_ARGUMENT;
    
    pthread_mutex_lock(&(drmaa_gw_session.mutex));
    
    if (drmaa_gw_session.session_id == -1)
    {
   	    pthread_mutex_unlock(&(drmaa_gw_session.mutex));
    	return DRMAA_ERRNO_NO_ACTIVE_SESSION;
    }
    
    pthread_mutex_unlock(&(drmaa_gw_session.mutex));
    
    *size = values->number_ids;
    
    return DRMAA_ERRNO_SUCCESS;    
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
 
void drmaa_release_attr_names (drmaa_attr_names_t * values)
{
    int i;
    
    if ( values == NULL )
        return;
            
    for (i=0; i < values->number_names; i++)
        free(values->names[i]);
    
    free(values->names);    
    free(values);
}


/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
 
void drmaa_release_attr_values (drmaa_attr_values_t * values)
{
    int i;
    
    if ( values == NULL )
        return;
            
    for (i=0; i < values->number_values; i++)
        free(values->values[i]);
    
    free(values->values);
    free(values);
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
 
void drmaa_release_job_ids (drmaa_job_ids_t * values)
{
    int i;
    
    if ( values == NULL )
        return;
            
    for (i=0; i < values->number_ids; i++)
        free(values->job_ids[i]);
    
    free(values->job_ids);
    free(values);    
}

/* ************************************************************************** *
 * SECTION 3 Session Management                                               *
 * ************************************************************************** */
  
int drmaa_init(const char *contact, char *error, size_t error_len)
{
    gw_client_t * gw_client;
    int           i;
    
    pthread_mutex_lock(&(drmaa_gw_session.mutex));
    
    if ( contact != NULL ) 
    {
       	snprintf(error,
		 error_len,
		 "%s",
       	         drmaa_gw_error_strs[DRMAA_ERRNO_INVALID_CONTACT_STRING]);
    
        pthread_mutex_unlock(&(drmaa_gw_session.mutex));
            	         
       	return DRMAA_ERRNO_INVALID_CONTACT_STRING;
    }
    
    if ( drmaa_gw_session.session_id != -1 )
    {
       	snprintf(error,
		 error_len,
		 "%s",
       	         drmaa_gw_error_strs[DRMAA_ERRNO_ALREADY_ACTIVE_SESSION]);
       	
   	    pthread_mutex_unlock(&(drmaa_gw_session.mutex));
   	    
        return DRMAA_ERRNO_ALREADY_ACTIVE_SESSION;
    }
    
    drmaa_gw_session.session_id     = getpid();    
    drmaa_gw_session.number_of_jobs = 0;
    
    for (i=0 ; i<DRMAA_GW_SESSION_JOBS ; i++)
        drmaa_gw_session.job_ids[i] = -1;   

    gw_client = gw_client_init();
    
    if (gw_client == NULL)
    {
      snprintf(error,
	       error_len,
	       "%s",
	       drmaa_gw_error_strs[DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE]);
       	         
		drmaa_gw_session.session_id = -1;
	    
	    pthread_mutex_unlock(&(drmaa_gw_session.mutex));
        
        return DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE;
    }
    
    pthread_mutex_unlock(&(drmaa_gw_session.mutex));
    
    return DRMAA_ERRNO_SUCCESS;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
            
int drmaa_exit(char *error_diagnosis, size_t error_diag_len)
{
	
	pthread_mutex_lock(&(drmaa_gw_session.mutex));
	
    if (drmaa_gw_session.session_id == -1)
    {
        snprintf(error_diagnosis,
                 error_diag_len,
		 "%s",
                 drmaa_gw_error_strs[DRMAA_ERRNO_NO_ACTIVE_SESSION]);
                 
        pthread_mutex_unlock(&(drmaa_gw_session.mutex));
        
    	return DRMAA_ERRNO_NO_ACTIVE_SESSION;
    }

	gw_client_finalize();
    
    drmaa_gw_session.session_id = -1;
    
    pthread_mutex_unlock(&(drmaa_gw_session.mutex));
    
    return DRMAA_ERRNO_SUCCESS;
}

/* ************************************************************************** *
 * SECTION 4 Job Template Routines                                            *
 * ************************************************************************** */
 
int drmaa_allocate_job_template(drmaa_job_template_t ** jt,
                                char *                  error, 
                                size_t                  error_len)
{
    int  rc;

	pthread_mutex_lock(&(drmaa_gw_session.mutex));
	
    if ( drmaa_gw_session.session_id == -1 )
    {
    	if ( error != NULL )
	  snprintf(error,
			 error_len,
			 "%s",
	       	         drmaa_gw_error_strs[DRMAA_ERRNO_NO_ACTIVE_SESSION]);
	       	         
	    pthread_mutex_unlock(&(drmaa_gw_session.mutex));
	    
        return DRMAA_ERRNO_NO_ACTIVE_SESSION;
    }
    
    pthread_mutex_unlock(&(drmaa_gw_session.mutex));
    
    if (jt == NULL)
    {
    	if ( error != NULL )
	       	snprintf(error,
			 error_len,
			 "%s",
	       	         drmaa_gw_error_strs[DRMAA_ERRNO_INVALID_ARGUMENT]);
       	         
        return DRMAA_ERRNO_INVALID_ARGUMENT;
	}
    	
	rc = drmaa_gw_jt_init (jt);

    if ( rc != DRMAA_ERRNO_SUCCESS )
    {
    	if ( error != NULL )    	
	       	snprintf(error,
			 error_len,
			 "%s",
	       	         drmaa_gw_error_strs[rc]);
    }

    return rc;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
 
int drmaa_delete_job_template(drmaa_job_template_t * jt, 
                              char *                 error, 
                              size_t                 error_len)
{
	pthread_mutex_lock(&(drmaa_gw_session.mutex));
	
    if ( drmaa_gw_session.session_id == -1 )
    {
    	if ( error != NULL )
	       	snprintf(error,
			 error_len,
			 "%s",
	       	         drmaa_gw_error_strs[DRMAA_ERRNO_NO_ACTIVE_SESSION]);
	       	         
	    pthread_mutex_unlock(&(drmaa_gw_session.mutex));
	    
        return DRMAA_ERRNO_NO_ACTIVE_SESSION;
    }
    
    pthread_mutex_unlock(&(drmaa_gw_session.mutex));

	drmaa_gw_jt_destroy(jt);
	
	return DRMAA_ERRNO_SUCCESS;

}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
 
int drmaa_set_attribute(drmaa_job_template_t * jt, 
                        const char *           name, 
                        const char *           value, 
                        char *                 error, 
                        size_t                 error_len)
{    
	int rc;
	
	pthread_mutex_lock(&(drmaa_gw_session.mutex));
	
    if ( drmaa_gw_session.session_id == -1 )
    {
    	if ( error != NULL )
	       	snprintf(error,
			 error_len,
			 "%s",
	       	         drmaa_gw_error_strs[DRMAA_ERRNO_NO_ACTIVE_SESSION]);
	       	         
	    pthread_mutex_unlock(&(drmaa_gw_session.mutex));
	    
        return DRMAA_ERRNO_NO_ACTIVE_SESSION;
    }
    
    pthread_mutex_unlock(&(drmaa_gw_session.mutex));
    
    if (( jt == NULL ) || ( name == NULL ) || ( value == NULL ))
    {
    	if ( error != NULL )
	       	snprintf(error,
			 error_len,
			 "%s",
	       	         drmaa_gw_error_strs[DRMAA_ERRNO_INVALID_ARGUMENT]);
       	         
        return DRMAA_ERRNO_INVALID_ARGUMENT;
	}

	pthread_mutex_lock(&(jt->mutex));
		
	rc = drmaa_gw_jt_set_val (jt, name, value);

	pthread_mutex_unlock(&(jt->mutex));
	
    if ( rc != DRMAA_ERRNO_SUCCESS )
    {
    	if ( error != NULL )
	  snprintf(error,
		   error_len,
		   "%s",
		   drmaa_gw_error_strs[rc]);
    }
       	         
    return rc;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */

int drmaa_get_attribute(drmaa_job_template_t * jt, 
                        const char *           name, 
                        char *                 value,
                        size_t                 value_len, 
                        char *                 error, 
                        size_t                 error_len)
{
	void * jt_val;
	int  * jt_num;
	char ** str_val;
	
	pthread_mutex_lock(&(drmaa_gw_session.mutex));
	
    if ( drmaa_gw_session.session_id == -1 )
    {
    	if ( error != NULL )
	       	snprintf(error,
			 error_len,
			 "%s",
	       	         drmaa_gw_error_strs[DRMAA_ERRNO_NO_ACTIVE_SESSION]);
	       	         
	    pthread_mutex_unlock(&(drmaa_gw_session.mutex));
	    
        return DRMAA_ERRNO_NO_ACTIVE_SESSION;
    }
    
    pthread_mutex_unlock(&(drmaa_gw_session.mutex));
	
	if ((jt == NULL)||(value == NULL)||(name==NULL)||(value_len<=0))
    {
    	if ( error != NULL )
	       	snprintf(error,
			 error_len,
			 "%s",
	       	         drmaa_gw_error_strs[DRMAA_ERRNO_INVALID_ARGUMENT]);
       	         
        return DRMAA_ERRNO_INVALID_ARGUMENT;
    }

	pthread_mutex_lock(&(jt->mutex));
	
    drmaa_gw_jt_get_ptr (jt, name, &jt_val, &jt_num);		
	
	str_val = (char **) jt_val;
	
	if ( str_val == NULL )
	{
		value[0]='\0';
		
		pthread_mutex_unlock(&(jt->mutex));
		
		return DRMAA_ERRNO_INVALID_ARGUMENT;
	}
	else
		strncpy(value, *str_val, value_len);

	pthread_mutex_unlock(&(jt->mutex));

    return DRMAA_ERRNO_SUCCESS;    
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
 
 int drmaa_set_vector_attribute(drmaa_job_template_t * jt, 
                                const char *           name, 
                                const char **          value, 
                                char *                 error, 
                                size_t                 error_len)
{
	int rc;
	
	pthread_mutex_lock(&(drmaa_gw_session.mutex));
	
    if ( drmaa_gw_session.session_id == -1 )
    {
    	if ( error != NULL )
	       	snprintf(error,
			 error_len,
			 "%s",
	       	         drmaa_gw_error_strs[DRMAA_ERRNO_NO_ACTIVE_SESSION]);
	       	         
	    pthread_mutex_unlock(&(drmaa_gw_session.mutex));
	    
        return DRMAA_ERRNO_NO_ACTIVE_SESSION;
    }
    
    pthread_mutex_unlock(&(drmaa_gw_session.mutex));
	
	if ((jt == NULL)||(value == NULL)||(name==NULL))
    {
    	if ( error != NULL )    	
	       	snprintf(error,
			 error_len,
			 "%s",
	       	         drmaa_gw_error_strs[DRMAA_ERRNO_INVALID_ARGUMENT]);
       	         
        return DRMAA_ERRNO_INVALID_ARGUMENT;
    }

	pthread_mutex_lock(&(jt->mutex));
	
	rc = drmaa_gw_jt_set_vval( jt, name, value);

	pthread_mutex_unlock(&(jt->mutex));

    if ( rc != DRMAA_ERRNO_SUCCESS )
    {
    	if ( error != NULL )    	
	       	snprintf(error,
			 error_len,
			 "%s",
	       	         drmaa_gw_error_strs[rc]);
    }
       	         
    return rc;
}
            
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */

int drmaa_get_vector_attribute(drmaa_job_template_t * jt,
                               const char *           name,
                               drmaa_attr_values_t ** values,
                               char *                 error_diagnosis,
                               size_t                 error_diag_len)
{
    char *** jt_values;
	void *   ptr;
	int  *   jt_num_values;
	int      i;
	
	pthread_mutex_lock(&(drmaa_gw_session.mutex));
		
	if ( drmaa_gw_session.session_id == -1 )
    {
    	if ( error_diagnosis != NULL )
	       	snprintf(error_diagnosis,
	       	         error_diag_len,
			 "%s",
	       	         drmaa_gw_error_strs[DRMAA_ERRNO_NO_ACTIVE_SESSION]);

		pthread_mutex_unlock(&(drmaa_gw_session.mutex));
		
        return DRMAA_ERRNO_NO_ACTIVE_SESSION;
    }

	pthread_mutex_unlock(&(drmaa_gw_session.mutex));
	
	if ((jt == NULL)||(name == NULL)||(values == NULL))
    {
    	if ( error_diagnosis != NULL )    	
	       	snprintf(error_diagnosis,
	       	         error_diag_len,
			 "%s",
	       	         drmaa_gw_error_strs[DRMAA_ERRNO_INVALID_ARGUMENT]);
       	         
        return DRMAA_ERRNO_INVALID_ARGUMENT;
    }
	
	*values = (drmaa_attr_values_t *) malloc(sizeof(drmaa_attr_values_t));
	
	if (*values == NULL)
	{
		if ( error_diagnosis != NULL )
	       	snprintf(error_diagnosis,
	       	         error_diag_len,
			 "%s",
	       	         drmaa_gw_error_strs[DRMAA_ERRNO_NO_MEMORY]);
       	         
        return DRMAA_ERRNO_NO_MEMORY;		
	}
	
	pthread_mutex_lock(&(jt->mutex));	
	
	drmaa_gw_jt_get_ptr (jt,name,&ptr,&jt_num_values);
    
    if (ptr == NULL)
    {
    	(*values)->number_values = 1;
    	(*values)->current       = 0;
    	(*values)->values        = (char **) malloc(sizeof(char *));
    	(*values)->values[0]     = (char *)  malloc(sizeof(char));
    	(*values)->values[0][0]  = '\0';

    	if ( error_diagnosis != NULL )    	
	       	snprintf(error_diagnosis,
	       	         error_diag_len,
			 "%s",
	       	         drmaa_gw_error_strs[DRMAA_ERRNO_INVALID_ARGUMENT]);

		pthread_mutex_unlock(&(jt->mutex));
		
        return DRMAA_ERRNO_INVALID_ARGUMENT;    	
    }
    
    jt_values = (char ***) ptr;
    
    if ((*jt_values == NULL) || (jt_num_values == NULL) )
    {
    	(*values)->number_values = 1;
    	(*values)->current       = 0;
    	(*values)->values        = (char **) malloc(sizeof(char *));
    	(*values)->values[0]     = (char *)  malloc(sizeof(char));
    	(*values)->values[0][0]  = '\0';

    	if ( error_diagnosis != NULL )    	
	       	snprintf(error_diagnosis,
	       	         error_diag_len,
			 "%s",
	       	         drmaa_gw_error_strs[DRMAA_ERRNO_INVALID_ARGUMENT]);

		pthread_mutex_unlock(&(jt->mutex));
		
        return DRMAA_ERRNO_INVALID_ARGUMENT;
    }
    else
    {
    	(*values)->number_values = *jt_num_values;
    	(*values)->current       = 0;
    	(*values)->values        = (char **) malloc((*jt_num_values)*sizeof(char *));
    	
    	for (i=0;i<*jt_num_values;i++)
    		(*values)->values[i] = strdup((*jt_values)[i]);
    }

	pthread_mutex_unlock(&(jt->mutex));
	    
    return DRMAA_ERRNO_SUCCESS;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */

int drmaa_get_attribute_names(drmaa_attr_names_t ** values, 
                              char *                error_diagnosis, 
                              size_t                error_diag_len)
{
    int  i = 0;

	pthread_mutex_lock(&(drmaa_gw_session.mutex));
	
	if ( drmaa_gw_session.session_id == -1 )
    {
    	if ( error_diagnosis != NULL )
	       	snprintf(error_diagnosis,
	       	         error_diag_len,
			 "%s",
	       	         drmaa_gw_error_strs[DRMAA_ERRNO_NO_ACTIVE_SESSION]);

		pthread_mutex_unlock(&(drmaa_gw_session.mutex));
		
        return DRMAA_ERRNO_NO_ACTIVE_SESSION;
    }

	pthread_mutex_unlock(&(drmaa_gw_session.mutex));
	
	if (values == NULL)
    {
    	if ( error_diagnosis != NULL )    	
	       	snprintf(error_diagnosis,
	       	         error_diag_len,
			 "%s",
	       	         drmaa_gw_error_strs[DRMAA_ERRNO_INVALID_ARGUMENT]);
       	         
        return DRMAA_ERRNO_INVALID_ARGUMENT;
    }
       
    (*values) = (drmaa_attr_names_t *) malloc(sizeof(drmaa_attr_names_t));
    
    (*values)->names        = (char **) malloc (sizeof(char *) * 11);
    (*values)->number_names = 11;
    (*values)->current      = 0;
    
    (*values)->names[i++] = strdup(DRMAA_ERROR_PATH);
    (*values)->names[i++] = strdup(DRMAA_INPUT_PATH);
    (*values)->names[i++] = strdup(DRMAA_JOB_NAME);
    (*values)->names[i++] = strdup(DRMAA_JS_STATE);
    (*values)->names[i++] = strdup(DRMAA_OUTPUT_PATH);
    (*values)->names[i++] = strdup(DRMAA_REMOTE_COMMAND);
    (*values)->names[i++] = strdup(DRMAA_WD);  
    (*values)->names[i++] = strdup(DRMAA_GW_RESCHEDULE_ON_FAILURE);
    (*values)->names[i++] = strdup(DRMAA_GW_RANK);
    (*values)->names[i++] = strdup(DRMAA_GW_REQUIREMENTS);    
    (*values)->names[i]   = strdup(DRMAA_GW_NUMBER_OF_RETRIES);
    
    return DRMAA_ERRNO_SUCCESS;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */

 int drmaa_get_vector_attribute_names(drmaa_attr_names_t ** values, 
                                      char *                error_diagnosis, 
                                      size_t                error_diag_len)
{
    int  i = 0;

	pthread_mutex_lock(&(drmaa_gw_session.mutex));
	
	if ( drmaa_gw_session.session_id == -1 )
    {
    	if ( error_diagnosis != NULL )
	       	snprintf(error_diagnosis,
	       	         error_diag_len,
			 "%s",
	       	         drmaa_gw_error_strs[DRMAA_ERRNO_NO_ACTIVE_SESSION]);

		pthread_mutex_unlock(&(drmaa_gw_session.mutex));
		
        return DRMAA_ERRNO_NO_ACTIVE_SESSION;
    }

	pthread_mutex_unlock(&(drmaa_gw_session.mutex));
    
	if (values == NULL)
    {
    	if ( error_diagnosis != NULL )    	
	       	snprintf(error_diagnosis,
	       	         error_diag_len,
			 "%s",
	       	         drmaa_gw_error_strs[DRMAA_ERRNO_INVALID_ARGUMENT]);
       	         
        return DRMAA_ERRNO_INVALID_ARGUMENT;
    }
       
    (*values) = (drmaa_attr_names_t *) malloc(sizeof(drmaa_attr_names_t ));
    
    (*values)->names        = (char **) malloc (sizeof(char *) * 5);
    (*values)->number_names = 5;
    (*values)->current      = 0;
    
    (*values)->names[i++] = strdup(DRMAA_V_GW_INPUT_FILES);
    (*values)->names[i++] = strdup(DRMAA_V_GW_OUTPUT_FILES);
    (*values)->names[i++] = strdup(DRMAA_V_GW_RESTART_FILES);
    (*values)->names[i++] = strdup(DRMAA_V_ARGV);
    (*values)->names[i]   = strdup(DRMAA_V_ENV);
    
    return DRMAA_ERRNO_SUCCESS;
}
 
/* ************************************************************************** *
 * SECTION 6 Job Submission                                                   *
 * ************************************************************************** */

int drmaa_run_job(char *                 job_id, 
                  size_t                 job_id_len, 
                  drmaa_job_template_t * jt, 
                  char *                 error, 
                  size_t                 error_len)
{
    char *jt_file;
    char *jt_parse;
    int  rc;
    int  jid;
    gw_return_code_t grc;

	pthread_mutex_lock(&(drmaa_gw_session.mutex));
	    
	if ( drmaa_gw_session.session_id == -1 )
    {
    	if ( error != NULL )
	       	snprintf(error,
			 error_len,
			 "%s",
	       	         drmaa_gw_error_strs[DRMAA_ERRNO_NO_ACTIVE_SESSION]);

		pthread_mutex_unlock(&(drmaa_gw_session.mutex));
		
        return DRMAA_ERRNO_NO_ACTIVE_SESSION;
    }
	    
    if ( drmaa_gw_session.number_of_jobs >= DRMAA_GW_SESSION_JOBS )
    {
    	if ( error != NULL )    	    	
	       	snprintf(error,
			 error_len,
			 "%s",
	       	         drmaa_gw_error_strs[DRMAA_ERRNO_TRY_LATER]);

		pthread_mutex_unlock(&(drmaa_gw_session.mutex));
		
        return DRMAA_ERRNO_TRY_LATER;
    }

	pthread_mutex_unlock(&(drmaa_gw_session.mutex));
	
	if ( (job_id == NULL) || ( jt == NULL ) )
    {
    	if ( error != NULL )    	
	       	snprintf(error,
			 error_len,
			 "%s",
	       	         drmaa_gw_error_strs[DRMAA_ERRNO_INVALID_ARGUMENT]);
       	         
        return DRMAA_ERRNO_INVALID_ARGUMENT;
    }    
    
   	pthread_mutex_lock(&(jt->mutex));
   	
    rc = gw_drmaa_jt_write (jt);
    
    if ( rc != DRMAA_ERRNO_SUCCESS )
    {
	   	if ( error != NULL )    	
	       	snprintf(error,
			 error_len,
			 "%s",
			 drmaa_gw_error_strs[rc]);
	       	         
       	pthread_mutex_unlock(&(jt->mutex));
       	
        return rc;
    }
    
    jt_file = drmaa_gw_jt_file(jt);
    
    if (jt_file == NULL)
    {
    	pthread_mutex_unlock(&(jt->mutex));
        return DRMAA_ERRNO_INTERNAL_ERROR;
    }  
        
    jt_parse = drmaa_gw_jt_parse(jt_file);
        
    if (jt_parse == NULL)
    {
    	free(jt_file);
    	
    	pthread_mutex_unlock(&(jt->mutex));
        return DRMAA_ERRNO_INTERNAL_ERROR; 
    }
        
    if (strcmp(jt->js_state,DRMAA_SUBMISSION_STATE_HOLD) == 0)
	    grc = gw_client_job_submit(jt_parse,
	                               GW_JOB_STATE_HOLD, 
	                               &jid,
	                               NULL,
	                               GW_JOB_DEFAULT_PRIORITY);
	else
	    grc = gw_client_job_submit(jt_parse,
	                               GW_JOB_STATE_PENDING,
	                               &jid,
	                               NULL,
	                               GW_JOB_DEFAULT_PRIORITY);
	    
	pthread_mutex_unlock(&(jt->mutex));
	
    if ( grc != GW_RC_SUCCESS )
    {
        free(jt_file);
        free(jt_parse);
    
    	if ( error != NULL )    
	  snprintf(error,
		   error_len,
		   "%s",
		   drmaa_gw_error_strs[DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE]);

        return DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE;    
    }

	pthread_mutex_lock(&(drmaa_gw_session.mutex));
	
    drmaa_gw_session.job_ids[drmaa_gw_session.number_of_jobs] = jid;
    
    drmaa_gw_session.number_of_jobs++;
    
    pthread_mutex_unlock(&(drmaa_gw_session.mutex));

    snprintf(job_id, job_id_len,"%i",jid);
        
    free(jt_file);
    free(jt_parse);   
    
    return DRMAA_ERRNO_SUCCESS;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
     
int drmaa_run_bulk_jobs(drmaa_job_ids_t **     jobids, 
                        drmaa_job_template_t * jt, 
                        int                    start, 
                        int                    end, 
                        int                    incr,
                        char *                 error, 
                        size_t                 error_len)
{
    int    total_jobs;
    int  * job_ids;
    int    array_id;
    char * jt_file;
	int    rc;
	int    jid;
	int    i;
    char * jt_parse;
    
    gw_return_code_t grc;

    total_jobs = (int) (((end - start)/incr)+1);
    
    pthread_mutex_lock(&(drmaa_gw_session.mutex));
        
	if ( drmaa_gw_session.session_id == -1 )
    {
    	if ( error != NULL )
	       	snprintf(error,
			 error_len,
			 "%s",
	       	         drmaa_gw_error_strs[DRMAA_ERRNO_NO_ACTIVE_SESSION]);
	       	         
	    pthread_mutex_unlock(&(drmaa_gw_session.mutex));
	    
        return DRMAA_ERRNO_NO_ACTIVE_SESSION;
    }
        
	if ( (jobids == NULL) || ( jt == NULL ) || ( total_jobs == 0 ) )
    {
    	if ( error != NULL )    	
	       	snprintf(error,
			 error_len,
			 "%s",
	       	         drmaa_gw_error_strs[DRMAA_ERRNO_INVALID_ARGUMENT]);

	    pthread_mutex_unlock(&(drmaa_gw_session.mutex));
	    
        return DRMAA_ERRNO_INVALID_ARGUMENT;
    }    
    
    
    if ( drmaa_gw_session.number_of_jobs + total_jobs >= DRMAA_GW_SESSION_JOBS )
    {
    	if ( error != NULL )
	  snprintf(error,
		   error_len,
		   "%s",
		   drmaa_gw_error_strs[DRMAA_ERRNO_TRY_LATER]);

	    pthread_mutex_unlock(&(drmaa_gw_session.mutex));
	    
        return DRMAA_ERRNO_TRY_LATER;
    }
    
    pthread_mutex_lock(&(jt->mutex));
    
    rc = gw_drmaa_jt_write (jt);
    
    if ( rc != DRMAA_ERRNO_SUCCESS )
    {
    	if ( error != NULL )
	       	snprintf(error,
			 error_len,
			 "%s",
	       	         drmaa_gw_error_strs[rc]);
	       	         
        pthread_mutex_unlock(&(jt->mutex));
        pthread_mutex_unlock(&(drmaa_gw_session.mutex));
        
        return rc;
    }
    
	jt_file = drmaa_gw_jt_file(jt); 
    
    if (jt_file == NULL)
    {
        pthread_mutex_unlock(&(jt->mutex));
        pthread_mutex_unlock(&(drmaa_gw_session.mutex));
    
        return DRMAA_ERRNO_INTERNAL_ERROR;   
    }
        
    jt_parse = drmaa_gw_jt_parse(jt_file);
        
    if (jt_parse == NULL)
    {
    	free(jt_file);
    	
        pthread_mutex_unlock(&(jt->mutex));
        pthread_mutex_unlock(&(drmaa_gw_session.mutex));
    	
        return DRMAA_ERRNO_INTERNAL_ERROR; 
    }
    
    
    if (strcmp(jt->js_state,DRMAA_SUBMISSION_STATE_HOLD) == 0)
	    grc = gw_client_array_submit(jt_parse,
	                                 total_jobs,
	                                 GW_JOB_STATE_HOLD, 
	                                 &array_id,
	                                 &job_ids,
	                                 NULL,
	                                 start,
	                                 incr,
	                                 GW_JOB_DEFAULT_PRIORITY);
	else
	    grc = gw_client_array_submit(jt_parse,
	                                 total_jobs,
	                                 GW_JOB_STATE_PENDING, 
	                                 &array_id,
	                                 &job_ids,
	                                 NULL,
	                                 start,
	                                 incr,
	                                 GW_JOB_DEFAULT_PRIORITY);
	                                 
	pthread_mutex_unlock(&(jt->mutex));
	                                     	    
    if ( grc != GW_RC_SUCCESS )
    {
        free(jt_file);
        free(jt_parse);
        
        if ( grc == GW_RC_FAILED_NO_MEMORY )
        	rc = DRMAA_ERRNO_NO_MEMORY;
        else
        	rc = DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE;
    
    	if ( error != NULL )    	
	       	snprintf(error,
			 error_len,
			 "%s",
	       	         drmaa_gw_error_strs[rc]);
	       	         
        pthread_mutex_unlock(&(drmaa_gw_session.mutex));
        return rc;    
    }
    
    *jobids = (drmaa_job_ids_t *) malloc (sizeof(drmaa_job_ids_t));
    
    if ((*jobids) == NULL )
    {
    	free(jt_file);
	    free(jt_parse);
	    
        pthread_mutex_unlock(&(drmaa_gw_session.mutex));
	    return DRMAA_ERRNO_NO_MEMORY;
    }
	    
    (*jobids)->number_ids = total_jobs;
    (*jobids)->job_ids    = (char **) malloc (sizeof(char *) * total_jobs);
    (*jobids)->current    = 0;
    
    if ((*jobids)->job_ids == NULL)
    {
	    (*jobids)->number_ids = 0;
	    free(jt_file);
	    free(jt_parse);
	    
        pthread_mutex_unlock(&(drmaa_gw_session.mutex));
        
       	return DRMAA_ERRNO_NO_MEMORY;	
    }
    
    for ( i=0; i<total_jobs ; i++)
    {
        jid = job_ids[i];
        
        (*jobids)->job_ids[i] = (char *) malloc(DRMAA_JOBNAME_BUFFER * sizeof(char));
	    if ((*jobids)->job_ids[i] == NULL)
    	{
		    free(jt_file);
		    free(jt_parse);
	    	(*jobids)->number_ids = i;
   	        
   	        pthread_mutex_unlock(&(drmaa_gw_session.mutex));
       		
       		return DRMAA_ERRNO_NO_MEMORY;	
    	}
        
        snprintf((*jobids)->job_ids[i], DRMAA_JOBNAME_BUFFER,"%i",jid);
        
        drmaa_gw_session.job_ids[drmaa_gw_session.number_of_jobs] = jid;        
        drmaa_gw_session.number_of_jobs++;  
    }

    pthread_mutex_unlock(&(drmaa_gw_session.mutex));
    
    free(jt_file);
    free(jt_parse);
    
    return DRMAA_ERRNO_SUCCESS;
}

/* ************************************************************************** *
 * SECTION 7. job status and control                                          *
 * ************************************************************************** */

static inline int drmaa_gw_control_job(int jid, int action)
{
	int grc;
	
    switch(action)
    {
        case DRMAA_CONTROL_SUSPEND:
            grc = gw_client_job_signal(jid, GW_CLIENT_SIGNAL_STOP,   GW_TRUE);
            break;  
        
        case DRMAA_CONTROL_RESUME:
            grc = gw_client_job_signal(jid, GW_CLIENT_SIGNAL_RESUME, GW_TRUE);
            break;

        case DRMAA_CONTROL_TERMINATE:
            grc = gw_client_job_signal(jid, GW_CLIENT_SIGNAL_KILL,   GW_TRUE);
            break;   

        case DRMAA_CONTROL_HOLD:  
	        grc = gw_client_job_signal(jid, GW_CLIENT_SIGNAL_HOLD,   GW_TRUE);
            break;
                  
        case DRMAA_CONTROL_RELEASE:
	        grc = gw_client_job_signal(jid, GW_CLIENT_SIGNAL_RELEASE,GW_TRUE);
            break;
                  
        default:
            return GW_RC_FAILED;
            break;
    }
    		
	return grc;
}

int drmaa_control(const char *jobid, int action, char *error, size_t error_len)
{
    int              jid;
    gw_return_code_t grc;
	int              all_rc = GW_RC_SUCCESS;
	int              i;

    pthread_mutex_lock(&(drmaa_gw_session.mutex));
    
	if ( drmaa_gw_session.session_id == -1 )
    {
    	if ( error != NULL )
	       	snprintf(error,
			 error_len,
			 "%s",
	       	         drmaa_gw_error_strs[DRMAA_ERRNO_NO_ACTIVE_SESSION]);
	       	         
        pthread_mutex_unlock(&(drmaa_gw_session.mutex));
       	         
        return DRMAA_ERRNO_NO_ACTIVE_SESSION;
    }

    pthread_mutex_unlock(&(drmaa_gw_session.mutex));
    
	if (jobid == NULL)
    {
    	if ( error != NULL )    	
	       	snprintf(error,
			 error_len,
			 "%s",
	       	         drmaa_gw_error_strs[DRMAA_ERRNO_INVALID_ARGUMENT]);
       	         
        return DRMAA_ERRNO_INVALID_ARGUMENT;
    } 
    	
	if (strcmp(jobid,DRMAA_JOB_IDS_SESSION_ALL) == 0)
	{
		pthread_mutex_lock(&(drmaa_gw_session.mutex));
		
		for (i=0;i<drmaa_gw_session.number_of_jobs;i++)
		{
			if ( drmaa_gw_session.job_ids[i] != -1 )
			{
				grc = drmaa_gw_control_job(drmaa_gw_session.job_ids[i],action);
			
				if (grc != GW_RC_SUCCESS) 
					if (all_rc == GW_RC_SUCCESS)
						all_rc = grc;
			}
		}
		
		pthread_mutex_unlock(&(drmaa_gw_session.mutex));
	}
	else
	{	
    	jid    = atoi (jobid);
		all_rc = drmaa_gw_control_job(jid,action);
	}
	
	if ( all_rc == GW_RC_FAILED_BAD_JOB_STATE )
	{
		if ( action == DRMAA_CONTROL_SUSPEND )
			all_rc = DRMAA_ERRNO_SUSPEND_INCONSISTENT_STATE;
		else if ( action == DRMAA_CONTROL_RESUME )
			all_rc = DRMAA_ERRNO_RESUME_INCONSISTENT_STATE;
		else if ( action == DRMAA_CONTROL_HOLD )
			all_rc = DRMAA_ERRNO_HOLD_INCONSISTENT_STATE;
		else if ( action == DRMAA_CONTROL_RELEASE )
			all_rc = DRMAA_ERRNO_RELEASE_INCONSISTENT_STATE;
	}
	else if ( all_rc == GW_RC_FAILED_BAD_JOB_ID )
		all_rc = DRMAA_ERRNO_INVALID_JOB;
	else if ( all_rc == GW_RC_FAILED_CONNECTION )
		all_rc = DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE;
	else if ( all_rc == GW_RC_SUCCESS )
		all_rc = DRMAA_ERRNO_SUCCESS;
	else
		all_rc = DRMAA_ERRNO_INVALID_ARGUMENT;
    
    if ( all_rc != DRMAA_ERRNO_SUCCESS )
		if ( error != NULL )    
	       	snprintf(error,
			 error_len,
			 "%s",
	       	         drmaa_gw_error_strs[all_rc]);

    return all_rc;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
      
int drmaa_job_ps(const char * job_id, 
                 int *        remote_ps, 
                 char *       error, 
                 size_t       error_len)
{
    int              jid;
    gw_return_code_t grc;
    gw_msg_job_t     status;
    int              i     = 0;
    int              found = 0;

	pthread_mutex_lock(&(drmaa_gw_session.mutex));
	
	if ( drmaa_gw_session.session_id == -1 )
    {
    	if ( error != NULL )
	       	snprintf(error,
			 error_len,
			 "%s",
	       	         drmaa_gw_error_strs[DRMAA_ERRNO_NO_ACTIVE_SESSION]);
    
	    pthread_mutex_unlock(&(drmaa_gw_session.mutex));
	    
        return DRMAA_ERRNO_NO_ACTIVE_SESSION;
    }
    
	pthread_mutex_unlock(&(drmaa_gw_session.mutex));
	
	if ((job_id == NULL)||(remote_ps == NULL ))
    {
    	if ( error != NULL )    	
	       	snprintf(error,
			 error_len,
			 "%s",
	       	         drmaa_gw_error_strs[DRMAA_ERRNO_INVALID_ARGUMENT]);
       	         
        return DRMAA_ERRNO_INVALID_ARGUMENT;
    }     

    jid = atoi (job_id);

    grc = gw_client_job_status(jid, &status);

    if ( grc == GW_RC_FAILED_CONNECTION )
    {   
		if ( error != NULL )    		
	       	snprintf(error,
	       	         error_len,
			 "%s",
	       	         drmaa_gw_error_strs[DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE]);
    	
        *remote_ps = DRMAA_PS_UNDETERMINED;
		
		return DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE;
    }
    else if ( grc == GW_RC_FAILED_BAD_JOB_ID )
    {
    	pthread_mutex_lock(&(drmaa_gw_session.mutex));
    	
	   	while ( (!found) && (i < DRMAA_GW_SESSION_JOBS) )
        	if ( drmaa_gw_session.job_ids[i] == jid )
	            found = 1;
	        else
	            i++;    	
	    
	    pthread_mutex_unlock(&(drmaa_gw_session.mutex));
	    
    	if ( found ) /* Job has been signaled with TERMINATE */
    	{
        	*remote_ps = DRMAA_PS_FAILED;
        	
        	return DRMAA_ERRNO_SUCCESS;
    	}
    	else  if ( error != NULL )
    	{
	       	snprintf(error,
	       	         error_len,
			 "%s",
	       	         drmaa_gw_error_strs[DRMAA_ERRNO_INVALID_JOB]);
	       	        
    	
        	*remote_ps = DRMAA_PS_UNDETERMINED;
		
			return DRMAA_ERRNO_INVALID_JOB;
    	}
    }
    
    switch(status.job_state)
    {
        case GW_JOB_STATE_INIT:  
            *remote_ps = DRMAA_PS_UNDETERMINED;
            break;    

        case GW_JOB_STATE_PENDING:
            *remote_ps = DRMAA_PS_QUEUED_ACTIVE;
            break;    
            
        case GW_JOB_STATE_HOLD:  
            *remote_ps = DRMAA_PS_USER_ON_HOLD;
            break;
            
        case GW_JOB_STATE_WRAPPER:
        case GW_JOB_STATE_PRE_WRAPPER:
        case GW_JOB_STATE_PROLOG:
        case GW_JOB_STATE_EPILOG:
		case GW_JOB_STATE_EPILOG_STD:
   		case GW_JOB_STATE_EPILOG_RESTART:
        case GW_JOB_STATE_EPILOG_FAIL:
   		case GW_JOB_STATE_STOP_CANCEL:
        case GW_JOB_STATE_STOP_EPILOG:
        case GW_JOB_STATE_KILL_CANCEL:
        case GW_JOB_STATE_KILL_EPILOG:
        case GW_JOB_STATE_MIGR_CANCEL:
        case GW_JOB_STATE_MIGR_PROLOG:
        case GW_JOB_STATE_MIGR_EPILOG:        
            *remote_ps = DRMAA_PS_RUNNING; 
            break;    

        case GW_JOB_STATE_STOPPED:
            *remote_ps = DRMAA_PS_USER_SUSPENDED; 
            break;    

        case GW_JOB_STATE_ZOMBIE:
            *remote_ps = DRMAA_PS_DONE;
            break;    

        case GW_JOB_STATE_FAILED:
            *remote_ps = DRMAA_PS_FAILED;
            break;    

        default:
            *remote_ps = DRMAA_PS_UNDETERMINED;
            break;    
    }
    
    return DRMAA_ERRNO_SUCCESS;
}            

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */

/* ************************************************************************** *
 * SECTION 8. Job Synchronize and Wait                                        *
 * ************************************************************************** */
            
int drmaa_synchronize(const char * job_ids[], 
                      signed long  timeout, 
                      int          dispose, 
                      char *       error, 
                      size_t       error_len)
{
    int *            jids;
    int              number_jobs;
    int              i;
    int *            exit_codes;
    gw_return_code_t grc;
    int              rc;
    int              all_jobs;

	pthread_mutex_lock(&(drmaa_gw_session.mutex));
	
	if ( drmaa_gw_session.session_id == -1 )
    {
		if ( error != NULL )    	
	       	snprintf(error,
	       	         error_len,
			 "%s",
	       	         drmaa_gw_error_strs[DRMAA_ERRNO_NO_ACTIVE_SESSION]);
	       	         
	    pthread_mutex_unlock(&(drmaa_gw_session.mutex));
	    
        return DRMAA_ERRNO_NO_ACTIVE_SESSION;
    }
    
    pthread_mutex_unlock(&(drmaa_gw_session.mutex));
    
    if (job_ids == NULL)
    {
		if ( error != NULL )    	
	       	snprintf(error,
	       	         error_len,
			 "%s",
	       	         drmaa_gw_error_strs[DRMAA_ERRNO_INVALID_ARGUMENT]);
       	         
        return DRMAA_ERRNO_INVALID_ARGUMENT;
    }   

	all_jobs = strcmp(job_ids[0], DRMAA_JOB_IDS_SESSION_ALL) == 0;
	
    if (all_jobs)
    {
    	pthread_mutex_lock(&(drmaa_gw_session.mutex));

		jids = (int *) malloc(sizeof(int)*(drmaa_gw_session.number_of_jobs+1));
	
		for (i=0;i<=drmaa_gw_session.number_of_jobs;i++)
			jids[i] = drmaa_gw_session.job_ids[i];
    	
        number_jobs = drmaa_gw_session.number_of_jobs;

    	pthread_mutex_unlock(&(drmaa_gw_session.mutex));
    	    
	    if ( number_jobs == 0 )
	    {
	    	free(jids);
			return DRMAA_ERRNO_SUCCESS;
	    }
    }
    else
    {
        number_jobs = 0;
        
        while (job_ids[number_jobs] != NULL)
            number_jobs++;
            
        jids = (int *) malloc (sizeof(int) * (number_jobs+1));
        
        for ( i = 0; i < number_jobs ; i++ )
            jids[i] = atoi(job_ids[i]);	     
        
        jids[number_jobs] = -1;
    }
    
    exit_codes = (int *) malloc (sizeof(int) * number_jobs);
                                                   
    grc = gw_client_wait_set(jids, &exit_codes, GW_FALSE,timeout);
   	    
    free(exit_codes);
    
    switch(grc)
    {
    	case GW_RC_FAILED_BAD_JOB_ID:
    		rc = DRMAA_ERRNO_INVALID_JOB;    		
    		dispose = 0;
		break;
		
		case GW_RC_FAILED_CONNECTION:
    		rc = DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE;
		   	dispose = 0;
		break;
		
		case GW_RC_FAILED_TIMEOUT:
	        rc = DRMAA_ERRNO_EXIT_TIMEOUT;
		   	dispose = 0;
		break;
		
		case GW_RC_FAILED_PERM:
		   	rc = DRMAA_ERRNO_AUTH_FAILURE;        
		   	dispose = 0;
		break;
		
		case GW_RC_FAILED_JOB_KILLED:
		case GW_RC_FAILED_JOB_FAIL:
    	case GW_RC_SUCCESS:
    		rc = DRMAA_ERRNO_SUCCESS;
    	break;
    		
    	default:
			rc = DRMAA_ERRNO_INTERNAL_ERROR;
		break;
	}
    	
    if (dispose == 1)
    {
        for ( i = 0 ;  i < number_jobs ; i++ )
            gw_client_job_signal(jids[i], GW_CLIENT_SIGNAL_KILL, GW_TRUE);

    	pthread_mutex_lock(&(drmaa_gw_session.mutex));
		    	
        for ( i = 0 ;  i < number_jobs ; i++ )            
            gw_drmaa_rm_session_id (jids[i]);
	            
        pthread_mutex_unlock(&(drmaa_gw_session.mutex));
    }

	if ((error != NULL) && (rc!=DRMAA_ERRNO_SUCCESS))
       	snprintf(error,
       	         error_len,
		 "%s",
       	         drmaa_gw_error_strs[rc]);
    free(jids);
        
    return rc;
}            

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */

void drmaa_gw_ctime(time_t the_time, char *str)
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

static inline void drmaa_clear_attr_values(drmaa_attr_values_t ** rusage)
{
	if (rusage != NULL)
	{
    	*rusage = (drmaa_attr_values_t *) malloc (sizeof(drmaa_attr_values_t));

	    (*rusage)->number_values = 0;
    	(*rusage)->values        = NULL;
    	(*rusage)->current       = 0;	   					
	}
}

/* -------------------------------------------------------------------------- */

static int drmaa_wait_any(char *      job_id_out, 
                          size_t      job_id_out_len, 
                          int *       stat, 
                          signed long timeout, 
						  char *      error, 
               			  size_t      error_len,
               			  int *       jid)
{	
	int *            job_ids;
	int              i;
    gw_return_code_t grc;
    int *            exit_codes;
    int              rc;
    	
   	pthread_mutex_lock(&(drmaa_gw_session.mutex));
    	
	if ( drmaa_gw_session.number_of_jobs == 0 )
	{
    	if ( error != NULL )
	       	snprintf(error,
	       	         error_len,
			 "%s",
	       	         drmaa_gw_error_strs[DRMAA_ERRNO_INVALID_JOB]);
       	
   		pthread_mutex_unlock(&(drmaa_gw_session.mutex));
   		
   	    return DRMAA_ERRNO_INVALID_JOB;
	}
	
	job_ids    = (int *) malloc(sizeof(int)*(drmaa_gw_session.number_of_jobs+1));
	exit_codes = (int *) malloc (sizeof(int) * drmaa_gw_session.number_of_jobs);
	
	for (i=0;i<=drmaa_gw_session.number_of_jobs;i++)
		job_ids[i] = drmaa_gw_session.job_ids[i];

   	pthread_mutex_unlock(&(drmaa_gw_session.mutex));	
	
	grc = gw_client_wait_set(job_ids, &exit_codes,GW_TRUE,timeout);

	if (stat != NULL)
		*stat = 256;

	switch(grc)
	{
		case GW_RC_FAILED_JOB_FAIL:
		    if (stat != NULL)
	    	    *stat = 258;    			

		case GW_RC_FAILED_BAD_JOB_ID:/*The job was in the session, was killed*/
		case GW_RC_FAILED_JOB_KILLED:
   			rc = DRMAA_ERRNO_SUCCESS;
	    			
   		    if ((stat != NULL) && (*stat == 256))
    		    *stat = 257;
	    		    
			*jid   = job_ids[0];
			
			pthread_mutex_lock(&(drmaa_gw_session.mutex));
			
			gw_drmaa_rm_session_id (*jid);

		   	pthread_mutex_unlock(&(drmaa_gw_session.mutex));
		   	
		    if (job_id_out != NULL)
			    snprintf(job_id_out, job_id_out_len, "%i", *jid);		
		break;
		
		case GW_RC_FAILED_CONNECTION:
    		rc = DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE;
		break;
		               
		case GW_RC_FAILED_TIMEOUT:
    		rc = DRMAA_ERRNO_EXIT_TIMEOUT;
		break;
		
		case GW_RC_FAILED_PERM:
    		rc = DRMAA_ERRNO_AUTH_FAILURE;
		break;
           
		case GW_RC_SUCCESS:
   			rc = DRMAA_ERRNO_SUCCESS;
   			
			if ( stat != NULL )
				*stat = exit_codes[0];
			
			*jid = job_ids[0];
			
		    if (job_id_out != NULL)
	    		snprintf(job_id_out, job_id_out_len, "%i", *jid);
		break;   
		
		default:
			rc = DRMAA_ERRNO_INTERNAL_ERROR;
		break;
	}
		
	free(exit_codes);
	free(job_ids);
		
	if ((error != NULL) && (rc != DRMAA_ERRNO_SUCCESS))
       	snprintf(error,
		 error_len,
		 "%s",
		 drmaa_gw_error_strs[rc]);
		
	return rc;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */

static int drmaa_wait_job(const char * job_id,      
						  char *       job_id_out, 
                          size_t       job_id_out_len, 
                          int *        stat, 
                          signed long  timeout, 
						  char *       error, 
               			  size_t       error_len,
               			  int          *jid)
{	
    gw_return_code_t grc;
    int              exit_code;
    int              i = 0;
    int              found = 0;    
    int              rc;
    
    *jid = atoi (job_id);
    
    grc = gw_client_wait(*jid, &exit_code,timeout);

    if (stat != NULL)
    	*stat = 256;    	

	switch(grc)
	{
		case GW_RC_FAILED_BAD_JOB_ID:
   			pthread_mutex_lock(&(drmaa_gw_session.mutex));
    			
		   	while ( (!found) && (i < DRMAA_GW_SESSION_JOBS) )
	        	if ( drmaa_gw_session.job_ids[i] == *jid )
		            found = 1;
		        else
		            i++;    	
			            
	    	if ( found )/*The job was in the session, was killed*/
	    	{
				rc = DRMAA_ERRNO_SUCCESS;
				gw_drmaa_rm_session_id (*jid);

			    if (job_id_out != NULL)
				    snprintf(job_id_out, job_id_out_len, "%i", *jid);
				    
				if (stat != NULL)
    				*stat = 256;
	    	}
   			else
   				rc = DRMAA_ERRNO_INVALID_JOB;
    				
   			pthread_mutex_unlock(&(drmaa_gw_session.mutex));	
		break;
		
		case GW_RC_FAILED_JOB_FAIL:
		    if (stat != NULL)
	    	    *stat = 258;    			
		
		case GW_RC_FAILED_JOB_KILLED:
   			rc = DRMAA_ERRNO_SUCCESS;
	    			
   		    if ((stat != NULL) && (*stat == 256))
    		    *stat = 257;
    		    			
			pthread_mutex_lock(&(drmaa_gw_session.mutex));
			
			gw_drmaa_rm_session_id (*jid);

		   	pthread_mutex_unlock(&(drmaa_gw_session.mutex));
		   	
		    if (job_id_out != NULL)
			    snprintf(job_id_out, job_id_out_len, "%i", *jid);
		break;
		
		case GW_RC_FAILED_CONNECTION:
    		rc = DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE;
		break;
		               
		case GW_RC_FAILED_TIMEOUT:
    		rc = DRMAA_ERRNO_EXIT_TIMEOUT;
		break;
		
		case GW_RC_FAILED_PERM:
    		rc = DRMAA_ERRNO_AUTH_FAILURE;
		break;
           
		case GW_RC_SUCCESS:
   			rc = DRMAA_ERRNO_SUCCESS;
   			
			if ( stat != NULL )
				*stat = exit_code;
			
		    if (job_id_out != NULL)
	    		snprintf(job_id_out, job_id_out_len, "%i", *jid);
		break;   
		
		default:
			rc = DRMAA_ERRNO_INTERNAL_ERROR;
		break;
	}

	if ((error != NULL) && (rc != DRMAA_ERRNO_SUCCESS))
	   	snprintf(error,
	   	         error_len,
			 "%s",
		         drmaa_gw_error_strs[rc]);
	        
	return rc;	
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */

static void drmaa_set_job_rusage(int jid,
                                 drmaa_attr_values_t ** rusage)
{
    gw_msg_job_t     status;
    gw_return_code_t grc;
    char             str[26];
	int              i;
    div_t            r;
    int              hours;
    
	if (rusage == NULL)
		return;
		
    grc = gw_client_job_status(jid, &status);
    
    if ( grc != GW_RC_SUCCESS )
    {
		drmaa_clear_attr_values(rusage);
    }
    else
    {
    	*rusage = (drmaa_attr_values_t *) malloc (sizeof(drmaa_attr_values_t));

	    (*rusage)->number_values = 4;
    	(*rusage)->values        = (char **) malloc(4 * sizeof(char *));
    	(*rusage)->current       = 0;

	    for (i = 0 ; i<4 ; i++)
			(*rusage)->values[i] = (char *)malloc(DRMAA_ATTR_BUFFER*sizeof(char));

		drmaa_gw_ctime(status.start_time, str);
	    snprintf((*rusage)->values[0], DRMAA_ATTR_BUFFER, "start_time=%s",str);

		drmaa_gw_ctime(status.exit_time, str);
	    snprintf((*rusage)->values[1], DRMAA_ATTR_BUFFER, "exit_time=%s",str);
	    
		r = div(status.cpu_time, 3600);
		hours = r.quot;		
		r = div(r.rem, 60);
	    snprintf((*rusage)->values[2], DRMAA_ATTR_BUFFER, "cpu_time=%02i:%02i:%02i",hours,r.quot,r.rem);

		r = div(status.xfr_time, 3600);
		hours = r.quot;		
		r = div(r.rem, 60);			    
	    snprintf((*rusage)->values[3], DRMAA_ATTR_BUFFER, "xfr_time=%02i:%02i:%02i",hours,r.quot,r.rem);
    }
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */

int drmaa_wait(const char *           job_id, 
               char *                 job_id_out, 
               size_t                 job_id_out_len, 
               int *                  stat, 
               signed long            timeout, 
               drmaa_attr_values_t ** rusage, 
               char *                 error, 
               size_t                 error_len)
{
    int rc;
	int any_ids;
	int jid;
	
    pthread_mutex_lock(&(drmaa_gw_session.mutex));
    
	if ( drmaa_gw_session.session_id == -1 )
    {
    	if (error != NULL)
	       	snprintf(error,
	       	         error_len,
			 "%s",
	       	         drmaa_gw_error_strs[DRMAA_ERRNO_NO_ACTIVE_SESSION]);
    
    	pthread_mutex_unlock(&(drmaa_gw_session.mutex));
    
        return DRMAA_ERRNO_NO_ACTIVE_SESSION;
    }
    
    pthread_mutex_unlock(&(drmaa_gw_session.mutex));
    
    if (job_id == NULL)
    {
    	if ( error != NULL )
	       	snprintf(error,
	       	         error_len,
			 "%s",
	       	         drmaa_gw_error_strs[DRMAA_ERRNO_INVALID_ARGUMENT]);
       	         
        return DRMAA_ERRNO_INVALID_ARGUMENT;
    }   

	any_ids = strcmp(job_id, DRMAA_JOB_IDS_SESSION_ANY) == 0;
	
    if (any_ids)
		rc = drmaa_wait_any(job_id_out, 
                            job_id_out_len, 
                            stat, 
                            timeout, 
						    error, 
               			    error_len,
               			    &jid);
    else
	   rc = drmaa_wait_job(job_id,
						   job_id_out, 
                           job_id_out_len, 
                           stat, 
                           timeout, 
						   error, 
               			   error_len,
               			   &jid);
    
    if (rc != DRMAA_ERRNO_SUCCESS)
    {
		drmaa_clear_attr_values(rusage);
		return rc;    	
    }

	drmaa_set_job_rusage(jid,rusage);
		    
	gw_client_job_signal(jid, GW_CLIENT_SIGNAL_KILL, GW_TRUE);

	pthread_mutex_lock(&(drmaa_gw_session.mutex));
	
    gw_drmaa_rm_session_id (jid);
    
	pthread_mutex_unlock(&(drmaa_gw_session.mutex));
    
    return DRMAA_ERRNO_SUCCESS;
}

/* ************************************************************************** *
 *    SECTION 8.2 Auxiliary functions for interpreting Wait status code       *
 * ************************************************************************** */  

int drmaa_wifexited(int *  exited, 
                    int    stat, 
                    char * error_diagnosis, 
                    size_t error_diag_len)
{
	pthread_mutex_lock(&(drmaa_gw_session.mutex));
	
	if ( drmaa_gw_session.session_id == -1 )
    {
    	if (error_diagnosis != NULL)
	       	snprintf(error_diagnosis,
	       	         error_diag_len,
			 "%s",
	       	         drmaa_gw_error_strs[DRMAA_ERRNO_NO_ACTIVE_SESSION]);

		pthread_mutex_unlock(&(drmaa_gw_session.mutex));
		
        return DRMAA_ERRNO_NO_ACTIVE_SESSION;
    }
    
	pthread_mutex_unlock(&(drmaa_gw_session.mutex));
    
    if ( exited == NULL )
    {
    	if ( error_diagnosis != NULL )
	       	snprintf(error_diagnosis,
	       	         error_diag_len,
			 "%s",
	       	         drmaa_gw_error_strs[DRMAA_ERRNO_INVALID_ARGUMENT]);
       	         
        return DRMAA_ERRNO_INVALID_ARGUMENT;
    }   
        
    *exited = stat < 256;
	
    return DRMAA_ERRNO_SUCCESS;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */

int drmaa_wexitstatus(int *  exit_status, 
                      int    stat, 
                      char * error_diagnosis, 
                      size_t error_diag_len)
{
	pthread_mutex_lock(&(drmaa_gw_session.mutex));
	
	if ( drmaa_gw_session.session_id == -1 )
    {
    	if (error_diagnosis != NULL)
	       	snprintf(error_diagnosis,
	       	         error_diag_len,
			 "%s",
	       	         drmaa_gw_error_strs[DRMAA_ERRNO_NO_ACTIVE_SESSION]);
	       	         
    	pthread_mutex_unlock(&(drmaa_gw_session.mutex));   	         
        return DRMAA_ERRNO_NO_ACTIVE_SESSION;
    }
	
	pthread_mutex_unlock(&(drmaa_gw_session.mutex));
	
    if ((exit_status == NULL) || (stat > 255))
    {
       	snprintf(error_diagnosis,
       	         error_diag_len,
		 "%s",
       	         drmaa_gw_error_strs[DRMAA_ERRNO_INVALID_ARGUMENT]);
       	         
        return DRMAA_ERRNO_INVALID_ARGUMENT;
    }   
	
	*exit_status = stat;
	
    return DRMAA_ERRNO_SUCCESS;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */

int drmaa_wifsignaled(int *  signaled, 
                      int    stat, 
                      char * error_diagnosis, 
                      size_t error_diag_len)
{
	pthread_mutex_lock(&(drmaa_gw_session.mutex));
	
	if ( drmaa_gw_session.session_id == -1 )
    {
    	if ( error_diagnosis != NULL )
	       	snprintf(error_diagnosis,
	       	         error_diag_len,
			 "%s",
	       	         drmaa_gw_error_strs[DRMAA_ERRNO_NO_ACTIVE_SESSION]);

		pthread_mutex_unlock(&(drmaa_gw_session.mutex));       	         
        return DRMAA_ERRNO_NO_ACTIVE_SESSION;
    }

	pthread_mutex_unlock(&(drmaa_gw_session.mutex));
	
    if ( signaled == NULL )
    {
    	if ( error_diagnosis != NULL )
	       	snprintf(error_diagnosis,
	       	         error_diag_len,
			 "%s",
	       	         drmaa_gw_error_strs[DRMAA_ERRNO_INVALID_ARGUMENT]);
       	         
        return DRMAA_ERRNO_INVALID_ARGUMENT;
    }   
    
    *signaled = (stat > 128) && (stat < 160);

    return DRMAA_ERRNO_SUCCESS;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */

int drmaa_wtermsig(char * signal, 
                   size_t signal_len, 
                   int    stat, 
                   char * error_diagnosis, 
                   size_t error_diag_len)
{
	pthread_mutex_lock(&(drmaa_gw_session.mutex));
	
	if ( drmaa_gw_session.session_id == -1 )
    {
    	if (error_diagnosis != NULL)
	       	snprintf(error_diagnosis,
	       	         error_diag_len,
			 "%s",
	       	         drmaa_gw_error_strs[DRMAA_ERRNO_NO_ACTIVE_SESSION]);
	       	         
		pthread_mutex_unlock(&(drmaa_gw_session.mutex));       	         
        return DRMAA_ERRNO_NO_ACTIVE_SESSION;
    }
    
   	pthread_mutex_unlock(&(drmaa_gw_session.mutex));
   	
    if ( signal == NULL )
    {
    	if ( error_diagnosis != NULL )
	       	snprintf(error_diagnosis,
	       	         error_diag_len,
			 "%s",
	       	         drmaa_gw_error_strs[DRMAA_ERRNO_INVALID_ARGUMENT]);
       	         
        return DRMAA_ERRNO_INVALID_ARGUMENT;
    }       
	 
	if ( (stat - 128) < 17 )  
		snprintf(signal,signal_len,"%s",drmaa_gw_signal_strs[stat-128]);
	else
		snprintf(signal,signal_len,"%s",drmaa_gw_signal_strs[17]);
		
    return DRMAA_ERRNO_SUCCESS;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */

int drmaa_wcoredump(int *  core_dumped, 
                    int    stat, 
                    char * error_diagnosis, 
                    size_t error_diag_len)
{
	pthread_mutex_lock(&(drmaa_gw_session.mutex));
	
	if ( drmaa_gw_session.session_id == -1 )
    {
    	if (error_diagnosis != NULL)
	       	snprintf(error_diagnosis,
	       	         error_diag_len,
			 "%s",
	       	         drmaa_gw_error_strs[DRMAA_ERRNO_NO_ACTIVE_SESSION]);
	       	         
		pthread_mutex_unlock(&(drmaa_gw_session.mutex));       	         
        return DRMAA_ERRNO_NO_ACTIVE_SESSION;
    }
	
	pthread_mutex_unlock(&(drmaa_gw_session.mutex));
	
    if ( core_dumped == NULL )
    {
    	if ( error_diagnosis != NULL )
	       	snprintf(error_diagnosis,
	       	         error_diag_len,
			 "%s",
	       	         drmaa_gw_error_strs[DRMAA_ERRNO_INVALID_ARGUMENT]);
       	         
        return DRMAA_ERRNO_INVALID_ARGUMENT;
    }       
	
	/* Could check return codes for signals that generate a core but that will
	 * not be true for all cases. So by default no core is generated 
	 * from the user point of view. In any case the user will not be 
	 * able to get the core file
	 */
	*core_dumped = 0;
	
    return DRMAA_ERRNO_SUCCESS;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */

int drmaa_wifaborted(int *  aborted, 
                     int    stat, 
                     char * error_diagnosis, 
                     size_t error_diag_len)
{
	pthread_mutex_lock(&(drmaa_gw_session.mutex));
		
	if ( drmaa_gw_session.session_id == -1 )
    {
    	if (error_diagnosis != NULL)
	       	snprintf(error_diagnosis,
	       	         error_diag_len,
			 "%s",
	       	         drmaa_gw_error_strs[DRMAA_ERRNO_NO_ACTIVE_SESSION]);
	       	         
		pthread_mutex_unlock(&(drmaa_gw_session.mutex));       	         
        return DRMAA_ERRNO_NO_ACTIVE_SESSION;
    }
    
	pthread_mutex_unlock(&(drmaa_gw_session.mutex));
	
    if ( aborted == NULL )
    {
    	if ( error_diagnosis != NULL )
	       	snprintf(error_diagnosis,
	       	         error_diag_len,
			 "%s",
	       	         drmaa_gw_error_strs[DRMAA_ERRNO_INVALID_ARGUMENT]);
       	         
        return DRMAA_ERRNO_INVALID_ARGUMENT;
    }       
	
	*aborted = 0;
	
    return DRMAA_ERRNO_SUCCESS;
}

/* ************************************************************************** *
 *    SECTION 9 Auxiliary functions 					                      *
 * ************************************************************************** */  

const char *drmaa_strerror(int drmaa_errno)
{
	if ( drmaa_errno < 25 )
		return drmaa_gw_error_strs[drmaa_errno];
	else
		return drmaa_gw_error_strs[25];
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */

int drmaa_get_contact(char * contact, 
                      size_t contact_len, 
                      char * error_diagnosis, 
                      size_t error_diag_len)
{
	int not_init;
	
    if ( contact == NULL )
    {
    	if ( error_diagnosis != NULL )
	       	snprintf(error_diagnosis,
	       	         error_diag_len,
			 "%s",
	       	         drmaa_gw_error_strs[DRMAA_ERRNO_INVALID_ARGUMENT]);
       	         
        return DRMAA_ERRNO_INVALID_ARGUMENT;
    }
	
	pthread_mutex_lock(&(drmaa_gw_session.mutex));
	
	not_init = drmaa_gw_session.session_id == -1;
	
	pthread_mutex_unlock(&(drmaa_gw_session.mutex));	
	
	if (not_init)
    {
		snprintf(contact,contact_len,"%s","localhost");
	    return DRMAA_ERRNO_SUCCESS;
    }
		
	snprintf(contact,contact_len,"%s","localhost");
	
    return DRMAA_ERRNO_SUCCESS;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */

int drmaa_version(unsigned int * major, 
                  unsigned int * minor, 
                  char *         error_diagnosis, 
                  size_t         error_diag_len)
{
    if ((major == NULL)||(minor == NULL))
    {
    	if ( error_diagnosis != NULL )
	       	snprintf(error_diagnosis,
	       	         error_diag_len,
			 "%s",
	       	         drmaa_gw_error_strs[DRMAA_ERRNO_INVALID_ARGUMENT]);
       	         
        return DRMAA_ERRNO_INVALID_ARGUMENT;
    }

    *major = 1;
    *minor = 0;

    return DRMAA_ERRNO_SUCCESS;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
 
int drmaa_get_DRM_system(char * drm_system, 
                         size_t drm_system_len, 
                         char * error_diagnosis, 
                         size_t error_diag_len)
{
	int not_init;
	
    if (drm_system == NULL)
    {
    	if ( error_diagnosis != NULL )
	       	snprintf(error_diagnosis,
	       	         error_diag_len,
			 "%s",
	       	         drmaa_gw_error_strs[DRMAA_ERRNO_INVALID_ARGUMENT]);
       	         
        return DRMAA_ERRNO_INVALID_ARGUMENT;
    }

	pthread_mutex_lock(&(drmaa_gw_session.mutex));
	
	not_init = drmaa_gw_session.session_id == -1;
	
	pthread_mutex_unlock(&(drmaa_gw_session.mutex));	
	
	if ( not_init )
    {
		snprintf(drm_system, drm_system_len,"%s","GridWay");
       	         
        return DRMAA_ERRNO_SUCCESS;
    }
    
    snprintf(drm_system, drm_system_len,"%s","GridWay");
 
    return DRMAA_ERRNO_SUCCESS;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */

int drmaa_get_DRMAA_implementation(char *drmaa_impl, size_t drmaa_impl_len, char *error_diagnosis, size_t error_diag_len)
{
	int not_init;
	
    if (drmaa_impl == NULL)
    {
    	if ( error_diagnosis != NULL )
	       	snprintf(error_diagnosis,
	       	         error_diag_len,
			 "%s",
	       	         drmaa_gw_error_strs[DRMAA_ERRNO_INVALID_ARGUMENT]);
       	         
        return DRMAA_ERRNO_INVALID_ARGUMENT;
    }

	pthread_mutex_lock(&(drmaa_gw_session.mutex));
	
	not_init = drmaa_gw_session.session_id == -1;
	
	pthread_mutex_unlock(&(drmaa_gw_session.mutex));	
    	
	if (not_init)
    {
		snprintf(drmaa_impl, drmaa_impl_len,"DRMAA for %s",GW_VERSION);	
       	         
        return DRMAA_ERRNO_SUCCESS;
    }
	
    snprintf(drmaa_impl, drmaa_impl_len,"DRMAA for %s",GW_VERSION);

    return DRMAA_ERRNO_SUCCESS;
}

/* **************************************************************************** *
 *   NOT PART OF DRMAA STANDARD                                                 *
 * **************************************************************************** */
 
const char *drmaa_gw_strstatus(int status)
{
    switch (status)
    {				
	case DRMAA_PS_QUEUED_ACTIVE:
	    return("QUEUED_ACTIVE");
	    break;
				
	case DRMAA_PS_SYSTEM_ON_HOLD:
	    return("SYSTEM_ON_HOLD");
	    break;
					
	case DRMAA_PS_USER_ON_HOLD:
	    return("USER_ON_HOLD");
	    break;
					
	case DRMAA_PS_USER_SYSTEM_ON_HOLD:
	    return("USER_SYSTEM_ON_HOLD");
	    break;
					
	case DRMAA_PS_RUNNING:
	    return("RUNNING");
	    break;
				
	case DRMAA_PS_SYSTEM_SUSPENDED:
	    return("SYSTEM_SUSPENDED");
	    break;
					
	case DRMAA_PS_USER_SUSPENDED:
	    return("USER_SUSPENDED");
	    break;
				
	case DRMAA_PS_USER_SYSTEM_SUSPENDED:
	    return("USER_SYSTEM_SUSPENDED");
	    break;
				
	case DRMAA_PS_DONE:
	     return("DONE");
	     break;
					
	case DRMAA_PS_FAILED:
	     return("FAILED");
	     break;
	     
	default:
	    return("UNDETERMINED");
	    break;			
    }
	
}

/* ************************************************************************** *
 * ************************************************************************** *
 *             MODULE FUNCTION IMPLEMENTATION (PRIVATE INTERFACE)             *
 * ************************************************************************** *
 * ************************************************************************** */

int gw_drmaa_rm_session_id (int jid)
{
    int i     = 0;
    int found = 0;
    int j;
        
    while ( (!found) && (i < DRMAA_GW_SESSION_JOBS) )
        if ( drmaa_gw_session.job_ids[i] == jid )
            found = 1;
        else
            i++;
    
    if (!found)
        return -1;
    
    drmaa_gw_session.number_of_jobs--;
    
    for ( j = i; j < (drmaa_gw_session.number_of_jobs); j++)
        drmaa_gw_session.job_ids[j] = drmaa_gw_session.job_ids[j+1];
        
    drmaa_gw_session.job_ids[drmaa_gw_session.number_of_jobs] = -1;
    
    return 0;
}
