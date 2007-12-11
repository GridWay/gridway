export UMASK=002
export XML_CATALOG_FILES=../../../catalog
#export XML_DEBUG_CATALOG= 1
FILENAME= index.xml

all: lint html

html:
	xsltproc \
	--xinclude \
	--stringparam target.database.document "../../../olinkdb.xml" \
	--stringparam current.docid "gridwayRN" \
	../../../custom_html.xsl \
	$(FILENAME)


pdf:
	~bacon/XEP/xep \
	-xml $(FILENAME) \
	-xsl ../../../custom_fo.xsl \
	-out index.pdf

debian-sarge:
	xsltproc --stringparam chunk.section.depth 0 \
	--stringparam suppress.navigation 1 \
	--stringparam  chapter.autolabel 0 \
	--stringparam  section.autolabel 1 \
	--stringparam toc.section.depth 0\
           /usr/share/xml/docbook/stylesheet/nwalsh/html/chunk.xsl \
           $(FILENAME)

mac:
	xsltproc --stringparam chunk.section.depth 0 \
	--stringparam chapter.autolabel 0 \
	/sw/share/xml/xsl/docbook-xsl/html/chunk.xsl \
	$(FILENAME)
	
lint:
	xmllint \
	--noout \
	--noent \
	--xinclude \
	--postvalid \
	$(FILENAME)

olink:
	xsltproc \
	--xinclude \
	--stringparam  collect.xref.targets  "only"  \
	../../../custom_html.xsl \
	$(FILENAME)
