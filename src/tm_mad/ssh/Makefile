install:
ifdef GW_LOCATION
	@mkdir $(GW_LOCATION)/libexec/ruby
	@cp gw_tm_mad_ssh.rb $(GW_LOCATION)/libexec/ruby
	@cp gw_tm_mad_ssh.sh $(GW_LOCATION)/bin/gw_tm_mad_ssh
	@chmod a+x $(GW_LOCATION)/bin/gw_tm_mad_ssh
	@echo SSH TM MAD successfully installed
else
        @echo "  GW_LOCATION undefined"
        @exit 2
endif
