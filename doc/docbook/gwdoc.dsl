<!DOCTYPE style-sheet PUBLIC "-//James Clark//DTD DSSSL Style Sheet//EN" [
<!ENTITY html-ss
  PUBLIC "-//Norman Walsh//DOCUMENT DocBook HTML Stylesheet//EN" CDATA dsssl>
<!ENTITY print-ss
  PUBLIC "-//Norman Walsh//DOCUMENT DocBook Print Stylesheet//EN" CDATA dsssl>
]>



<style-specification id="print" use="print-stylesheet">
<STYLE-SPECIFICATION-BODY>
;; ===================================================================
;; Print Parameters
;; Call: jade -d docbook-utils.dsl#print

; === Page layout ===
;; (define %paper-type% "A4")   ;; use A4 paper - comment this out if needed

; === Media objects ===
(define preferred-mediaobject-extensions  ;; this magic allows to use different graphical
   (list "eps"))      ;;   formats for printing and putting online
(define acceptable-mediaobject-extensions
   '())
(define preferred-mediaobject-notations
   (list "EPS"))
(define acceptable-mediaobject-notations
   (list "linespecific"))

; === Rendering ===
(define %head-after-factor% 0.2)  ;; not much whitespace after orderedlist head
(define %paper-sep% 10pt)
(define %block-sep% 3pt)
(define ($paragraph$)     ;; more whitespace after paragraph than before
  (make paragraph
    first-line-start-indent: (if (is-first-para)
                                 %para-indent-firstpara%
                                 %para-indent%)
    space-before: %para-sep%
    space-after: %para-sep%
    quadding: %default-quadding%
    hyphenate?: %hyphenation%
    language: (dsssl-language-code)
    (process-children)))

    </STYLE-SPECIFICATION-BODY>
  </STYLE-SPECIFICATION>

<style-specification id="html" use="html-stylesheet">
<style-specification-body>

;; customize the html stylesheet
(define %stylesheet% "../gwdoc.css")
(define %stylesheet-type% "text/css")
(define %generate-book-toc% #t)
</style-specification-body>
</style-specification>



<external-specification id="print-stylesheet" document="print-ss">
<external-specification id="html-stylesheet"  document="html-ss">
</style-sheet>

