include makefile-header

CC = $(GLOBUS_CC)
LD = $(GLOBUS_LD)

CFLAGS = $(GLOBUS_CFLAGS) $(GLOBUS_INCLUDES) -I../../../include
LDFLAGS = $(GLOBUS_LDFLAGS) $(GLOBUS_PKG_LIBS)

progs = gw_tm_mad_ftp

all: $(progs)

gw_tm_mad_ftp: gw_tm_mad_ftp.c gw_tm_ftp_queue.c gw_tm_ftp_stack.c gw_tm_ftp_transfer.c gw_tm_ftp_xfr_pool.c

install:
ifdef GW_LOCATION
	@mv gw_tm_mad_ftp $(GW_LOCATION)/bin/gw_tm_mad_ftp.bin
	@mv gw_tm_mad_ftp.sh $(GW_LOCATION)/bin/gw_tm_mad_ftp
else
	@echo "  GW_LOCATION undefined"
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
