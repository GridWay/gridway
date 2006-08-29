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

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "gw_host_update_syntax.h"
#include "gw_host_rank_syntax.h"
#include "gw_host_reqs_syntax.h"
#include "gw_host.h"
#include "gw_log.h"

#define YY_DECL int host_attr_lex (YYSTYPE *lvalp, YYLTYPE *llocp)

#define YY_USER_ACTION  llocp->first_column = llocp->last_column; llocp->last_column += yyleng;

int  host_update_parse (gw_host_t *host, int *pos);
int  host_rank_parse (gw_host_t *host, int queue, int *result, int *pos);
int  host_reqs_parse (gw_host_t *host, int queue, gw_boolean_t *result, int *pos);
/*void host_update_error (gw_host_t *host, const char *str);*/

pthread_mutex_t host_attr_mutex = PTHREAD_MUTEX_INITIALIZER;

%}

%option nounput
%option prefix="host_attr_"

%%

HOSTNAME                { lvalp->val_int = HOSTNAME;            return VARIABLE;}
ARCH                    { lvalp->val_int = ARCH;                return VARIABLE;}
OS_NAME                 { lvalp->val_int = OS_NAME;             return VARIABLE;}
OS_VERSION              { lvalp->val_int = OS_VERSION;          return VARIABLE;}
CPU_MODEL               { lvalp->val_int = CPU_MODEL;           return VARIABLE;}
CPU_MHZ                 { lvalp->val_int = CPU_MHZ;             return VARIABLE;}
CPU_FREE                { lvalp->val_int = CPU_FREE;            return VARIABLE;}
CPU_SMP                 { lvalp->val_int = CPU_SMP;             return VARIABLE;}
NODECOUNT               { lvalp->val_int = NODECOUNT;           return VARIABLE;}
SIZE_MEM_MB             { lvalp->val_int = SIZE_MEM_MB;         return VARIABLE;}
FREE_MEM_MB             { lvalp->val_int = FREE_MEM_MB;         return VARIABLE;}
SIZE_DISK_MB            { lvalp->val_int = SIZE_DISK_MB;        return VARIABLE;}
FREE_DISK_MB            { lvalp->val_int = FREE_DISK_MB;        return VARIABLE;}
FORK_NAME               { lvalp->val_int = FORK_NAME;           return VARIABLE;}
LRMS_NAME               { lvalp->val_int = LRMS_NAME;           return VARIABLE;}
LRMS_TYPE               { lvalp->val_int = LRMS_TYPE;           return VARIABLE;}
QUEUE_NAME              { lvalp->val_int = QUEUE_NAME;          return VARIABLE;}
QUEUE_NODECOUNT         { lvalp->val_int = QUEUE_NODECOUNT;     return VARIABLE;}
QUEUE_FREENODECOUNT     { lvalp->val_int = QUEUE_FREENODECOUNT; return VARIABLE;}
QUEUE_MAXTIME           { lvalp->val_int = QUEUE_MAXTIME;       return VARIABLE;}
QUEUE_MAXCPUTIME        { lvalp->val_int = QUEUE_MAXCPUTIME;    return VARIABLE;}
QUEUE_MAXCOUNT          { lvalp->val_int = QUEUE_MAXCOUNT;      return VARIABLE;}
QUEUE_MAXRUNNINGJOBS    { lvalp->val_int = QUEUE_MAXRUNNINGJOBS;return VARIABLE;}
QUEUE_MAXJOBSINQUEUE    { lvalp->val_int = QUEUE_MAXJOBSINQUEUE;return VARIABLE;}
QUEUE_STATUS            { lvalp->val_int = QUEUE_STATUS;        return VARIABLE;}
QUEUE_DISPATCHTYPE      { lvalp->val_int = QUEUE_DISPATCHTYPE;  return VARIABLE;}
QUEUE_PRIORITY          { lvalp->val_int = QUEUE_PRIORITY;      return VARIABLE;}
[A-Za-z][0-9A-Za-z_]*   { lvalp->val_str = strdup(yytext);      return GENERICVAR;}
-?[0-9]+                { lvalp->val_int = atoi(yytext);        return INTEGER;}
\"[^\"]*\"              { lvalp->val_str = strdup(yytext+1);
                          lvalp->val_str[yyleng-2] = '\0';
				          return STRING;}
[!&|=><();\[\]\*\+/\^-] { return *yytext;}
[ \t]+                  ;
.                       ;
%%

int host_attr_wrap()
{
    return 1;
}

int gw_host_update_attr(gw_host_t *host, char *attr)
{
	YY_BUFFER_STATE	str_buffer;
	int             rc;
	int             pos;
	
#ifdef GWHOSTDEBUG	
	int  i;
    char pos_str[5000];
#endif
    
    if ( attr == NULL)
    	return -1;

	pthread_mutex_lock(&host_attr_mutex);
			
	str_buffer = yy_scan_string(attr);

    if (str_buffer == NULL)
    {
        gw_log_print("IM",'E',"Lex error parsing attributes (host %d) \"%s\".\n",
                     host->host_id,
                     attr);
        	
		pthread_mutex_unlock(&host_attr_mutex);
        return -1;
    }
    
    rc = host_update_parse (host, &pos);
    
    if ( rc != 0 ) 
    {
        gw_log_print("IM",'E',"Syntax error parsing attributes (host %i) \"%s\".\n",
                     host->host_id,
                     attr);        	
#ifdef GWHOSTDEBUG
        gw_log_print("IM",'D',"%s\n", attr);
        
        for (i= 0; i<pos; i++)
            pos_str[i]=' ';
        
        pos_str[pos]   = '^';
        pos_str[pos+1] = '\0';
        
        gw_log_print("IM",'D',"%s\n", pos_str);
#endif
		yy_delete_buffer(str_buffer);        
				
		pthread_mutex_unlock(&host_attr_mutex);				
        return -1;
    }
    
	yy_delete_buffer(str_buffer);
	
	pthread_mutex_unlock(&host_attr_mutex);
	return 0;
}

