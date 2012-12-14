CC = gcc
CXX = g++

GCCBASEFLAGS = $(WARNING)
WARNING = -W -Wall -Woverloaded-virtual
VERBOSE=1
DBG = -g -O2 
CFLAGS = -pipe -I<path_to_arc-1_branch_3.0>/src/hed/acc/EMIES/ -I/usr/include/libxml2/ -I/usr/include/sigc++-2.0/ -I/usr/lib64/sigc++-2.0/include/ -I/usr/include/glibmm-2.4/ -I/usr/include/glib-2.0/ -I/usr/lib64/glib-2.0/include/ -I/usr/lib64/glibmm-2.4/include -I<path_to_arc-1_branch_3.0>/include/
# TODO: Uncomment for nordugrid-arc 3.0
#CFLAGS = -pipe -I/usr/src/debug/nordugrid-arc-3.0.0/src/hed/acc/EMIES/ -I/usr/include/libxml2/ -I/usr/include/sigc++-2.0/ -I/usr/lib64/sigc++-2.0/include/ -I/usr/include/glibmm-2.4/ -I/usr/include/glib-2.0/ -I/usr/lib64/glib-2.0/include/ -I/usr/lib64/glibmm-2.4/include -I/usr/src/debug/nordugrid-arc-2.0.0/src/hed/libs/common/
CXXFLAGS = $(DBG) -fPIC -pipe $(GCCBASEFLAGS) $(BASEFLAGS) $(CFLAGS)
LDFLAGS = -L/usr/local/lib/arc $(LIBS)
# TODO: Uncomment for nordugrid-arc 3.0
#LDFLAGS = -L/usr/lib64/arc/ $(LIBS)
LIBS = -laccEMIES 

progs = gw_em_mad_emies

all: $(progs)

clean:
	@rm $(progs)

gw_em_mad_emies: gw_em_mad_emies.cpp gw_em_mad_emies_actions.cpp

install: gw_em_mad_emies
ifdef GW_LOCATION
	@cp gw_em_mad_emies $(GW_LOCATION)/bin/gw_em_mad_emies.bin
	@cp gw_em_mad_emies.sh $(GW_LOCATION)/bin/gw_em_mad_emies
	@chmod a+x $(GW_LOCATION)/bin/gw_em_mad_emies
	@echo EMI-ES EM MAD successfully installed
else
	@echo "  You must define the GW_LOCATION env variable"
	@exit 2
endif
