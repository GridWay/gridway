install:
ifdef GW_LOCATION
	@mkdir -p $(GW_LOCATION)/libexec/perl
	@cp gw_im_mad_bdii.pl $(GW_LOCATION)/libexec/perl/gw_im_mad_bdii.pl
	@cp gw_im_mad_bdii.sh $(GW_LOCATION)/bin/gw_im_mad_bdii
	@chmod a+x $(GW_LOCATION)/libexec/perl/gw_im_mad_bdii.pl
	@chmod a+x $(GW_LOCATION)/bin/gw_im_mad_bdii
	@echo BDII IM MAD successfully installed
else
        @echo "  You must define the GW_LOCATION env variable"
        @exit 2
endif