int gw_host_compute_rank(gw_host_t *host, int queue, char *rank)
{
	YY_BUFFER_STATE	str_buffer;
	int             result;
	int             rc;
	int             pos;
	
	if ( host == NULL)
		return 0;
		
    if ( rank == NULL) /*If not defined, all host will be ranked with nice*/
    	return host->nice;

	pthread_mutex_lock(&host_attr_mutex);
	    			
	str_buffer = yy_scan_string(rank);

    if (str_buffer == NULL) 
    {
        gw_log_print("IM",'E',"Lexical error evaluating rank (host %i) \"%s\"\n",
        	         host->host_id,
        	         rank);

		pthread_mutex_unlock(&host_attr_mutex);
		
		return 0;
    }
    
    rc = host_rank_parse (host, queue, &result, &pos);
    
    if ( rc != 0 ) 
    {
        gw_log_print("IM",'E',"Syntax error evaluating rank (host %i) \"%s\".\n",
        	         host->host_id,
        	         rank);
        result = 0;
    } 
    else
    	result += host->nice;
    
	yy_delete_buffer(str_buffer);
	
	pthread_mutex_unlock(&host_attr_mutex);
	
	return result;
}

gw_boolean_t gw_host_check_reqs(gw_host_t *host, int queue, char *reqs)
{
	YY_BUFFER_STATE	str_buffer;
	gw_boolean_t    result = GW_TRUE;
	int             rc;
	int             pos;
	
	if ( host == NULL )
		return GW_FALSE;
		
	if ( reqs == NULL ) /* If not defined, all hosts will be candidates*/
		return GW_TRUE;

	pthread_mutex_lock(&host_attr_mutex);
			
	str_buffer = yy_scan_string(reqs);

    if (str_buffer == NULL) 
    {
        gw_log_print("IM",'E',"Lexical error evaluating requirements (host %i) \"%s\".\n",
        	         host->host_id,
        	         reqs);

		pthread_mutex_unlock(&host_attr_mutex);        
		return GW_FALSE;
    }
    
    rc = host_reqs_parse (host, queue, &result, &pos);
    
    if ( rc != 0 ) 
    {
        gw_log_print("IM",'I',"Syntax error evaluating requirements (host %i) \"%s\".\n",
        	         host->host_id,
        	         reqs);

        result = GW_FALSE;
    }
    
	yy_delete_buffer(str_buffer);
	
	pthread_mutex_unlock(&host_attr_mutex);
	
	return result;
}



int gw_host_client_check_syntax(gw_host_t *host, char *reqs, char *rank)
{
	YY_BUFFER_STATE	str_buffer;
	int             rc;
	int             pos;
	int             i;
    char            pos_str[5000];
    gw_boolean_t    result;
    int             result_rank;
	
	if ( host == NULL )
		return -2;
		
	if ( reqs == NULL )
		return -2;

	if ( rank == NULL )
		return -2;

	pthread_mutex_lock(&host_attr_mutex);
	
	/* Check requirements */
	
	str_buffer = yy_scan_string(reqs);

    if (str_buffer == NULL) 
    {
		pthread_mutex_unlock(&host_attr_mutex);        
		return -2;
    }
    
    rc = host_reqs_parse (host, 0, &result, &pos);
    
    if ( rc != 0 ) 
    {
    	fprintf(stderr,"Syntax error evaluating REQUIREMENTS attribute.\n");
    	fprintf(stderr,"%s\n", reqs);

        for (i= 0; i<pos; i++)
            pos_str[i]=' ';
        
        pos_str[pos]   = '^';
        pos_str[pos+1] = '\0';
        
    	fprintf(stderr,"%s\n", pos_str);
    	
		pthread_mutex_unlock(&host_attr_mutex);
		return -1;
    }
    
	yy_delete_buffer(str_buffer);

	/* Check rank */
		
	str_buffer = yy_scan_string(rank);

    if (str_buffer == NULL) 
    {
		pthread_mutex_unlock(&host_attr_mutex);		
		return -2;
    }
    
    rc = host_rank_parse (host, 0, &result_rank, &pos);
    
    if ( rc != 0 ) 
    {
    
    	fprintf(stderr,"Syntax error evaluating RANK attribute.\n");
    	fprintf(stderr,"%s\n", rank);

        for (i= 0; i<pos; i++)
            pos_str[i]=' ';
        
        pos_str[pos]   = '^';
        pos_str[pos+1] = '\0';
        
    	fprintf(stderr,"%s\n", pos_str);
    	
		pthread_mutex_unlock(&host_attr_mutex);
		return -1;
    }
        
	yy_delete_buffer(str_buffer);
	
	pthread_mutex_unlock(&host_attr_mutex);
	
	return 0;
}
