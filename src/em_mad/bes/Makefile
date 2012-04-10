## RPM/DEB build 
#JAVA_EXT = ""

## Usual build
JAVA_EXT = "/usr/share/gridsam-2.3.0-client/"

CLASSPATH = `find /usr/share/java -name jaxrpc.jar`:`find /usr/share/java ${JAVA_EXT}lib/ -name xbean.jar -o -name xmlbeans.jar | head -n 1`:${JAVA_EXT}lib/gridsam-schema-2.3.0.jar

gw_em_mad_bes.jar: GW_em_mad_bes.java GW_em_mad_bes_actions.java
	@rm -f lib/gw_em_mad_bes.jar
	@mkdir -p package
	javac GW_em_mad_bes_actions.java -classpath .:$(CLASSPATH) -d package
	javac GW_em_mad_bes.java -classpath package:`find /usr/share/java -name jaxrpc.jar` -d package
	jar cf gw_em_mad_bes.jar -C package .
	@rm -rf package

clean: 
	@rm -f gw_em_mad_bes.jar

install: gw_em_mad_bes.jar

ifdef GW_LOCATION
	@cp gw_em_mad_bes.jar $(GW_LOCATION)/lib/
	@cp crypto.properties $(GW_LOCATION)/etc
	@cp client-config.wsdd $(GW_LOCATION)/etc
	@cp gw_em_mad_bes.sh $(GW_LOCATION)/bin/GW_em_mad_bes
	@chmod a+x $(GW_LOCATION)/bin/GW_em_mad_bes
	@echo OGSA-BES EM MAD successfully installed
else
	@echo "  You must define the GW_LOCATION env variable"
	@exit 2
endif
