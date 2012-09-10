CC = gcc
CXX = g++

GCCBASEFLAGS = $(WARNING)
WARNING = -W -Wall -Woverloaded-virtual
DBG = -g -O0
CFLAGS = -pipe -I../../../include -DWANT_NAMESPACES -DWANT_CLASSAD_NAMESPACE
CXXFLAGS = $(DBG) -fPIC -pipe $(GCCBASEFLAGS) $(BASEFLAGS) $(CFLAGS)
LDFLAGS = $(LIBS)
LIBS = -lglite_ce_cream_client_soap -lglite_ce_cream_client_util -lglite_security_gsoap_plugin_2713_cxx

progs = gw_em_mad_cream

all: $(progs)

clean:
	@rm $(progs)

gw_em_mad_cream: gw_em_mad_cream.cpp gw_em_mad_cream_actions.cpp

install: gw_em_mad_cream
ifdef GW_LOCATION
	@cp gw_em_mad_cream $(GW_LOCATION)/bin/gw_em_mad_cream.bin
	@cp gw_em_mad_cream.sh $(GW_LOCATION)/bin/gw_em_mad_cream
	@chmod a+x $(GW_LOCATION)/bin/gw_em_mad_cream
	@echo CREAM EM MAD successfully installed
else
	@echo "  You must define the GW_LOCATION env variable"
	@exit 2
endif
