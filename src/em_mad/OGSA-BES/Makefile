
JAR_FILES = /usr/share/java
GRIDSAM_HOME = /var/lib/tomcat6/webapps/gridsam

CLASSPATH = $(JAR_FILES)/dom4j.jar:$(JAR_FILES)/jaxrpc.jar:$(JAR_FILES)/xmlbeans.jar:$(GRIDSAM_HOME)/WEB-INF/lib/gridsam-schema-2.3.0.jar:$(GRIDSAM_HOME)/WEB-INF/lib/gridsam-core-2.3.0.jar

gw_em_mad_bes.jar: GW_em_mad_bes.java
	@rm -f lib/gw_em_mad_bes.jar
	@mkdir -p package
	javac GW_em_mad_bes_actions.java -classpath .:$(CLASSPATH) -d package
	javac GW_em_mad_bes.java -classpath package -d package
	jar cf gw_em_mad_bes.jar -C package .
	@rm -rf package

install:
ifdef GW_LOCATION
	@cp gw_em_mad_bes.jar $(GW_LOCATION)/lib/
	@cp gw_em_mad_bes.sh $(GW_LOCATION)/bin/GW_em_mad_bes
	@chmod a+x $(GW_LOCATION)/bin/GW_em_mad_bes
	@cp client-config.wsdd $(GW_LOCATION)/lib
	@mkdir -p $(GW_LOCATION)/lib/endorsed
	@cp $(JAR_FILES)/saaj.jar $(GW_LOCATION)/lib/endorsed
	@cp $(JAR_FILES)/xalan2.jar $(GW_LOCATION)/lib/endorsed
	@echo OGSA-BES EM MAD successfully installed
else
	@echo "  GW_LOCATION undefined"
	@exit 2
endif
