CFLAGS = -I../../../include

progs = gw_tm_mad_dummy

all: $(progs)

gw_tm_mad_dummy: gw_tm_mad_dummy.c

install: gw_tm_mad_dummy
ifdef GW_LOCATION
	@cp gw_tm_mad_dummy $(GW_LOCATION)/bin/gw_tm_mad_dummy.bin
	@cp gw_tm_mad_dummy.sh $(GW_LOCATION)/bin/gw_tm_mad_dummy
	@chmod a+x $(GW_LOCATION)/bin/gw_tm_mad_dummy
	@echo Dummy TM MAD successfully installed
else
	@echo "  You must define the GW_LOCATION env variable"
        @exit 2
endif

clean:
	rm -f $(progs)
