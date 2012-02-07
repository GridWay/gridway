/* -------------------------------------------------------------------------- */
/* Copyright 2002-2012, GridWay Project Leads (GridWay.org)                   */
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

#include "gw_em_mad_gram2.h"

/*----------------------------------------------------------------------------*/

struct mad_s  mad;
extern struct job_pool_s  job_pool;

/*----------------------------------------------------------------------------*/

int gw_em_mad_init( int max_job, char *info )
{
    int rc;

    rc = globus_module_activate(GLOBUS_GRAM_CLIENT_MODULE);

    if ( rc != GLOBUS_SUCCESS)
    {
        sprintf(info, "GRAM client module activation failed: %s (error %d)",
                globus_gram_client_error_string(rc), rc);
        return 1;
    }

    rc = globus_gram_client_callback_allow(gw_em_mad_state_callback, NULL,
                 &(mad.callback_contact));
                 
    if ( rc != GLOBUS_SUCCESS)
    {
        sprintf(info, "GRAM client callback allow failed: %s (error %d)",
                globus_gram_client_error_string(rc), rc);
        return 1;
    }

    init_job_pool(max_job);

    mad.initialized = 1;
    
    printf("INIT - SUCCESS -\n");

    return 0;
}

/*----------------------------------------------------------------------------*/

int gw_em_mad_submit( int jid, char *rm_contact, char *rsl_file, char *info )
{
    int  rc;
    int  *i;
    char rsl_string[4096];
    int  fd;
    int  j;
    
    if (!mad.initialized)
    {
        strcpy(info, "MAD not initialized");
        return 1;
    }

    fd = open(rsl_file, O_RDONLY);

    if (fd == -1)
    {
        strcpy(info, "Error opening RSL file");
        return 1;
    }

    j = read(fd, (void *) rsl_string, sizeof(char)*4095);

    if ((j ==-1)||(j==0))
    {
        strcpy(info, "Error reading RSL file");
        close(fd);
        
        return 1;
    }
    else
        rsl_string[j]='\0';

    i  = (int *) malloc( sizeof(int));
    *i = jid;    

    rc = globus_gram_client_register_job_request(rm_contact, rsl_string, 
            GLOBUS_GRAM_PROTOCOL_JOB_STATE_ALL, mad.callback_contact, 
            GLOBUS_GRAM_CLIENT_NO_ATTR, gw_em_mad_submit_callback, (void *) i);
                        
    if (rc != GLOBUS_SUCCESS) 
    {
        sprintf(info, "GRAM client job request failed: %s (error %d)",
                globus_gram_client_error_string(rc), rc);
        close(fd);
        
        return 1;
    }
    
    close(fd);
    return 0;
}

/*----------------------------------------------------------------------------*/

void gw_em_mad_submit_callback(void *arg, globus_gram_protocol_error_t
            failure_code, const char * job_contact, 
            globus_gram_protocol_job_state_t job_state, 
            globus_gram_protocol_error_t error_code)
{
    int *jid;
    
    jid = (int *) arg;

    if ( failure_code == GLOBUS_SUCCESS)
    {
        add_job(*jid, job_contact);
        printf("SUBMIT %d SUCCESS %s\n", *jid, job_contact);
    }
    else
    {
         printf("SUBMIT %d FAILURE %s (%i)\n", *jid,
             globus_gram_client_error_string(failure_code), error_code);    
    }
    
    free(jid);
}

/*----------------------------------------------------------------------------*/

int gw_em_mad_recover( int jid, char *job_contact, char *info )
{
    int rc;
    int *i;
    
    if (!mad.initialized)
    {
        strcpy(info, "MAD not initialized");
        return 1;
    }

    i  = (int *) malloc( sizeof(int));
    *i = jid;

    rc = globus_gram_client_register_job_callback_registration(job_contact,
            GLOBUS_GRAM_PROTOCOL_JOB_STATE_ALL, mad.callback_contact,
            GLOBUS_GRAM_CLIENT_NO_ATTR, gw_em_mad_recover_callback, i);

    if (rc != GLOBUS_SUCCESS) 
    {
        sprintf(info, "GRAM client job callback register failed: %s (error %d)",
                globus_gram_client_error_string(rc), rc);
        return 1;
    }
        
    add_job(jid, job_contact);

    return 0;
}

/*----------------------------------------------------------------------------*/

void gw_em_mad_recover_callback(void *arg, globus_gram_protocol_error_t
            failure_code, const char * job_contact, 
            globus_gram_protocol_job_state_t job_state, 
            globus_gram_protocol_error_t error_code)
{
    int *jid;
    
    jid = (int *) arg;

    if ( failure_code == GLOBUS_SUCCESS)
    {
        printf("RECOVER %d SUCCESS %s\n", *jid, get_job_state_name(job_state));
    }
    else
    {
        printf("RECOVER %d FAILURE %s (%i)\n", *jid,
                globus_gram_client_error_string(failure_code), error_code);    
        del_job(*jid);
    }
    
    free(jid);
}
/*----------------------------------------------------------------------------*/

