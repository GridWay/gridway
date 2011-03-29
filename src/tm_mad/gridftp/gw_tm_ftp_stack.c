/* -------------------------------------------------------------------------- */
/* Copyright 2002-2011, GridWay Project Leads (GridWay.org)                   */
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "gw_tm_ftp_transfer.h"


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

void gw_tm_ftp_stack_init(gw_tm_ftp_stack_t **stack)
{
    *stack = NULL ;
}
	
void gw_tm_ftp_stack_destroy(gw_tm_ftp_stack_t **stack)
{
    if (*stack != NULL)
    {

        gw_tm_ftp_stack_destroy(&((*stack)->next));
        
        free((*stack)->file_name);        
        free((*stack));
    }
    
    return;
}

int gw_tm_ftp_stack_push(gw_tm_ftp_stack_t **stack, const char *file_name, int type, gw_boolean_t expanded)
{
    gw_tm_ftp_stack_t *new_file;

    new_file = (gw_tm_ftp_stack_t *) malloc(sizeof(gw_tm_ftp_stack_t));

    if (new_file == NULL)
        return -1;

    new_file->file_name = strdup(file_name);
    new_file->type      = type;
    new_file->expanded  = expanded;
    
    new_file->next = *stack;    
    *stack         = new_file;

    return 0;
}

gw_tm_ftp_stack_t * gw_tm_ftp_stack_pop(gw_tm_ftp_stack_t **stack)
{
	gw_tm_ftp_stack_t *file;
	
	file  = *stack;
	
	if (*stack != NULL )
		*stack = (*stack)->next;
	else
		*stack = NULL;

	return file;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
