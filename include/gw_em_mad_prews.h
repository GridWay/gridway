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

#ifndef  _GW_EM_MAD_PREWS_H
#define  _GW_EM_MAD_PREWS_H

#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>

#include "globus_gram_client.h"
#include "globus_gss_assist.h"
#include "globus_gsi_credential.h"

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

struct job_pool_s
{   
	int      max_jobs;
    char **  job_contact;
    int  *   cancel_state;
};

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

void init_job_pool(int max_job);
void add_job( int jid, const char *job_contact );
void del_job( int jid );
char *get_job_contact( int jid );
int  get_jid( char *job_contact );
int  get_max_jobs( );

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

struct mad_s
{    
    char *callback_contact;
    int  initialized;
};

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

int gw_em_mad_init( int max_job, char *info );
int gw_em_mad_submit( int jid, char *rm_contact, char *rsl_string, char *info );
int gw_em_mad_recover( int jid, char *job_contact, char *info );
int gw_em_mad_cancel( int jid, char *info );
int gw_em_mad_poll( int jid, char *info );
int gw_em_mad_finalize( char *info );
int gw_em_mad_check_credentials();
int gw_em_mad_refresh(gss_cred_id_t creds, char *info);

char *get_job_state_name(globus_gram_protocol_job_state_t job_state);

void gw_em_mad_state_callback( void *arg, char *job_contact, int job_state,
            int error_code );
            
void gw_em_mad_submit_callback (void *arg, globus_gram_protocol_error_t
            failure_code, const char * job_contact, 
            globus_gram_protocol_job_state_t job_state, 
            globus_gram_protocol_error_t error_code);

void gw_em_mad_recover_callback (void *arg, globus_gram_protocol_error_t
            failure_code, const char * job_contact, 
            globus_gram_protocol_job_state_t job_state, 
            globus_gram_protocol_error_t error_code);

void gw_em_mad_cancel_callback (void *arg, globus_gram_protocol_error_t
            failure_code, const char * job_contact, 
            globus_gram_protocol_job_state_t job_state, 
            globus_gram_protocol_error_t error_code);

void gw_em_mad_poll_callback (void *arg, globus_gram_protocol_error_t
            failure_code, const char * job_contact, 
            globus_gram_protocol_job_state_t job_state, 
            globus_gram_protocol_error_t error_code);            

void gw_em_mad_refresh_callback(void *arg, globus_gram_protocol_error_t
            failure_code, const char * job_contact, 
            globus_gram_protocol_job_state_t job_state, 
            globus_gram_protocol_error_t error_code);
 
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
            
#endif
