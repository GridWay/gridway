SRC = Gw_im_mad_mds4.java Host.java Mds4QueryParser.java QueryClient.java Queue.java ServiceIM.java

gw_im_mad_mds4.jar: QueryClient.java Mds4QueryParser.java
	@rm -f lib/gw_im_mad_mds4.jar
	@mkdir -p package
	javac Gw_im_mad_mds4.java -classpath .:$(CLASSPATH) -d package
	jar cf gw_im_mad_mds4.jar -C package/ .
	@rm -rf package

QueryClient.java:
	ln -s QueryClient_GT42.java QueryClient.java

Mds4QueryParser.java:
	ln -s Mds4QueryParser_GT42.java Mds4QueryParser.java
