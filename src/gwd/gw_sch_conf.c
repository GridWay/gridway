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
#include <stdio.h>
#include <stdlib.h>

#include "gw_sch_conf.h"
#include "gw_log.h"
#include "gw_common.h"
#include "gw_job.h"
#include "gw_host.h"

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

void gw_sch_conf_init (gw_sch_conf_t *conf)
{
    conf->disable      = 0;
    
    conf->max_dispatch = GW_SCH_DIPATCH_CHUNK_DEFAULT;
    conf->max_resource = GW_SCH_MAX_RUNNING_RESOURCE_DEFAULT;
    conf->max_user     = GW_SCH_MAX_RUNNING_USER_DEFAULT;

	conf->wfixed         = GW_SCH_FP_WEIGHT_DEFAULT;
	conf->ufixed_default = GW_SCH_FP_DEFAULT;
	
	conf->nufixed  = 0;
    conf->ufixed   = NULL;

	conf->ngfixed  = 0;
    conf->gfixed   = NULL;
	
	conf->wshare         = GW_SCH_SH_WEIGHT_DEFAULT;
	conf->ushare_default = GW_SCH_SH_DEFAULT;

	conf->nushare  = 0;
    conf->ushare   = NULL;
	
	conf->wwaiting  = GW_SCH_WT_WEIGHT_DEFAULT;
    conf->wdeadline = GW_SCH_DL_WEIGHT_DEFAULT;
    
    conf->window_size  = GW_SCH_SH_WINDOW_SIZE_DEFAULT;
    conf->window_depth = GW_SCH_SH_WINDOW_DEPTH_DEFAULT;
    
	conf->wrfixed        = GW_SCH_RP_WEIGHT_DEFAULT; 
	conf->rfixed_default = GW_SCH_RP_DEFAULT;
  
	conf->nifixed = 0;
	conf->ifixed  = NULL;

	conf->nhfixed = 0;
	conf->hfixed  = NULL;
	
	conf->wrank   = GW_SCH_RA_WEIGHT_DEFAULT; 
	
	conf->fr_max_banned = GW_SCH_FR_MAX_BANNED_DEFAULT;
	conf->fr_banned_c   = GW_SCH_FR_BANNED_C_DEFAULT;	

	conf->wusage    = GW_SCH_UG_WEIGHT_DEFAULT;
	conf->ug_window = GW_SCH_UG_WINDOW_DEFAULT;
	conf->ug_ratio  = GW_SCH_UG_RATIO_DEFAULT ;		
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

int gw_sch_get_user_priority(gw_sch_conf_t * conf, 
                             const char *    user, 
                             const char *    group)
{
	int i;
	int priority;
	
	for (i=0; i<conf->nufixed; i++)
	{
        if (strcmp(user,conf->ufixed[i].name)==0)
        {
        	return conf->ufixed[i].value;
        }
	}

	priority = conf->ufixed_default;
	
	for (i=0; i<conf->ngfixed; i++)
	{
        if (strcmp(group,conf->gfixed[i].name)==0)
        {
        	priority    = conf->gfixed[i].value;
        	break;
        }
	}
	
	return priority;
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

int gw_sch_get_host_priority(gw_sch_conf_t * conf, 
                             const char *    host, 
                             const char *    im)
{
	int i;
	int priority;
	
	for (i=0; i<conf->nhfixed; i++)
	{
        if (strcmp(host,conf->hfixed[i].name)==0)
        {
        	return conf->hfixed[i].value;
        }
	}

	priority = conf->rfixed_default;
	
	for (i=0; i<conf->nifixed; i++)
	{
        if (strcmp(im,conf->ifixed[i].name)==0)
        {
        	priority    = conf->ifixed[i].value;
        	break;
        }
	}
	
	return priority;
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

int gw_sch_get_user_fixed(gw_sch_conf_t *conf, const char *user)
{
	int i;
	int priority;
	
	priority = conf->ufixed_default;
	
	for (i=0; i<conf->nufixed; i++)
	{
        if (strcmp(user,conf->ufixed[i].name)==0)
        {
        	priority = conf->ufixed[i].value;
        	break;
        }
	}

	return priority;
}

/* ------------------------------------------------------------------------- */

void gw_sch_set_user_fixed(gw_sch_conf_t *conf, const char *user, int priority)
{
    int i;
    
    if (priority < GW_JOB_MIN_PRIORITY)
        priority = GW_JOB_MIN_PRIORITY;
    else if (priority > GW_JOB_MAX_PRIORITY )
        priority = GW_JOB_MAX_PRIORITY;    

    if (strcmp(user,"DEFAULT")==0)
    {
    	conf->ufixed_default = priority;
    	return;
    }

	for (i=0; i<conf->nufixed; i++)
	{
        if (strcmp(user,conf->ufixed[i].name)==0)
        {
        	conf->ufixed[i].value = priority;
        	return;
        }
	}
	
	i = conf->nufixed++;
	
	conf->ufixed = (gw_sch_conf_array_t *) realloc (conf->ufixed, 
		sizeof (gw_sch_conf_array_t) * conf->nufixed);
		
	conf->ufixed[i].name  = strdup(user);
	conf->ufixed[i].value = priority;
}

/* ------------------------------------------------------------------------- */

int gw_sch_get_group_fixed(gw_sch_conf_t *conf, const char *group)
{
	int i;
	int priority;
	
	priority = -1;
	
	for (i=0; i<conf->ngfixed; i++)
	{
        if (strcmp(group,conf->gfixed[i].name)==0)
        {
        	priority = conf->gfixed[i].value;
        	break;
        }
	}
	
	return priority;
}

/* ------------------------------------------------------------------------- */

void gw_sch_set_group_fixed(gw_sch_conf_t *conf, const char *group, int priority)
{
	int i;

    if (priority < GW_JOB_MIN_PRIORITY)
        priority = GW_JOB_MIN_PRIORITY;
    else if (priority > GW_JOB_MAX_PRIORITY )
        priority = GW_JOB_MAX_PRIORITY;    
    
	for (i=0; i<conf->ngfixed; i++)
	{
        if (strcmp(group,conf->gfixed[i].name)==0)
        {
        	conf->gfixed[i].value = priority;
        	return;
        }
	}

	i = conf->ngfixed++;
	
	conf->gfixed = (gw_sch_conf_array_t *) realloc (conf->gfixed, 
		sizeof (gw_sch_conf_array_t) * conf->ngfixed);
		
	conf->gfixed[i].name  = strdup(group);
	conf->gfixed[i].value = priority;
}

/* ------------------------------------------------------------------------- */

void gw_sch_set_user_share(gw_sch_conf_t *conf, const char *user, int share)
{
    int i;
    int found;
    
    if (strcmp(user,"DEFAULT")==0)
    {
    	conf->ushare_default = share;
    	return;
    }
    
    found = 0;
    
	for (i=0; i<conf->nushare; i++)
	{
        if (strcmp(user,conf->ushare[i].name)==0)
        {
        	conf->ushare[i].value = share;
        	found = 1;
        	break;
        }
	}
	
	if ( found )
		return;
	
	i = conf->nushare++;
	
	conf->ushare = (gw_sch_conf_array_t *) realloc (conf->ushare, 
		sizeof (gw_sch_conf_array_t) * conf->nushare);
		
	conf->ushare[i].name  = strdup(user);
	conf->ushare[i].value = share;
}

/* ------------------------------------------------------------------------- */

int gw_sch_get_user_share(gw_sch_conf_t *conf, const char *user)
{
	int i;
	int share;
	
	share = conf->ushare_default;
	
	for (i=0; i<conf->nushare; i++)
	{
        if (strcmp(user,conf->ushare[i].name)==0)
        {
        	share = conf->ushare[i].value;
        	break;
        }
	}
	
	return share;
}

/* ------------------------------------------------------------------------- */

void gw_sch_set_im_fixed(gw_sch_conf_t *conf, const char *im, int priority)
{
    int i;
    
    if (priority < GW_HOST_MIN_PRIORITY)
        priority = GW_HOST_MIN_PRIORITY;
    else if (priority > GW_HOST_MAX_PRIORITY )
        priority = GW_HOST_MAX_PRIORITY;
    
    if (strcmp(im,"DEFAULT")==0)
    {
    	conf->rfixed_default = priority;
    	return;
    }
    
	for (i=0; i<conf->nifixed; i++)
	{
        if (strcmp(im,conf->ifixed[i].name)==0)
        {
        	conf->ifixed[i].value = priority;
        	return;
        }
	}
	
	i = conf->nifixed++;
	
	conf->ifixed = (gw_sch_conf_array_t *) realloc (conf->ifixed, 
		sizeof (gw_sch_conf_array_t) * conf->nifixed);
		
	conf->ifixed[i].name  = strdup(im);
	conf->ifixed[i].value = priority;
}
/* ------------------------------------------------------------------------- */

int gw_sch_get_im_fixed(gw_sch_conf_t *conf, const char *im)
{
	int i;
	int priority;
	
	priority = conf->rfixed_default;
	
	for (i=0; i<conf->nifixed; i++)
	{
        if (strcmp(im,conf->ifixed[i].name)==0)
        {
        	priority = conf->ifixed[i].value;
        	break;
        }
	}
	
	return priority;
}

/* ------------------------------------------------------------------------- */

void gw_sch_set_host_fixed(gw_sch_conf_t *conf, const char *host, int priority)
{
	int i;

    if (priority < GW_HOST_MIN_PRIORITY)
        priority = GW_HOST_MIN_PRIORITY;
    else if (priority > GW_HOST_MAX_PRIORITY )
        priority = GW_HOST_MAX_PRIORITY;
    
	for (i=0; i<conf->nhfixed; i++)
	{
        if (strcmp(host,conf->hfixed[i].name)==0)
        {
        	conf->hfixed[i].value = priority;
        	return;
        }
	}
	
	i = conf->nhfixed++;
	
	conf->hfixed = (gw_sch_conf_array_t *) realloc (conf->hfixed, 
		sizeof (gw_sch_conf_array_t) * conf->nhfixed);
		
	conf->hfixed[i].name  = strdup(host);
	conf->hfixed[i].value = priority;
}

/* ------------------------------------------------------------------------- */

int gw_sch_get_host_fixed(gw_sch_conf_t *conf, const char *host)
{
	int i;
	int priority;
	
	priority = -1;
	
	for (i=0; i<conf->nhfixed; i++)
	{
        if (strcmp(host,conf->hfixed[i].name)==0)
        {
        	priority = conf->hfixed[i].value;
        	break;
        }
	}
	
	return priority;
}


/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

void gw_sch_set_var (gw_sch_conf_t * conf, int var, float val)
{
	gw_sch_var_t variable = (gw_sch_var_t) var;

	switch (variable)
	{
		case DISABLE:
            conf->disable = val;
        break;
        
        case DISPATCH_CHUNK:
            conf->max_dispatch = val;
        break;
        
        case MAX_RUNNING_USER:
            conf->max_user = val;
        break;
        
        case MAX_RUNNING_RESOURCE:
            conf->max_resource = val;
        break;
        
        case FP_WEIGHT:
			conf->wfixed = val;
		break;
		
		case WT_WEIGHT:
            conf->wwaiting = val;
		break;
		
		case DL_WEIGHT:
            conf->wdeadline = val;
		break;

		case DL_HALF:
            conf->dl_half = (int) val;
		break;
				
		case SH_WEIGHT:
			conf->wshare = val;
		break;

		case SH_WINDOW_SIZE:
			conf->window_size = val;
		break;

		case SH_WINDOW_DEPTH:
			if ( val > 10 )
				conf->window_depth = 10;
			else
				conf->window_depth = (int) val;
		break;

		case RP_WEIGHT:
			conf->wrfixed = val;
		break;

		case RA_WEIGHT:
			conf->wrank = val;
		break;
		
		case  FR_MAX_BANNED:
			conf->fr_max_banned = (int) val;
		break;

		case  FR_BANNED_C:
			conf->fr_banned_c = val;
		break;
		
		case UG_WEIGHT:
			conf->wusage = val;
		break;
		
		case UG_HISTORY_WINDOW:
			conf->ug_window = val;
		break;
		
		case UG_HISTORY_RATIO:
			conf->ug_ratio = val;		
		break;
		
		default:
		break;
	}
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

void gw_sch_set_svar(gw_sch_conf_t * conf, int var, char *str, int val)
{
	switch ((gw_sch_var_t) var)
	{
		case FP_USER:
			gw_sch_set_user_fixed(conf, str, val);
		break;
		
		case FP_GROUP:
			gw_sch_set_group_fixed(conf, str, val);
		break;
		
		case SH_USER:
			gw_sch_set_user_share(conf, str, val);
		break;

		case RP_HOST:
			gw_sch_set_host_fixed(conf, str, val);
		break;

		case RP_IM:
			gw_sch_set_im_fixed(conf, str, val);
		break;
		
		default:
		break;
	}
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
