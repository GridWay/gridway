CFLAGS = -I../../../include

progs = gw_tm_mad_dummy

all: $(progs)

gw_tm_mad_dummy: gw_tm_mad_dummy.c

install:
ifdef GW_LOCATION
	@mv gw_tm_mad_dummy $(GW_LOCATION)/bin/gw_tm_mad_dummy.bin
	@mv gw_tm_mad_dummy.sh $(GW_LOCATION)/bin/gw_tm_mad_dummy
else
        @echo "  GW_LOCATION undefined"
        @exit 2
endif

clean:
	rm -f $(progs)
