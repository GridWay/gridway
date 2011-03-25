include makefile-header

CC = $(GLOBUS_CC)
LD = $(GLOBUS_LD)

CFLAGS = $(GLOBUS_CFLAGS) $(GLOBUS_INCLUDES) -I../../../include
LDFLAGS = $(GLOBUS_LDFLAGS) $(GLOBUS_PKG_LIBS)

progs = gw_tm_mad_ftp

all: $(progs)

gw_tm_mad_ftp: gw_tm_mad_ftp.c gw_tm_ftp_queue.c gw_tm_ftp_stack.c gw_tm_ftp_transfer.c gw_tm_ftp_xfr_pool.c

install: gw_tm_mad_ftp
ifdef GW_LOCATION
	@cp gw_tm_mad_ftp $(GW_LOCATION)/bin/gw_tm_mad_ftp.bin
	@cp gw_tm_mad_ftp.sh $(GW_LOCATION)/bin/gw_tm_mad_ftp
	@chmod a+x $(GW_LOCATION)/bin/gw_tm_mad_ftp
	@echo GridFTP TM MAD successfully installed
else
	@echo "  You must define the GW_LOCATION env variable"
	@exit 2
endif

clean:
	rm -f $(progs)

makefile-header:
	@echo "  First, you have to create a makefile-header file with:"
	@echo "      globus-makefile-header --flavor=FLAVOR globus_gass_copy > makefile-header"
	@echo "  Replace FLAVOR with gcc32dbg, gcc32dbgpthr, gcc64dbg..."
	@echo "  Maybe, you also have to execute this as root:"
	@echo "      gpt-build -nosrc FLAVOR"
	@exit 2
