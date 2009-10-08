<?xml version="1.0"  encoding="iso-8859-1" ?>
<xsl:stylesheet
 xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
 xmlns:fo="http://www.w3.org/1999/XSL/Format"
 version="1.0">
 
 <!-- now replace all these settings with those specific for use with the fo stylesheet (for pdf output) -->
 <!-- just realized both html and fo can share many parameters - need to create common.xsl that gets imported to both so i can 
single source
 those variables -->
                
                <!-- which stylesheet to use? -->
                <xsl:import href="/usr/share/xml/docbook/stylesheet/nwalsh/fo/docbook.xsl"/>
               
               <!-- links to olink database -->
               <xsl:param 
name="target.database.document">olinkdb_fo.xml</xsl:param>
               
               <!-- allow fragment identifiers in pdf? -->
               <xsl:param name="insert.olink.pdf.frag" select="1"></xsl:param>
                
               <!-- needed for olink processing (?)-->
               <xsl:param name="collect.xref.targets">yes</xsl:param>
               
               <!-- making olinks underlined in pdf output -->
               <xsl:attribute-set name="olink.properties">
                              <xsl:attribute name="text-decoration">underline</xsl:attribute>
               </xsl:attribute-set>
               
               <!-- enable extensions -->
               <xsl:param name="use.extensions" select="1"></xsl:param>
               <xsl:param name="xep.extensions" select="1"></xsl:param>
               
               <!-- turn off table column extensions (unless you use xalan or saxon - it's a java thing -->
               <xsl:param name="tablecolumns.extension" select="'0'"></xsl:param>
               
               <!-- should output be in draft mode? -->
               <xsl:param name="draft.mode" select="'yes'"></xsl:param>
               
               <!-- ALIGNMENT -->
               <xsl:param name="alignment">left</xsl:param>
               
               <!-- GRAPHICS -->
               
               <!-- Use graphics in admonitions? like 'warnings' 'important' 'note' etc COMMON -->
                <xsl:param name="admon.graphics">1</xsl:param>
                
               <!-- Set path to admonition graphics  COMMON -->
                <xsl:param name="admon.graphics.path">images/</xsl:param>

                
                <!-- Set path to docbook graphics (testing)
                                <xsl:param name="admon.graphics.path">file:///Z:/testing/alliance/docbook-images/</xsl:param> -->
                
               <!-- Again, if 1 above, what is the filename extension for admon graphics?-->
                <xsl:param name="admon.graphics.extension" select="'.png'"/> 
               
               <!-- for some reason, xep makes the admon graphics too large, this scales them back down -->
               <xsl:template match="*" mode="admon.graphic.width">14pt</xsl:template>
                
               <!-- Set path to callout graphics COMMON
               <xsl:param name="callout.graphics.path">/usr/share/doc/docbook-xsl-doc/doc/images/callouts/</xsl:param> -->
               
               <!-- callouts look fuzzy in print - using the following two parameters to force unicode -->
               <xsl:param name="callout.graphics" select="'0'"></xsl:param>
               
               <xsl:param name="callout.unicode" select="1"></xsl:param>
               
               <!-- NUMBERING -->

               <!-- are parts enumerated?  COMMON -->
                <xsl:param name="part.autolabel">1</xsl:param>
                
               <!-- Are chapters automatically enumerated? COMMON-->
                <xsl:param name="chapter.autolabel">1</xsl:param> 
                
               <!-- Are sections enumerated? COMMON -->
                <xsl:param name="section.autolabel">1</xsl:param>
                
               <!-- how deep should each toc be? (how many levels?) COMMON -->
                <xsl:param name="toc.max.depth">2</xsl:param>
                
               <!-- How deep should recursive sections appear in the TOC? COMMON -->
                <xsl:param name="toc.section.depth">1</xsl:param>
               
               <!-- LINKS -->
               
               <!-- display ulinks as footnotes at bottom of page? -->
               <xsl:param name="ulink.footnotes" select="1"></xsl:param>
               
               <!-- display xref links with underline? -->
               <xsl:attribute-set name="xref.properties">
                              <xsl:attribute name="text-decoration">underline</xsl:attribute> 
               </xsl:attribute-set>
               
               <!-- TABLES -->
               
               <xsl:param name="default.table.width" select="'6in'"></xsl:param>
              

                <!-- INDEX  -->
               
               <!-- do you want an index? COMMON -->
                <xsl:param name="generate.index">1</xsl:param>
               
               <!-- index attributes for xep -->
               <xsl:attribute-set name="xep.index.item.properties">
                              <xsl:attribute name="merge-subsequent-page-numbers">true</xsl:attribute>
                              <xsl:attribute name="link-back">true</xsl:attribute>
               </xsl:attribute-set>
                
                <!-- GLOSSARY  -->
               
               <!-- Display glossentry acronyms? COMMON> -->
                <xsl:param name="glossentry.show.acronym">yes</xsl:param>

                              <!-- Name of the glossary collection file COMMON -->
               <xsl:param 
name="glossary.collection">glossary.xml</xsl:param>
                
               <!-- Generate links from glossterm to glossentry automatically?  COMMON-->
                <xsl:param name="glossterm.auto.link">1</xsl:param>
                
                <!-- if non-zero value for previous parameter, does automatic glossterm linking only apply to firstterms? COMMON
                <xsl:param name="firstterm.only.link">1</xsl:param>-->
               
               <!-- reduce 'indentation' of body text -->
               <xsl:param name="body.start.indent">
                              <xsl:choose>
                                             <xsl:when test="$fop.extensions != 0">0pt</xsl:when>
                                             <xsl:when test="$passivetex.extensions != 0">0pt</xsl:when>
                                             <xsl:otherwise>0pc</xsl:otherwise>
                              </xsl:choose>
               </xsl:param>
                
                 <!-- try to add titleabbrev to part -->
               <xsl:template match="part">
                              <xsl:if test="not(partintro)">
                                             <xsl:apply-templates select="." mode="part.titlepage.mode"/>
                                             <xsl:call-template name="generate.part.toc"/>
                              </xsl:if>
                              <xsl:apply-templates/>
               </xsl:template>
               
               <xsl:template match="part" mode="part.titlepage.mode">
                              <!-- done this way to force the context node to be the part -->
                              <xsl:param name="additional.content"/>
                              
                              <xsl:variable name="id">
                                             <xsl:call-template name="object.id"/>
                              </xsl:variable>
                              
                              <xsl:variable name="titlepage-master-reference">
                                             <xsl:call-template name="select.pagemaster">
                                                            <xsl:with-param name="pageclass" select="'titlepage'"/>
                                             </xsl:call-template>
                              </xsl:variable>
                              
                              <fo:page-sequence hyphenate="{$hyphenate}"
                                             master-reference="{$titlepage-master-reference}">
                                             <xsl:attribute name="language">
                                                            <xsl:call-template name="l10n.language"/>
                                             </xsl:attribute>
                                             <xsl:attribute name="format">
                                                            <xsl:call-template name="page.number.format">
                                                                           <xsl:with-param name="master-reference"
                                                                                          select="$titlepage-master-reference"/>
                                                            </xsl:call-template>
                                             </xsl:attribute>
                                             
                                             <xsl:attribute name="initial-page-number">
                                                            <xsl:call-template name="initial.page.number">
                                                                           <xsl:with-param name="master-reference"
                                                                                          select="$titlepage-master-reference"/>
                                                            </xsl:call-template>
                                             </xsl:attribute>
                                             
                                             <xsl:attribute name="force-page-count">
                                                            <xsl:call-template name="force.page.count">
                                                                           <xsl:with-param name="master-reference"
                                                                                          select="$titlepage-master-reference"/>
                                                            </xsl:call-template>
                                             </xsl:attribute>
                                             
                                             <xsl:attribute name="hyphenation-character">
                                                            <xsl:call-template name="gentext">
                                                                           <xsl:with-param name="key" 
select="'hyphenation-character'"/>
                                                            </xsl:call-template>
                                             </xsl:attribute>
                                             <xsl:attribute name="hyphenation-push-character-count">
                                                            <xsl:call-template name="gentext">
                                                                           <xsl:with-param name="key" 
select="'hyphenation-push-character-count'"/>
                                                            </xsl:call-template>
                                             </xsl:attribute>
                                             <xsl:attribute name="hyphenation-remain-character-count">
                                                            <xsl:call-template name="gentext">
                                                                           <xsl:with-param name="key" 
select="'hyphenation-remain-character-count'"/>
                                                            </xsl:call-template>
                                             </xsl:attribute>
                                             
                                             <xsl:apply-templates select="." mode="running.head.mode">
                                                            <xsl:with-param name="master-reference" 
select="$titlepage-master-reference"/>
                                             </xsl:apply-templates>
                                             
                                             <xsl:apply-templates select="." mode="running.foot.mode">
                                                            <xsl:with-param name="master-reference" 
select="$titlepage-master-reference"/>
                                             </xsl:apply-templates>
                                             
                                             <fo:flow flow-name="xsl-region-body">
                                                            <xsl:call-template name="set.flow.properties">
                                                                           <xsl:with-param name="element" 
select="local-name(.)"/>
                                                                           <xsl:with-param name="master-reference"
                                                                                          select="$titlepage-master-reference"/>
                                                            </xsl:call-template>
                                                            
                                                            <fo:block id="{$id}">
                                                                           <xsl:call-template name="part.titlepage"/>
                                                            </fo:block>
                                                            <xsl:copy-of select="$additional.content"/>
                                             </fo:flow>
                              </fo:page-sequence>
               </xsl:template>
               
               <xsl:template match="part/docinfo|partinfo"></xsl:template>
               <xsl:template match="part/title"></xsl:template>
               <xsl:template match="part/titleabbrev"></xsl:template>
               <xsl:template match="part/subtitle"></xsl:template>
               
                
</xsl:stylesheet>
