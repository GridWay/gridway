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

#include <stdlib.h>
#include "gw_array.h"

int gw_array_init(gw_array_t *array, int number_of_tasks, int array_id)
{
    int i;
    
    pthread_mutex_init(&(array->mutex),(pthread_mutexattr_t *)NULL); 
    
    pthread_mutex_lock(&(array->mutex));

    array->array_id     = array_id;    
    array->last_task_id = -1;

    array->number_of_tasks = number_of_tasks;
    
    array->freed_tasks = 0;
        
    array->job_ids = (int*) malloc(number_of_tasks * sizeof(int));
    
    if ( array->job_ids == NULL )
    {
        pthread_mutex_unlock(&(array->mutex));
        pthread_mutex_destroy(&(array->mutex));        
        return -1;
    }
        
    for ( i = 0; i < number_of_tasks ; i++)
        array->job_ids[i] = -1;
                    
    pthread_mutex_unlock(&(array->mutex));
    
    return 0;
}

/* -------------------------------------------------------------------------- */

void gw_array_destroy(gw_array_t *array)
{
    free(array->job_ids);

    pthread_mutex_unlock(&(array->mutex));
    
    pthread_mutex_destroy(&(array->mutex));
    
    return;
}

/* -------------------------------------------------------------------------- */

int gw_array_add_task(gw_array_t *array, int job_id)
{
    int task_id;
    
    task_id = array->last_task_id;
    
    if ( task_id >= array->number_of_tasks )
        return -1;
            
    task_id++;
    
    array->job_ids[task_id] = job_id;
    array->last_task_id     = task_id;
    
    return (task_id);
}

/* -------------------------------------------------------------------------- */

int gw_array_del_task(gw_array_t *array, int task_id)
{
    int remaining_tasks;
    
    if ( ( task_id >= 0 ) && ( task_id < array->number_of_tasks ) )
        if ( array->job_ids[task_id] != -1 )
        {
            array->job_ids[task_id] = -1;
            array->freed_tasks++;
        }
    
    remaining_tasks = array->number_of_tasks - array->freed_tasks;
    
    return ( remaining_tasks );
}

/* -------------------------------------------------------------------------- */