int gw_em_mad_cancel( int jid, char *info )
{
    int rc;
    char *job_contact;
    int  *i;
    
    if (!mad.initialized)
    {
        strcpy(info, "MAD not initialized");
        return 1;
    }

    job_contact = get_job_contact(jid);

    if (job_contact == NULL)
    {
        strcpy(info, "Job does not exist");
        return 1;
    }

    i  = (int *) malloc( sizeof(int) );
    *i = jid;

    rc = globus_gram_client_register_job_cancel(job_contact, 
            GLOBUS_GRAM_CLIENT_NO_ATTR, gw_em_mad_cancel_callback, (void *) i);

    if (rc != GLOBUS_SUCCESS)
        job_pool.cancel_state[jid] = 1;
    
    strcpy(info, "-");

    return 0;
}

/*----------------------------------------------------------------------------*/

void gw_em_mad_cancel_callback(void *arg, globus_gram_protocol_error_t
            failure_code, const char * job_contact, 
            globus_gram_protocol_job_state_t job_state, 
            globus_gram_protocol_error_t error_code)
{
    int *jid;
    
    jid = (int *) arg;

    if ( failure_code == GLOBUS_SUCCESS)
        printf("CANCEL %d SUCCESS -\n", *jid);
    else
        printf("CANCEL %d FAILURE %s (%i)\n", *jid,
             globus_gram_client_error_string(failure_code), error_code);
    
    free(jid);
}

/*----------------------------------------------------------------------------*/

int gw_em_mad_poll( int jid, char *info )
{
    char *job_contact;
    int rc;
    int  *i;

    if (!mad.initialized)
    {
        strcpy(info, "MAD not initialized");
        return 1;
    }

    job_contact = get_job_contact(jid);

    if (job_contact == NULL)
    {
        strcpy(info, "Job does not exist");
        return 1;
    }
    
    i  = (int *) malloc( sizeof(int));
    *i = jid;
    
    rc = globus_gram_client_register_job_status(job_contact,
            GLOBUS_GRAM_CLIENT_NO_ATTR, gw_em_mad_poll_callback, (void *) i);
            
    if (rc != GLOBUS_SUCCESS)
    {
        sprintf(info, "GRAM client job status failed: %s (error %d)",
                globus_gram_client_error_string(rc), rc);
        return 1;
    }

    return 0;
}

/*----------------------------------------------------------------------------*/

void gw_em_mad_poll_callback(void *arg, globus_gram_protocol_error_t
            failure_code, const char * job_contact, 
            globus_gram_protocol_job_state_t job_state, 
            globus_gram_protocol_error_t error_code)
{
    int *jid;
    
    jid = (int *) arg;

    if ( failure_code == GLOBUS_SUCCESS)
    {
        printf("POLL %d SUCCESS %s\n", *jid, get_job_state_name(job_state));
    }
    else
        printf("POLL %d FAILURE %s (%i)\n", *jid,
             globus_gram_client_error_string(failure_code), error_code);
    
    free(jid);
}

/*----------------------------------------------------------------------------*/

char *get_job_state_name(globus_gram_protocol_job_state_t job_state)
{
        switch (job_state)
        {
            case GLOBUS_GRAM_PROTOCOL_JOB_STATE_PENDING:
                return "PENDING";
            break;

            case GLOBUS_GRAM_PROTOCOL_JOB_STATE_SUSPENDED:
                return "SUSPENDED";
            break;

            case GLOBUS_GRAM_PROTOCOL_JOB_STATE_ACTIVE:
                return "ACTIVE";
            break;

            case GLOBUS_GRAM_PROTOCOL_JOB_STATE_FAILED:
                return "FAILED";
            break;

            case GLOBUS_GRAM_PROTOCOL_JOB_STATE_DONE:
                return "DONE";
            break;
            
            default:
                return "UNKNOWN";
            break;
        }
}

/*----------------------------------------------------------------------------*/


int gw_em_mad_finalize( char *info )
{

    if (!mad.initialized)
    {
        strcpy(info, "MAD not initialized");
        return 1;
    }

    printf("FINALIZE - SUCCESS -\n");
    
    return 0;
}

/*----------------------------------------------------------------------------*/

