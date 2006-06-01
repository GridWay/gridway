%.html: %.sgml
	db2html -d gwdoc.dsl#html $<

%.pdf: %.sgml
	db2pdf -d gwdoc.dsl#print $<

images:
	ln -s ../images ./images

all: htmls pdfs


pdfs: images appdeveloperguide.pdf  gwdeveloperguide.pdf \
      installconfguide.pdf  userguide.pdf \
      commandreference.pdf

htmls: images appdeveloperguide.html  \
       gwdeveloperguide.html  \
       installconfguide.html  \
       userguide.html commandreference.html

clean:
	rm -rf userguide appdeveloperguide commandreference gwdeveloperguide installconfguide *.junk *~ *.pdf images
