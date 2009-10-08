# -------------------------------------------------------------------------- 
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
# -------------------------------------------------------------------------- 

$:<<ENV['GW_LOCATION']+'/lib/ruby'

require 'DRMAA'
include DRMAA

(result, error)=drmaa_init(nil)

if result != DRMAA_ERRNO_SUCCESS
  STDERR.puts "drmaa_init() failed: #{error}"
  exit -1
else
  puts "drmaa_init() success"
  
  (result, contact, error)=drmaa_get_contact
  (result, major, minor, error)=drmaa_version
  (result, drm, error)=drmaa_get_DRM_system
  (result, impl, error)=drmaa_get_DRMAA_implementation
  
  puts "Using #{impl}, details:"
  puts "\t DRMAA version #{major}.#{minor}"
  puts "\t DRMS #{drm} (contact: #{contact})"

  (result, error)=drmaa_exit
  
  if result != DRMAA_ERRNO_SUCCESS
    STDERR.puts "drmaa_exit() failed: #{error}"
    exit -1
  else
    puts "drmaa_exit() success"
  end
  
  exit 0
end

