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

require 'mkmf'
require 'getoptlong'

help_text=<<EOT

Generate Makefile for DRMAA ruby extension

  -g dir, --gridway dir     Specify the directory where GridWay is installed.
  -r [dir], --rubylib [dir] Location where the libraries will be installed.
                            If the directory is ommited the ruby installation
                            directory will be used. By default the extension
                            will be installed in $GW_LOCATION/lib/ruby.
  -h, --help                This text.
EOT

opts=GetoptLong.new(
	['--gridway', '-g', GetoptLong::REQUIRED_ARGUMENT],
	['--rubylib', '-r', GetoptLong::OPTIONAL_ARGUMENT],
	['--help', '-h', GetoptLong::NO_ARGUMENT]
)

gw_location=nil
install_location=nil

opts.each do |opt, arg|
	case opt
	when '--gridway':
		gw_location=arg
	when '--rubylib'
		if arg && arg!=""
			install_location=arg
		else
			install_location=CONFIG['sitearchdir']
		end
	when '--help'
		puts help_text
		exit
	end
end

# Set location variables
if !gw_location
	if !ENV['GW_LOCATION']
		puts "GW_LOCATION not set and --gridway parameter not supplied."
		exit -1
	end
	gw_location=ENV['GW_LOCATION']
end

install_location=gw_location+"/lib/ruby" if !install_location

CONFIG['sitearchdir']=install_location

# Generate bindings if it does not already exist
if !File.exists?('drmaa_r_wrap.c')
	out=system("swig -ruby -I#{gw_location}/include drmaa_r.i")
	if !out
	    STDERR.puts "Swig command failed, do you have swig installed?\n"
	    exit -1
	end
end

# Name of C files to build
build_files=['drmaa_r_wrap']

$srcs=Array.new
$objs=Array.new

build_files.each{|f|
	$srcs<<f+'.c'
	$objs<<f+'.o'
}

# Configure directories and library
dir_config('DRMAA', gw_location+'/include', gw_location+'/src/drmaa/.libs')
#have_library('drmaa')
$libs+=" -ldrmaa "

# Generate Makefile
create_makefile('DRMAA')
