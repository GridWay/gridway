#!/bin/sh

# ------------------------------------------------------------------------------
# Copyright 2002-2006 GridWay Team, Distributed Systems Architecture
# Group, Universidad Complutense de Madrid
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may
# not use this file except in compliance with the License. You may obtain
# a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ------------------------------------------------------------------------------


CURRENT_DIR=$PWD

cd /local/jfontan/tmp/nordugrid

. ./setup.sh

cd $CURRENT_DIR

/usr/bin/python2.3 $GW_LOCATION/bin/gw_em_mad_nordugrid.py | tee /tmp/nordugrid_em.log




