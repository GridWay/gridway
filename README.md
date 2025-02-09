# GridWay Metascheduler

## Description

The GridWay Metascheduler enables large-scale, reliable and efficient sharing 
of computing resources: clusters, supercomputers, stand-alone servers... It
supports different LRM systems (PBS, SGE, LSF, Condor) within a single
organization or scattered across several administrative domains. GridWay
provides a single point of access to all resources in your organization, from
in-house systems to Grid infrastructures and Cloud providers.

GridWay supports most of existing Grid middlewares (Globus WS and pre-WS, LCG, 
CREAM, ARC...) and has been used on main production Grid infrastructures.

## Release Notes

The release notes can be found online:
http://gridway.ucm.es/doku.php?id=software:release_notes:5.14

## Documentation

The documentation is completely online:
http://gridway.ucm.es/doku.php?id=documentation

## Installation

Check the Installation Guide for more details.

Depending on whether you need a single-user or multi-user configuration, it gets
slightly more complex due to the role of GridWay administrator. In any case, it
summarizes to these steps:

1) Install the required packages (make, gcc, java, ruby...)
2) In a multi-user configuration, create the gwusers group and the gwadmin user.
3) Set the envinronment ($GW_LOCATION, $GLOBUS_LOCATION, $JAVA_HOME, $PATH,
$LD_LIBRARY_PATH...)
4) Change to the directory containing the GridWay distribution.
5) Execute "./configure; make; make install".
6) Build and install the needed drivers executing "make; make install", in
$GW_LOCATION/src/{im_mad,em_mad,tm_mad}. Before, make sure you have installed
the needed middleware (client-side) and packages. Some drivers have specific
requirements that will be shown at build time.
7) Configure GridWay through the file $GW_LOCATION/etc/gwd.conf (check the
Configuration Guide).
8) Launch the GridWay daemon from $GW_LOCATION/bin/gwd.
9) Test it and enjoy!

## Troubleshooting

Sometimes problems arise. The GridWay community provides free support 
http://gridway.ucm.es/doku.php?id=support.

## Credits

The GridWay team.
http://gridway.ucm.es, http://dsa.ucm.es.

## License

Copyright 2002-2013, GridWay Project Leads (GridWay.ucm.es)          

Licensed under the Apache License, Version 2.0 (the "License"); you may
not use this file except in compliance with the License. You may obtain
a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
