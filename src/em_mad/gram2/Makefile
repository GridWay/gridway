include makefile-header

CC = $(GLOBUS_CC)
LD = $(GLOBUS_LD)

CFLAGS = $(GLOBUS_CFLAGS) $(GLOBUS_INCLUDES) -I../../../include
LDFLAGS = $(GLOBUS_LDFLAGS) $(GLOBUS_PKG_LIBS)

progs = gw_em_mad_gram2

all: $(progs)

gw_em_mad_gram2: gw_em_mad_gram2.c gw_em_mad_gram2_actions.c gw_em_mad_gram2_pool.c

clean:
	rm -f $(progs)

makefile-header:
	@echo "  First, you have to create a makefile-header file with:"
	@echo "      globus-makefile-header --flavor=FLAVOR globus_gram_client > makefile-header"
	@echo "  Replace FLAVOR with gcc32dbg, gcc32dbgpthr, gcc64dbg..."
	@echo "  Maybe, you also have to execute this as root:"
	@echo "      gpt-build -nosrc FLAVOR"
	@exit 2
