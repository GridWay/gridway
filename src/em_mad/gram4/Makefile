gw_em_mad_gram4.jar: Gw_em_mad_gram4.java
	@rm -f lib/gw_em_mad_gram4.jar
	@mkdir -p package
	javac Gw_em_mad_gram4.java -classpath .:$(CLASSPATH) -d package
	jar cf gw_em_mad_gram4.jar -C package .
	@rm -rf package

Gw_em_mad_gram4.java:
	ln -s Gw_em_mad_gram4_GT42.java Gw_em_mad_gram4.java

install:
ifdef GW_LOCATION
	@cp gw_em_mad_gram4.jar $(GW_LOCATION)/lib/
	@cp gw_em_mad_gram4.sh $(GW_LOCATION)/bin/gw_em_mad_gram4
	@echo GRAM4 EM MAD successfully installed
else
        @echo "  GW_LOCATION undefined"
        @exit 2
endif
