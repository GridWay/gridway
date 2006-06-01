doxygen: ../include/drmaa.h
	doxygen drmaa_c/drmaa.dox
	cp drmaa_c/index.html drmaa_c/html/index.html

javadoc: 
	javadoc org.ggf.drmaa -sourcepath ../src/drmaa/ -d ./drmaa_java

clean:
	rm -rf drmaa_java drmaa_c/html drmaa_c/latex
