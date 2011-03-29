#
# Makefile
#

# -------------------------------------------------------------------------- 
# Copyright 2002-2011, GridWay Project Leads (GridWay.org)                   
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

INCLUDE_DIR  = $(GW_LOCATION)/include
EXAMPLES_DIR = ./
LIB_DIR      = $(GW_LOCATION)/lib

DRMAA        = $(GW_LOCATION)/lib/drmaa.jar


#---------------------------------
# Compilation Flags and Libraries
#---------------------------------
CC    = gcc
JAVAC = javac

#Solaris configuration (uncomment)
#LIBS_SUNOS    = -lnsl -lsocket

#64 bits arch (uncomment)
#CFLAGS64  = -m64

LIBS           = -ldrmaa -lpthread $(LIBS_SUNOS)
CFLAGS         = -Wall -g $(CFLAGS64)
INCLUDE_FLAGS  = -I$(INCLUDE_DIR)
LDOPTIONS      = -shared

#----------------------------------------------
#  Objects and Programs for DRMAA Howtos
#----------------------------------------------

HOWTOS_COBJS := $(wildcard $(EXAMPLES_DIR)/drmaa_c/*.c)
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

all: drmaa_c_howtos 

drmaa_c_howtos: $(HOWTOS_COBJS)
	$(CC) $(CFLAGS) $(EXAMPLES_DIR)/drmaa_c/Howto1.o -o $(EXAMPLES_DIR)/drmaa_c/Howto1 -L $(LIB_DIR) $(LIBS)
	$(CC) $(CFLAGS) $(EXAMPLES_DIR)/drmaa_c/Howto2.o -o $(EXAMPLES_DIR)/drmaa_c/Howto2 -L $(LIB_DIR) $(LIBS)
	$(CC) $(CFLAGS) $(EXAMPLES_DIR)/drmaa_c/Howto3.o -o $(EXAMPLES_DIR)/drmaa_c/Howto3 -L $(LIB_DIR) $(LIBS)
	$(CC) $(CFLAGS) $(EXAMPLES_DIR)/drmaa_c/Howto4.o -o $(EXAMPLES_DIR)/drmaa_c/Howto4 -L $(LIB_DIR) $(LIBS)

drmaa_java_howtos: $(HOWTOS_JOBJS)

clean:
	@rm -f $(EXAMPLES_DIR)/drmaa_c/core* $(EXAMPLES_DIR)/drmaa_c/ht[1-6] \
	$(EXAMPLES_DIR)/drmaa_c/std* $(EXAMPLES_DIR)/drmaa_java/job_template* \
	$(EXAMPLES_DIR)/drmaa_java/std* $(HOWTOS_COBJS)  $(HOWTOS_JOBJS) \
	$(EXAMPLES_DIR)/drmaa/Howto[1-6]
