all: lint html pdf
	
images:
	ln -s ../../../images  images
        
%.html: %.xml
	xsltproc \
	--xinclude \
	--stringparam target.database.document "../../../olinkdb.xml" \
	--stringparam current.docid "gridwayUser" \
	../../../custom_html.xsl \
	index.xml

%.pdf: %.xml
	xsltproc --xinclude -o index.fo --stringparam current.docid "gridwayUser" \
	../../../custom_fo.xsl  \
	index.xml;

	xep -quiet -fo index.fo -pdf GWUser.pdf
    
    
html: images index.html
    
pdf: images index.pdf
	
lint:
	xmllint \
	--noout \
	--noent \
	--xinclude \
	--postvalid \
	index.xml
	
clean:
	@rm -f *.pdf;
	@rm -f *.html;  