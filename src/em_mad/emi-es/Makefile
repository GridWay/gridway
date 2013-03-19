CC = gcc
CXX = g++

GCCBASEFLAGS = $(WARNING)
WARNING = -W -Wall -Woverloaded-virtual
VERBOSE=1
DBG = -g -O2 
#ARC_SRC = /usr/src/debug/nordugrid-arc-3.0.0/
#ARC_INSTALL = /usr/local
ARC_SRC = $(HOME)/arc1-3.0
ARC_INSTALL = $(HOME)/arc_install
LIBS = -laccEMIES -larccommon
CFLAGS = -pipe -I$(ARC_SRC)/src/hed/acc/EMIES/ -I/usr/include/libxml2/ -I/usr/include/sigc++-2.0/ -I/usr/lib64/sigc++-2.0/include/ -I/usr/include/glibmm-2.4/ -I/usr/include/glib-2.0/ -I/usr/lib64/glib-2.0/include/ -I/usr/lib64/glibmm-2.4/include -I$(ARC_INSTALL)/include/
CXXFLAGS = $(DBG) -fPIC -pipe $(GCCBASEFLAGS) $(BASEFLAGS) $(CFLAGS)
LDFLAGS = -L$(ARC_INSTALL)/lib -L$(ARC_INSTALL)/lib/arc $(LIBS)

progs = gw_em_mad_emies

all: $(progs)

clean:
	@rm *.o
	@rm $(progs)

gw_em_mad_emies: gw_em_mad_emies.o gw_em_mad_emies_actions.o

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
