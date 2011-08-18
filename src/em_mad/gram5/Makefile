include makefile-header

CC = $(GLOBUS_CC)
LD = $(GLOBUS_LD)

CFLAGS = $(GLOBUS_CFLAGS) $(GLOBUS_INCLUDES) -I../../../include
LDFLAGS = $(GLOBUS_LDFLAGS) $(GLOBUS_PKG_LIBS)

progs = gw_em_mad_gram5

all: $(progs)

gw_em_mad_gram5: gw_em_mad_gram5.c gw_em_mad_gram5_actions.c gw_em_mad_gram5_pool.c

install: gw_em_mad_gram5
ifdef GW_LOCATION
	@cp gw_em_mad_gram5 $(GW_LOCATION)/bin/gw_em_mad_gram5.bin
	@cp gw_em_mad_gram5.sh $(GW_LOCATION)/bin/gw_em_mad_gram5
	@chmod a+x $(GW_LOCATION)/bin/gw_em_mad_gram5
	@echo GRAM5 EM MAD successfully installed
else
	@echo "  You must define the GW_LOCATION env variable"
	@exit 2
endif

clean:
	rm -f $(progs)

makefile-header:
	@echo "  First, you have to create a makefile-header file with:"
	@echo "    globus-makefile-header --flavor=FLAVOR globus_gram_client > makefile-header"
	@echo "  Replace FLAVOR with gcc32dbg, gcc32dbgpthr, gcc64dbg..."
	@echo "  Maybe, you also have to execute this as root:"
	@echo "    gpt-build -nosrc FLAVOR"
	@exit 2
