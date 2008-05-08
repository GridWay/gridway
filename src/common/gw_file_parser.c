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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "gw_file_parser.h"

static int gw_parse_line( const char *line, const char *pattern , char **return_msg )
{
    int  found = 0;
    char *pline, *token, *lasts;
    char delimiters[] = GW_FIELD_DELIMITERS;
    
    *return_msg = NULL;
    
    pline  = strdup(line);
    
    token = strtok_r(pline, delimiters, &lasts);
   
    if ( token != NULL )
    {
        if ( strcasecmp(pattern, token) == 0 )
        {
            token = strchr(line, '=');   

            if (token != NULL)
                *return_msg = strdup(token+1);
            else
                *return_msg = NULL;
   
            found = 1;
        }
    }

    free(pline);
        
    return found;
}

int gw_parse_file( const char *file, const char *pattern ,char **return_msg )
{
    FILE *fd;
    char line[256];
    int end=0;
    int found;
    
    line[255]='\0';
    fd = fopen(file,"r");
    
    if (fd==NULL)
    {
        *return_msg = NULL;
        return -1;
    }
    
    while(!end)
    {
        if ( fgets(line, 255, fd) == NULL )
        {
            *return_msg = NULL;
            found = 0;
            end   = 1;
        }
                
        if ( gw_parse_line( line, pattern, return_msg ) )
        {
            found = 1;
            end   = 1;             
        }
    }
        
    fclose (fd);
    
    return found;
}