void gw_em_mad_state_callback(void *arg, char *job_contact, int job_state,
            int error_code )
{                              
    int jid = -1;
    int done = 0;

    jid = get_jid(job_contact);

    if (jid == -1)
    {
        return;
    }
 
    switch (job_state)
    {
        case GLOBUS_GRAM_PROTOCOL_JOB_STATE_PENDING:    
            printf("CALLBACK %d SUCCESS PENDING\n", jid);
            if (job_pool.cancel_state[jid] == 1)
            {
                globus_gram_client_job_cancel(job_contact);            
                job_pool.cancel_state[jid] = 0;
            }
        break;
        
        case GLOBUS_GRAM_PROTOCOL_JOB_STATE_SUSPENDED:
            printf("CALLBACK %d SUCCESS SUSPENDED\n", jid);
            if (job_pool.cancel_state[jid] == 1)
            {
                globus_gram_client_job_cancel(job_contact);            
                job_pool.cancel_state[jid] = 0;
            }
        break;
        
        case GLOBUS_GRAM_PROTOCOL_JOB_STATE_ACTIVE:
            printf("CALLBACK %d SUCCESS ACTIVE\n", jid);
            if (job_pool.cancel_state[jid] == 1)
            {
                globus_gram_client_job_cancel(job_contact);            
                job_pool.cancel_state[jid] = 0;
            }

        break;
        
        case GLOBUS_GRAM_PROTOCOL_JOB_STATE_FAILED:
            if (error_code == GLOBUS_GRAM_PROTOCOL_ERROR_USER_CANCELLED )
                printf("CALLBACK %d SUCCESS FAILED\n", jid);
            else
                printf("CALLBACK %d FAILURE %s (error %d)\n", jid,
                        globus_gram_client_error_string(error_code),
                        error_code);
            done = 1;            
            job_pool.cancel_state[jid] = 0;
                  
        break;
        
        case GLOBUS_GRAM_PROTOCOL_JOB_STATE_DONE:
            printf("CALLBACK %d SUCCESS DONE\n", jid);
            done = 1;
            job_pool.cancel_state[jid] = 0;            
        break;
    }

    if (done && (jid != -1))
        del_job(jid);
}

/*----------------------------------------------------------------------------*/

int gw_em_mad_check_credentials(char *info)
{
    int rc;
    gss_cred_id_t gss_cred = GSS_C_NO_CREDENTIAL;
    OM_uint32 major_status; 
    OM_uint32 minor_status;
    OM_uint32 lifetime;
    time_t goodtill;
    static time_t last_goodtill = 0;
    char goodtill_str[26];
    
    info[0] = '\0';
    
    /* (Re)adquire credentials */
    major_status = globus_gss_assist_acquire_cred(&minor_status,
            GSS_C_INITIATE,
            &gss_cred);

    if (major_status != GSS_S_COMPLETE) 
    {
        sprintf(info, "Error loading credentials");
        return 1;
    } 

    gss_inquire_cred(&minor_status, gss_cred, NULL, &lifetime, NULL, NULL);
    goodtill = time(NULL) + lifetime;

#ifdef GWSOLARIS
    ctime_r(&(goodtill), goodtill_str, sizeof(char)*26);
#else
    ctime_r(&(goodtill), goodtill_str);
#endif

    goodtill_str[24]='\0';
    
    printf("TIMER - SUCCESS Credential is valid until %s\n", goodtill_str);
    
    if (last_goodtill == 0)
    {
        last_goodtill = goodtill;
    }
    else if (goodtill > last_goodtill)
    {
        rc = globus_gram_client_set_credentials(gss_cred);

        if (rc != 0)
        {
            sprintf(info, "Error setting credentials");
            return 1;
        }

        printf("TIMER - SUCCESS Refreshing credentials until %s\n",
                goodtill_str);
        
        last_goodtill = goodtill;
        rc = gw_em_mad_refresh(gss_cred, info);

        if (rc != 0)
        {
            return 1;
        }
    }

    return 0;
}   

/*----------------------------------------------------------------------------*/

int gw_em_mad_refresh(gss_cred_id_t creds, char *info)
{
    int jid, max_jobs;
    int rc, status = 0;
    int *i;
    char *job_contact;

    max_jobs = get_max_jobs();
    
    for (jid = 0; jid<max_jobs; jid++)
    {
        job_contact = get_job_contact(jid);
        
        if (job_contact != NULL)
        {
            i  = (int *) malloc( sizeof(int));
            *i = jid;
            
            rc = globus_gram_client_register_job_refresh_credentials(
                    job_contact, creds, GLOBUS_GRAM_CLIENT_NO_ATTR,
                    gw_em_mad_refresh_callback, i);

            if (rc != GLOBUS_SUCCESS) 
            {
                sprintf(info,
                        "GRAM client job refresh credentials failed: %s (%d)",
                        globus_gram_client_error_string(rc), rc);
                status = -1;
            }
        }
    }
    
    return status;
}

/*----------------------------------------------------------------------------*/

void gw_em_mad_refresh_callback (void *arg,
        globus_gram_protocol_error_t failure_code, const char * job_contact,
        globus_gram_protocol_job_state_t job_state, 
        globus_gram_protocol_error_t error_code)
{
    int *jid;

    jid = (int *)arg;

    if (failure_code == GLOBUS_SUCCESS)
    {
        printf("REFRESH %d SUCCESS -\n", *jid);
    }
    else
    {
        printf("REFRESH %d FAILURE %s (%d:%d)\n", *jid,
                globus_gram_client_error_string(failure_code), failure_code,
                error_code);
    }
    
    free(jid);
}
