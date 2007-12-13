CWSUBDIRS=admin developer pi qp rn user

TARGETS=
if XMLLINTC
TARGETS+=lint
endif

if DOCHTMLC
if DOCPDFC
TARGETS+=htmls subdirs
else
TARGETS+=html
endif
endif

all: $(TARGETS)
    
%.html: %.xml
	xsltproc \
	--xinclude \
	--stringparam target.database.document "../../olinkdb.xml" \
	--stringparam current.docid "gridway" \
	../../custom_html.xsl \
	index.xml
    
html: index.html subdirs-html
    
htmls: index.html

pdf: 
	for dir in $(CWSUBDIRS); do \
	(cd $$dir; ${MAKE} pdf); \
	done
	
lint:
	xmllint \
	--noout \
	--noent \
	--xinclude \
	--postvalid \
	index.xml


subdirs:
	for dir in $(CWSUBDIRS); do \
	(cd $$dir; ${MAKE} all); \
	done
	
subdirs-html:
	for dir in $(CWSUBDIRS); do \
	(cd $$dir; ${MAKE} html); \
	done

	
clean:
	@rm -f *.pdf;
	@rm -f *.html;
	@for dir in $(CWSUBDIRS); do \
	(cd $$dir; ${MAKE} clean); \
	done
