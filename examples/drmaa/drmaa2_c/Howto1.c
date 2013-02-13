/* -------------------------------------------------------------------------- */
/* Copyright 2002-2013, GridWay Project Leads (GridWay.org)                   */
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
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "drmaa2.h"
#include <unistd.h>

int main(int argc, char *argv[])
{
    drmaa2_string name;
    drmaa2_version version;

    name = drmaa2_get_drmaa_name();
    printf("=========================================\n");
    printf("This is %s C bindings.\n", name);
    drmaa2_string_free(&name);

    version = drmaa2_get_drmaa_version();
    printf("=========================================\n");
    printf("This DRMAA version is %s.%s.\n", version->major, version->minor);
    drmaa2_version_free(&version);

    name=drmaa2_get_drms_name();
    printf("=========================================\n");
    printf("The DRM is %s.\n", name);
    drmaa2_string_free(&name);
  
    version = drmaa2_get_drms_version();
    printf("=========================================\n");
    printf("The DRM version is %s.%s\n", version->major, version->minor);
    printf("=========================================\n");
    drmaa2_version_free(&version);

    return 0;
}
