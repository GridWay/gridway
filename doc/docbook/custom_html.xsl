<?xml version="1.0"  encoding="iso-8859-1" ?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                version="1.0">
                
                <!-- Which DocBook standard xsl file should we use as the default? -->
                <xsl:import href="http://docbook.sourceforge.net/release/xsl/current/html/chunk.xsl"/>
                
                <!-- testing: if you want to generate your own html without installing stylesheets, substitute the following url
                for the import href above:
                http://docbook.sourceforge.net/release/xsl/current/html/chunk.xsl
                -->

                <!-- speed up the chunking process? -->
                <xsl:param name="chunk.fast">1</xsl:param>
                
                <!-- which css stylesheet to use?
                <xsl:param name="html.stylesheet" select="'/toolkit/css/default.css'"></xsl:param> -->
                
                <!-- Use graphics in admonitions? like 'warnings' 'important' 'note' etc -->
                <xsl:param name="admon.graphics">1</xsl:param>
                
                <!-- Set path to admonition graphics-->
                <xsl:param name="admon.graphics.path">/docbook-images/</xsl:param>
                
                
                <!-- Set path to docbook graphics (testing)
                <xsl:param name="admon.graphics.path">file:///Z:/testing/alliance/docbook-images/</xsl:param> -->
                
                <!-- Again, if 1 above, what is the filename extension for admon graphics? -->
                <xsl:param name="admon.graphics.extension" select="'.gif'"/>
                
                <!-- Set path to callout graphics -->
                <xsl:param name="callout.graphics.path">/docbook-images/callouts/</xsl:param>
                
                <!-- Depth to which sections should be chunked -->
                <xsl:param name="chunk.section.depth">0</xsl:param>
                
                <!-- Are parts automatically enumerated? -->
                <xsl:param name="part.autolabel">0</xsl:param>
                
                <!-- Are chapters automatically enumerated? -->
                <xsl:param name="chapter.autolabel">0</xsl:param>
                
                <!-- Are sections enumerated? -->
                <xsl:param name="section.autolabel">1</xsl:param>
                
                <!-- how deep should each toc be? (how many levels?) -->
                <xsl:param name="toc.max.depth">2</xsl:param>
                
                <!-- How deep should recursive sections appear in the TOC for chapters? -->
                 <xsl:param name="toc.section.depth">1</xsl:param>
                
                <!-- Should the first section be chunked separately from its parent? > 0 = yes-->
                <xsl:param name="chunk.first.sections">1</xsl:param>
                
                <!-- Instead of using default filenames, use ids for filenames (dbhtml directives take precedence) -->
                <xsl:param name="use.id.as.filename">1</xsl:param>
                
                <!-- custom toc - book only shows chapter -->
                <xsl:template match="preface|chapter|reference|appendix|article" mode="toc">
                                <xsl:param name="toc-context" select="."/>
                                
                                <xsl:choose>
                                                <xsl:when test="local-name($toc-context) = 'book'">
                                                                <xsl:call-template name="subtoc">
                                                                                <xsl:with-param name="toc-context" select="$toc-context"/>
                                                                                <xsl:with-param name="nodes" select="foo"/>
                                                                </xsl:call-template>
                                                </xsl:when>
                                                <xsl:otherwise>
                                                                <xsl:call-template name="subtoc">
                                                                                <xsl:with-param name="toc-context" select="$toc-context"/>
                                                                                <xsl:with-param name="nodes"
                                                                                                select="section|sect1|glossary|bibliography|index
                                                                                                |bridgehead[$bridgehead.in.toc != 0]"/>
                                                                </xsl:call-template>
                                                </xsl:otherwise>
                                </xsl:choose>
                </xsl:template>
                
                <!-- control TOCs -->
                <xsl:param name="generate.toc">
                                appendix  toc,title
                                article/appendix  nop
                                article   toc,title
                                book      toc,title
                                chapter   toc,title
                                part      toc,title
                                preface   toc,title
                                qandadiv  toc
                                qandaset  toc
                                reference toc,title
                                sect1     toc
                                sect2     toc
                                sect3     toc
                                sect4     toc
                                sect5     toc
                                section   toc
                                set       toc,title
                </xsl:param>
                

                <!-- INDEX PARAMETERS -->
                <!-- do you want an index?  -->
                <xsl:param name="generate.index">1</xsl:param>
               
                <!-- Select indexterms based on type attribute value -->
                <xsl:param name="index.on.type">1</xsl:param>
                
                <!-- GLOSSARY PARAMETERS -->
                <!-- Display glossentry acronyms? -->
                <xsl:param name="glossentry.show.acronym">yes</xsl:param>
                
                <!-- Name of the glossary collection file -->
                <xsl:param name="glossary.collection" select="'http://root/glossary.xml'"></xsl:param>
                
                <!-- Generate links from glossterm to glossentry automatically? -->
                <xsl:param name="glossterm.auto.link">1</xsl:param>
                
                <!-- if non-zero value for previous parameter, does automatic glossterm linking only apply to firstterms?
                <xsl:param name="firstterm.only.link">1</xsl:param> -->
                
                <!-- permit wrapping of long lines of code
                <xsl:attribute-set name="monospace.verbatim.properties" 
                                use-attribute-sets="verbatim.properties monospace.properties">
                                <xsl:attribute name="wrap-option">wrap</xsl:attribute>
                </xsl:attribute-set> -->
                
                <!-- INCORPORATING DOCBOOK PAGES INTO WEBSITE -->

                <!-- make sure there's a DOCTYPE in the html output (otherwise, some css renders strangely -->
                <xsl:param name="chunker.output.doctype-public" select="'-//W3C//DTD HTML 4.01 Transitional//EN'"/>
                <xsl:param name="chunker.output.doctype-system" select="'http://www.w3.org/TR/html4/loose.dtd'"/>
                
                <!-- add elements to the HEAD tag -->
                <!-- the following template is for the conditional comments for detecting certain browsers -->
                <xsl:template name="conditionalComment">
                                <xsl:param name="qualifier" select="'IE 7'"/>
                                <xsl:param name="contentRTF" select="''" />
                                <xsl:comment>[if <xsl:value-of select="$qualifier"/>]<![CDATA[>]]>
                                                <xsl:copy-of select="$contentRTF" />
                                                <![CDATA[<![endif]]]></xsl:comment>
                </xsl:template>
                
                <xsl:template name="user.head.content">
                                <link href="http://www.globus.org/toolkit/css/default.css" rel="stylesheet" type="text/css" /> 
                                <link rel="stylesheet" type="text/css" href="/toolkit/css/print.css" media="print" />

                                <xsl:comment> calling in special style sheet if detected browser is IE 7 </xsl:comment>

                                <xsl:call-template name="conditionalComment">
                                                <xsl:with-param name="qualifier" select="'IE 7'"/>
                                                <xsl:with-param name="contentRTF">
                                                                &lt;link rel="stylesheet" type="text/css" href="/toolkit/css/ie7.css" /&gt;
                                                </xsl:with-param>
                                </xsl:call-template>                                

                                <link rel="alternate" title="Globus Toolkit RSS" href="/toolkit/rss/downloadNews/downloadNews.xml" type="application/rss+xml"/>
                                <script>
                                                <xsl:comment>
                                                function GlobusSubmit()
                                                {
                                                var f=document.GlobusSearchForm;
                                                
                                                f.action="http://www.google.com/custom";
                                                if (f.elements[0].checked) {
                                                f.q.value = f.qinit.value + " -inurl:mail_archive " ;
                                                } else {
                                                f.q.value = f.qinit.value + " inurl:mail_archive " ;
                                                }
                                                }
                                                </xsl:comment>
                                </script>
                </xsl:template>
                
                <!-- add an attribute to the BODY tag -->
                <xsl:template name="body.attributes">
                                <xsl:attribute name="class">section-3</xsl:attribute>
                </xsl:template>
                
                <!-- pull in 'website' with this code by modifying chunk-element-content from html/chunk-common.xsl-->
                <xsl:template name="chunk-element-content">
                                <xsl:param name="prev"/>
                                <xsl:param name="next"/>
                                <xsl:param name="nav.context"/>
                                <xsl:param name="content">
                                                <xsl:apply-imports/>
                                </xsl:param>
                                
                                <xsl:call-template name="user.preroot"/>
                                
                                <html>
                                                <xsl:call-template name="html.head">
                                                                <xsl:with-param name="prev" select="$prev"/>
                                                                <xsl:with-param name="next" select="$next"/>
                                                </xsl:call-template>
                                                
                                                <body>
                                                                <xsl:call-template name="body.attributes"/>
                                                                <xsl:processing-instruction name="php">
                                                                                include_once("http://www.globus.org/toolkit/docs/development/4.2-drafts/includes/globus_header_docbook.inc");
                                                                                ?</xsl:processing-instruction>
                                                                
 
                                                                <xsl:call-template name="user.header.navigation"/>
                                                                
                                                                <xsl:call-template name="header.navigation">
                                                                                <xsl:with-param name="prev" select="$prev"/>
                                                                                <xsl:with-param name="next" select="$next"/>
                                                                                <xsl:with-param name="nav.context" select="$nav.context"/>
                                                                </xsl:call-template>
                                                                
                                                                <xsl:call-template name="user.header.content"/>
                                                                
                                                                
                                                               

                                                                
                                                                <xsl:copy-of select="$content"/>
                                                                
                                                                <xsl:call-template name="user.footer.content"/>
                                                                
                                                                <xsl:call-template name="footer.navigation">
                                                                                <xsl:with-param name="prev" select="$prev"/>
                                                                                <xsl:with-param name="next" select="$next"/>
                                                                                <xsl:with-param name="nav.context" select="$nav.context"/>
                                                                </xsl:call-template>
                                                                
                                                                <xsl:call-template name="user.footer.navigation"/>

                                                                <xsl:processing-instruction name="php">
                                                                                include_once("http://www.globus.org/toolkit/docs/development/4.2-drafts/includes/globus_footer_docbook.inc");
                                                                                ?</xsl:processing-instruction>
                                                      
                                                </body>
                                </html>
                </xsl:template>
                
                <!-- prevent h1 and h2 using clear: both - want to control in css, instead -->
                <xsl:template name="section.heading">
                                <xsl:param name="section" select="."/>
                                <xsl:param name="level" select="'1'"/>
                                <xsl:param name="title"/>
                                <xsl:element name="h{$level+1}">
                                                <xsl:attribute name="class">title</xsl:attribute>
                                                <a>
                                                                <xsl:attribute name="name">
                                                                                <xsl:call-template name="object.id">
                                                                                                <xsl:with-param name="object" select="$section"/>
                                                                                </xsl:call-template>
                                                                </xsl:attribute>
                                                </a>
                                                <xsl:copy-of select="$title"/>
                                </xsl:element>
                </xsl:template>
                
                <!-- taking out top table row of Navigational Header -->
                
                <xsl:template name="header.navigation">
                                <xsl:param name="prev" select="/foo"/>
                                <xsl:param name="next" select="/foo"/>
                                <xsl:param name="nav.context"/>
                                
                                <xsl:variable name="home" select="/*[1]"/>
                                <xsl:variable name="up" select="parent::*"/>
                                
                                <xsl:variable name="row1" select="$navig.showtitles != 0"/>
                                <xsl:variable name="row2" select="count($prev) &gt; 0
                                                or (count($up) &gt; 0
                                                and generate-id($up) != generate-id($home)
                                                and $navig.showtitles != 0)
                                                or count($next) &gt; 0"/>
                                
                                <xsl:if test="$suppress.navigation = '0' and $suppress.header.navigation = '0'
                                                ">
                                                <div class="navheader">
                                                                <xsl:if test="$row1 or $row2">
                                                                                <table width="100%" summary="Navigation header">
                                                                                                
                                                                                                <xsl:if test="$row1">
                                                                                                                <!-- 
                                                                                                                <tr>
                                                                                                                <th colspan="3" align="center">
                                                                                                                <xsl:apply-templates select="." mode="object.title.markup"/>
                                                                                                                </th>
                                                                                                                </tr>
                                                                                                                -->
                                                                                                </xsl:if>
                                                                                                
                                                                                                <xsl:if test="$row2">
                                                                                                                <tr>
                                                                                                                                <td width="20%" align="left">
                                                                                                                                                <xsl:if test="count($prev)>0">
                                                                                                                                                                <a accesskey="p">
                                                                                                                                                                                <xsl:attribute name="href">
                                                                                                                                                                                                <xsl:call-template name="href.target">
                                                                                                                                                                                                                <xsl:with-param name="object" select="$prev"/>
                                                                                                                                                                                                </xsl:call-template>
                                                                                                                                                                                </xsl:attribute>
                                                                                                                                                                                <xsl:call-template name="navig.content">
                                                                                                                                                                                                <xsl:with-param name="direction" select="'prev'"/>
                                                                                                                                                                                </xsl:call-template>
                                                                                                                                                                </a>
                                                                                                                                                </xsl:if>
                                                                                                                                                <xsl:text>&#160;</xsl:text>
                                                                                                                                </td>
                                                                                                                                <th width="60%" align="center">
                                                                                                                                                <xsl:choose>
                                                                                                                                                                <xsl:when test="count($up) > 0
                                                                                                                                                                                and generate-id($up) != generate-id($home)
                                                                                                                                                                                and $navig.showtitles != 0">
                                                                                                                                                                                <xsl:apply-templates select="$up" mode="object.title.markup"
                                                                                                                                                                                                />
                                                                                                                                                                </xsl:when>
                                                                                                                                                                <xsl:otherwise>&#160;</xsl:otherwise>
                                                                                                                                                </xsl:choose>
                                                                                                                                </th>
                                                                                                                                <td width="20%" align="right">
                                                                                                                                                <xsl:text>&#160;</xsl:text>
                                                                                                                                                <xsl:if test="count($next)>0">
                                                                                                                                                                <a accesskey="n">
                                                                                                                                                                                <xsl:attribute name="href">
                                                                                                                                                                                                <xsl:call-template name="href.target">
                                                                                                                                                                                                                <xsl:with-param name="object" select="$next"/>
                                                                                                                                                                                                </xsl:call-template>
                                                                                                                                                                                </xsl:attribute>
                                                                                                                                                                                <xsl:call-template name="navig.content">
                                                                                                                                                                                                <xsl:with-param name="direction" select="'next'"/>
                                                                                                                                                                                </xsl:call-template>
                                                                                                                                                                </a>
                                                                                                                                                </xsl:if>
                                                                                                                                </td>
                                                                                                                </tr>
                                                                                                </xsl:if>
                                                                                </table>
                                                                </xsl:if>
                                                                <xsl:if test="$header.rule != 0">
                                                                                <hr/>
                                                                </xsl:if>
                                                </div>
                                </xsl:if>
                </xsl:template>
                
                

</xsl:stylesheet>
