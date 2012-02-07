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


#include "gw_scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

/* ------------------------------------------------------------------------- */
/* FUNCTION DEFINICION                                                       */
/* ------------------------------------------------------------------------- */

static void setup_args(char **argv, char ***av, int *ac);

static void scheduler (gw_scheduler_t * sched,
                       void *           user_arg);
							 
int main(int argc, char **argv)
{
  	char          opt;
  	char **       av;
  	int           ac;
  	
  	/* GWD pass all the arguments in one string, this function
  	 * splits the argument in argc, argv fashion. So you can
  	 * use getopt for argument parsing
  	 * 
  	 * opt = getopt(ac, av, "h:u:c:"));
  	 */
  	 
	setup_args(argv, &av, &ac);
	     
    free(av);
	
	/* Just call the scheduler loop. GWD will call your
	 * scheduler every scheduling interval
	 */
	 
     gw_scheduler_loop(scheduler, (void *)&params);
     
     return 0;
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

static void scheduler (gw_scheduler_t * sched,
    				   void *           user_arg)
{
	
/* You issue schedules with the following strings 
 * (see the gridway developer documentation for more
 * details)
 */
 
/* "SCHEDULE_JOB %i SUCCESS %i:%s:%i\n"
 * The arguments are (in order):
 *     1.- the job_id (%i)
 *     2.- the host_id (%i)
 *     3.- queue name (%s)
 *     4.- rank (%i)
 */

 
	return 0;	
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

static void setup_args(char **argv, char ***av, int *ac)
{
  char *  arg;
  char *  tk;

  if ((argv == NULL) || (argv[0] == NULL) || (argv[1] == NULL))
  {
  	*av = NULL;
  	*ac = 0;
  	return;
  }
  
  *av      = (char **) malloc(sizeof(char *));
  (*av)[0] = strdup(argv[0]);

  arg = strdup(argv[1]);
  tk  = strtok(arg," ");
 
  for(*ac=1; tk != NULL; tk = strtok(NULL," "), (*ac)++)
  {
      *av       = realloc( (void *) (*av), (*ac+1) * sizeof(char *));
      (*av)[*ac] = strdup(tk);
  }

  free(arg);	
}
