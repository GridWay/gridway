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

#include <stdlib.h>
#include <stdio.h>       
#include <sys/select.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

extern char *optarg;
extern int   optind, opterr, optopt;

int main (int argc, char **argv )
{
    char   str[4096];
    int    rc,j;
    char   action[20];
    char   xfr_id_s[20];
    int    xfr_id;
    char   cp_xfr_id_s[20];
    int    cp_xfr_id;
    char   src_url[1024];
    char   dst_url[1024];
    char * dst_url_tmp;
    char   modex;
    int    end = 0;

    fd_set in_pipes;
    char   c;
    
    char * url;
    char * stg_url;
    char   buffer[512];
    int    u, f, g;
    char   opt;

    int server_pid = -1;
    int pipes[2];
    FILE *file;

    setbuf(stdout,NULL);

    opterr = 0;
    optind = 1;
    
    u = f = g = 0;
    url = NULL;

    while((opt = getopt(argc, argv, "h:u:fg")) != -1)
        switch(opt)
        {
            case 'u': 
                url = strdup(optarg);
                u   = 1;
                break;

            case 'f': 
                f = 1;
                break;
                
            case 'g': 
                g = 1;
                break;
        }
           
    while (!end)
    {
        FD_ZERO(&in_pipes);
        FD_SET (0,&in_pipes);

        rc = select(1, &in_pipes, NULL, NULL, NULL);
        
        if (rc == -1)
        {
            exit(-1);
        }
        else if (rc == 1)
        {
            j = 0;

            do
            {
                rc = read(0, (void *) &c, sizeof(char));
                str[j++] = c;
            }
            while ( rc > 0 && c != '\n' );
            
            str[j] = '\0';

            if (rc <= 0)
            {
                exit(-1);
            }
            
            rc = sscanf(str,"%s %s %s %c %s %s",
                         action,
                         xfr_id_s,
                         cp_xfr_id_s,
                         &modex,
                         src_url,
                         dst_url);
            
            if (rc != 6 )
            {
                printf("%s %s %s FAILURE Not enough parameters.\n", action, xfr_id_s, cp_xfr_id_s);
                continue;
            }
        
            xfr_id = atoi(xfr_id_s);

            if (strcmp(action, "INIT") == 0 )
            {
                if ( u == 1 )
                {
                    stg_url = url;
                    rc      = 0;
                }
                else if ( g == 1 )
                {
                    /* Start a GASS server */

                    if (pipe(pipes) == -1)
                    {
                        fprintf(stderr, "couldn't create pipes\n");
                        rc = 1;
                    }
                    else
                    {
                        int fscanf_result;
                        server_pid = fork();

                        switch (server_pid)
                        {
                        case 0:     /* Child */
                            close(pipes[0]);
                            dup2(pipes[1], 1);
                            execlp("globus-gass-server", "globus-gass-server", NULL);
                            rc = 1;

                        case -1:    /* Error */
                            fprintf(stderr, "couldn't fork\n");
                            rc = 1;

                        default:    /* Parent */
                            /* Read stdout */
                            close(pipes[1]);
                            file = fdopen(pipes[0], "r");
                            if (file == NULL)
                                perror("opening stdout");
                            fscanf_result = fscanf(file, "%s", buffer);
                            stg_url = strdup(buffer);
                            rc = 0;
                        }
                    }
                }
                else if ( f == 1 )
                {
                    /* Start a GridFTP server */
                    /* Currently unsupported as transfers must be started
                      with "-s 'grid-info-search -issuer'" */
                    if (pipe(pipes) == -1)
                    {
                        fprintf(stderr, "couldn't create pipes\n");
                        rc = 1;
                    }
                    else
                    {
                        int fscanf_result;
                        server_pid = fork();

                        switch (server_pid)
                        {
                        case 0:     /* Child */
                            close(pipes[0]);
                            dup2(pipes[1], 1);
                            execlp("globus-gridftp-server", "globus-gridftp-server", NULL);
                            rc = 1;

                        case -1:    /* Error */
                            fprintf(stderr, "couldn't fork\n");
                            rc = 1;

                        default:    /* Parent */
                            /* Read stdout */
                            close(pipes[1]);
                            file = fdopen(pipes[0], "r");
                            if (file == NULL)
                                perror("opening stdout");
                            fscanf_result = fscanf(file, "%s", buffer);
                            fscanf_result = fscanf(file, "%s", buffer);
                            fscanf_result = fscanf(file, "%s", buffer);
                            fscanf_result = fscanf(file, "%s", buffer);
                            sprintf(str, "gsiftp://%s", buffer);
                            stg_url = strdup(str);
                            rc = 0;
                        }
                    }
                }
                else
                {
                    stg_url = strdup("-");
                    rc = 0;
                }
                                
                if (rc == 1)
                    printf("INIT - - FAILURE Error Activating the staging module.\n");
                else
                    printf("INIT - - SUCCESS %s\n",stg_url);
            }
            else if (strcmp(action, "START") == 0 )
            {
                printf("START %i - SUCCESS -\n",xfr_id);
            }
            else if (strcmp(action, "END") == 0 )
            {
                printf("END %i - SUCCESS -\n",xfr_id);
            }
            else if (strcmp(action, "RMDIR") == 0 )
            {
                printf("RMDIR %i - SUCCESS -\n", xfr_id);
            }
            else if (strcmp(action, "EXISTS") == 0 )
            {
                printf("EXISTS %d - SUCCESS -\n",xfr_id);
            }
            else if (strcmp(action, "MKDIR") == 0 )
            {
                printf("MKDIR %d - SUCCESS %s\n", xfr_id,src_url);
            }                
            else if (strcmp(action, "CP") == 0 )
            {
                cp_xfr_id = atoi(cp_xfr_id_s);
                                    
                if ( g == 1 ) /* Using a gass server */
                {
                    if ((strstr(src_url, "stdout.wrapper") != NULL)
                            || (strstr(src_url, "stderr.wrapper") != NULL))
                    {
                        dst_url_tmp = dst_url + 7; /*skip file:// */
                        chmod(dst_url_tmp, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH );
                    }
                }
                
                printf("CP %d %d SUCCESS (%s->%s)\n", xfr_id,cp_xfr_id,
                    src_url,dst_url);                
            }                
            else if (strcmp(action, "FINALIZE") == 0 )
            {
                end = 1;
            }
            else
            {
                printf("%s %s - FAILURE not a valid action\n", action, xfr_id_s);
            }
        }                       
    }
    
    printf("FINALIZE - - SUCCESS -\n");
    
    return 0;
}
