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

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "drmaa.h"
#include <unistd.h>

int main(int argc, char *argv[])
{
    char error[DRMAA_ERROR_STRING_BUFFER];
    int  result;
    char contact[DRMAA_ATTR_BUFFER];
    unsigned int  major;
    unsigned int  minor;
    char drm[DRMAA_ATTR_BUFFER];
    char impl[DRMAA_ATTR_BUFFER];

    result = drmaa_init (NULL, error, DRMAA_ERROR_STRING_BUFFER-1);

    if ( result != DRMAA_ERRNO_SUCCESS)
    {
      fprintf(stderr,"drmaa_init() failed: %s\n", error);
      return -1;
    }
    else
      printf("drmaa_init() success \n");

	drmaa_get_contact(contact,
	                  DRMAA_ATTR_BUFFER-1,
	                  error,
	                  DRMAA_ERROR_STRING_BUFFER-1);

	drmaa_version(&major,&minor,error,DRMAA_ERROR_STRING_BUFFER-1);

	drmaa_get_DRM_system(drm,
	                     DRMAA_ATTR_BUFFER-1,
	                     error,
	                     DRMAA_ERROR_STRING_BUFFER-1);

	drmaa_get_DRMAA_implementation(impl,
	                               DRMAA_ATTR_BUFFER-1,
	                               error,
	                               DRMAA_ERROR_STRING_BUFFER-1);

 	printf("Using %s, details:\n",impl);
 	printf("\t DRMAA version %i.%i\n",major,minor);
 	printf("\t DRMS %s (contact: %s)\n",drm,contact);

    result = drmaa_exit (error, DRMAA_ERROR_STRING_BUFFER-1);

    if ( result != DRMAA_ERRNO_SUCCESS)
    {
      fprintf(stderr,"drmaa_exit() failed: %s\n", error);
      return -1;
    }
    else
      printf("drmaa_exit() success \n");

    return 0;
}
