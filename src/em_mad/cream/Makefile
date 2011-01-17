CC = gcc
CXX = g++

GCCBASEFLAGS = $(WARNING)
WARNING = -W -Wall -Woverloaded-virtual
DBG = -g -O0
CFLAGS = -pipe -I/opt/glite/include -I/usr/lib -I/opt/globus/include/gcc64dbg -I$(HOME)/gsoap-linux-2.7 -I../../../include -DWANT_NAMESPACES -DWANT_CLASSAD_NAMESPACE
CXXFLAGS = $(DBG) -fPIC -pipe $(GCCBASEFLAGS) $(BASEFLAGS) $(CFLAGS)
LDFLAGS = -L/opt/glite/lib -L/opt/glite/lib64/ -L/usr/lib64/ -L/opt/c-ares/lib/ -L/opt/classads/lib64 $(LIBS)
LIBS = -lglite_ce_cream_client_soap -lglite_ce_cream_client_util -lglite_security_gsoap_plugin_276b_gcc64dbgpthr -lvomsapi_gcc64dbgpthr -lboost_regex -lboost_thread -lclassad_ns -lgridsite -llog4cpp -lxml2

progs = gw_em_mad_cream

all: $(progs)

clean:
	@rm $(progs)

gw_em_mad_cream: gw_em_mad_cream.cpp gw_em_mad_cream_actions.cpp

install:
ifdef GW_LOCATION
	@mv gw_em_mad_cream $(GW_LOCATION)/bin/gw_em_mad_cream.bin
	@mv gw_em_mad_cream.sh $(GW_LOCATION)/bin/gw_em_mad_cream
else
	@echo "  GW_LOCATION undefined"
	@exit 2
endif

