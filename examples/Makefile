#
# Makefile
#

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

# -----------------------------
# Directory definitions
# -----------------------------

INCLUDE_DIR  = ../include
EXAMPLES_DIR = ./
LIB_DIR      = ../lib

DRMAA        = ../lib/drmaa.jar


#---------------------------------
# Compilation Flags and Libraries
#---------------------------------
CC    = gcc
JAVAC = javac

SYSYTEM   = linux
# Solaris configuration (uncomment)
#SYSYTEM       = solaris
#LIBS_SUNOS    = -lnsl -lsocket
#CFLAGS_SUNOS  = -m64

LIBS           =  -ldrmaa -lpthread $(LIBS_SUNOS)
CFLAGS         = -Wall -g $(CFLAGS_SUNOS)
INCLUDE_FLAGS  = -I$(INCLUDE_DIR)
LDOPTIONS      = -shared

#----------------------------------------------
#  Objects and Programs for DRMAA Howtos
#----------------------------------------------

HOWTOS_COBJS := $(wildcard $(EXAMPLES_DIR)/drmaa/*.c)
HOWTOS_COBJS := $(HOWTOS_COBJS:%.c=%.o)

HOWTOS_JOBJS := $(wildcard $(EXAMPLES_DIR)/drmaa_java/*.java)
HOWTOS_JOBJS := $(HOWTOS_JOBJS:%.java=%.class)

#------------------------
# Compilation Rules
#------------------------
.SUFFIXES: .c .o

.c.o:
	$(CC) $(CFLAGS) $(INCLUDE_FLAGS) -g -c -o $@ $<

%.class: %.java
	$(JAVAC) -classpath $(DRMAA) $<

#--------------------
#  targets
#--------------------

all: pi drmaa_c_howtos drmaa_java_howtos

pi: $(EXAMPLES_DIR)/pi/pi.o
	$(CC) $(CFLAGS) -o $(EXAMPLES_DIR)/pi/pi $(EXAMPLES_DIR)/pi/pi.o

drmaa_c_howtos: $(HOWTOS_COBJS)
	$(CC) $(CFLAGS) $(EXAMPLES_DIR)/drmaa/Howto1.o -o $(EXAMPLES_DIR)/drmaa/Howto1 -L $(LIB_DIR) $(LIBS)
	$(CC) $(CFLAGS) $(EXAMPLES_DIR)/drmaa/Howto2.o -o $(EXAMPLES_DIR)/drmaa/Howto2 -L $(LIB_DIR) $(LIBS)
	$(CC) $(CFLAGS) $(EXAMPLES_DIR)/drmaa/Howto3.o -o $(EXAMPLES_DIR)/drmaa/Howto3 -L $(LIB_DIR) $(LIBS)
	$(CC) $(CFLAGS) $(EXAMPLES_DIR)/drmaa/Howto4.o -o $(EXAMPLES_DIR)/drmaa/Howto4 -L $(LIB_DIR) $(LIBS)

drmaa_java_howtos: $(HOWTOS_JOBJS)

clean:
	@rm -f $(EXAMPLES_DIR)/drmaa/core* $(EXAMPLES_DIR)/drmaa/ht[1-6] \
	$(EXAMPLES_DIR)/drmaa/std* $(EXAMPLES_DIR)/drmaa_java/job_template* \
	$(EXAMPLES_DIR)/drmaa_java/std* $(HOWTOS_COBJS)  $(HOWTOS_JOBJS) \
	$(EXAMPLES_DIR)/drmaa/Howto[1-6] $(EXAMPLES_DIR)/pi/std* \
	$(EXAMPLES_DIR)/ls/std* $(EXAMPLES_DIR)/sleep/oo* \
	$(EXAMPLES_DIR)/sleep/ee* $(EXAMPLES_DIR)/sleep/outfile* \
	$(EXAMPLES_DIR)/sleep/env.* $(EXAMPLES_DIR)/pi/pi \
	$(EXAMPLES_DIR)/pi/pi.o
