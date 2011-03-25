install:
ifdef GW_LOCATION
	@cp gw_im_mad_mds2.sh $(GW_LOCATION)/bin/gw_im_mad_mds2
	@chmod a+x $(GW_LOCATION)/bin/gw_im_mad_mds2
	@echo MDS2 IM MAD successfully installed
else
	@echo "  You must define the GW_LOCATION env variable"
	@exit 2
endif
