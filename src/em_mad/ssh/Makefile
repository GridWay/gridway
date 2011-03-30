install:
ifdef GW_LOCATION
	@mkdir -p $(GW_LOCATION)/libexec/ruby
	@cp gw_em_mad_ssh.rb gwmad.rb rslparser.rb sshjob.rb sshresource.rb $(GW_LOCATION)/libexec/ruby
	@cp gw_em_mad_ssh.sh $(GW_LOCATION)/bin/gw_em_mad_ssh
	@chmod a+x $(GW_LOCATION)/bin/gw_em_mad_ssh
	@echo SSH EM MAD successfully installed
else
	@echo "  You must define the GW_LOCATION env variable"
	@exit 2
endif
