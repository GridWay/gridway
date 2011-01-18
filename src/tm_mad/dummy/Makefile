CFLAGS = -I../../../include

progs = gw_tm_mad_dummy

all: $(progs)

gw_tm_mad_dummy: gw_tm_mad_dummy.c

install:
ifdef GW_LOCATION
	@cp gw_tm_mad_dummy $(GW_LOCATION)/bin/gw_tm_mad_dummy.bin
	@cp gw_tm_mad_dummy.sh $(GW_LOCATION)/bin/gw_tm_mad_dummy
	@echo Dummy TM MAD successfully installed
else
        @echo "  GW_LOCATION undefined"
        @exit 2
endif

clean:
	rm -f $(progs)
