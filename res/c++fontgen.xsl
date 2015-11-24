<?xml version="1.0" encoding="ISO-8859-1"?>

<xsl:stylesheet
	version="1.0"
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
	xmlns:xs="http://www.w3.org/2001/XMLSchema">
	<xsl:output method="text" encoding="ISO-8859-1"/>
	<xsl:strip-space elements="*"/>

	<xsl:template match="/">
		 <xsl:apply-templates select="//project"/>
		 <xsl:apply-templates select="//project/group"/>
	</xsl:template>

	<!-- Generate Preambel -->
	<xsl:template match="project">
		<xsl:text>//--------------------------------------------------------------&#xa;</xsl:text>
		<xsl:text>//               Font and Bitmap Generator                      &#xa;</xsl:text>
		<xsl:text>//                                                              &#xa;</xsl:text>
		<xsl:text>// 					   www.muGUI.de                           &#xa;</xsl:text> 
		<xsl:text>//                                                              &#xa;</xsl:text>
		<xsl:text>// Version:   </xsl:text>
		<xsl:value-of select="version"/> 
		<xsl:text>&#xa;</xsl:text>
		<xsl:text>// Project Name:  </xsl:text>
		<xsl:value-of select="projectname"/> 
		<xsl:text>&#xa;</xsl:text>	
		<xsl:text>//--------------------------------------------------------------&#xa;</xsl:text>
		<xsl:text>// Generated using c++fontgen.xsl,                              &#xa;</xsl:text>
		<xsl:text>//--------------------------------------------------------------&#xa;</xsl:text>
		<xsl:text>&#xa;</xsl:text>
	</xsl:template> 


	<xsl:template match="/project/group">
		<xsl:variable name="StopCharacter">
			<xsl:value-of select="startcharacter + numberofbitmaps -1"/>
		</xsl:variable>
		<xsl:variable name="Bytesperbitmap">
			<xsl:value-of select="bytesperbitmapline"/>
		</xsl:variable>
		<xsl:variable name="Width">
			<xsl:value-of select="width"/>
		</xsl:variable>
		<xsl:variable name="Index">0</xsl:variable>


		<xsl:text>//--------------------------------------------------------------&#xa;</xsl:text>
		<xsl:text>// Group Name: </xsl:text>
		<xsl:value-of select="groupname"/>
		<xsl:text>&#xa;</xsl:text>
		<xsl:text>// Designer: </xsl:text>
		<xsl:value-of select="designer"/>
		<xsl:text>&#xa;</xsl:text>
		<xsl:text>// Date: </xsl:text>
		<xsl:value-of select="date"/>
		<xsl:text>&#xa;</xsl:text>
		<xsl:text>// Description: </xsl:text>
		<xsl:value-of select="description"/>
		<xsl:text>&#xa;</xsl:text>
		<xsl:text>// Byteorientation: </xsl:text>
		<xsl:value-of select="byteorientation"/>
		<xsl:text>&#xa;</xsl:text>
		<xsl:text>// Font Height: </xsl:text>
		<xsl:value-of select="height"/>
		<xsl:text> Pixel&#xa;</xsl:text>
		<xsl:if test="contains(proportional,'True')">
			<xsl:text>// Proportional font &#xa;</xsl:text>
			<xsl:text>// Font Width: Individual &#xa;</xsl:text>
		</xsl:if>
		<xsl:if test="contains(proportional,'False')">
			<xsl:text>// Monospace font &#xa;</xsl:text>
			<xsl:text>// Font Width: </xsl:text>
			<xsl:value-of select="width"/>
			<xsl:text> Pixel&#xa;</xsl:text>
		</xsl:if>
		<xsl:text>// Number of Bitmaps: </xsl:text>
		<xsl:value-of select="numberofbitmaps"/>
		<xsl:text>&#xa;</xsl:text>
		<xsl:if test="contains(proportional,'False')">
			<xsl:text>// Bytes per Bitmap: </xsl:text>
			<xsl:value-of select="bytesperbitmap"/>
			<xsl:text>&#xa;</xsl:text>
		</xsl:if>
		<xsl:text>// Start Character: </xsl:text>
		<xsl:value-of select="startcharacter"/>
		<xsl:text>&#xa;</xsl:text>
		<xsl:text>// Stop Character: </xsl:text>
		<xsl:value-of select="$StopCharacter"/>
		<xsl:text>&#xa;</xsl:text>
		<xsl:text>//---------------------------------------------------------------&#xa;</xsl:text>
		
		<xsl:if test="contains(proportional,'True')">
		
			<!-- Make index -->
			<xsl:text>static const int glyphIndices_</xsl:text>
			<xsl:value-of select="groupname"/>
			<xsl:text>[] = &#xa;{ // Glyph indices: </xsl:text>
				<xsl:value-of  select="count(bitmap)"/>
			<xsl:text> entries&#xa;</xsl:text>
			<xsl:call-template name="makeIndex"/>
			<xsl:text>};&#xa;&#xa;</xsl:text>
			
			<!-- Make bitmaps for proportional fonts-->
			<xsl:text>static const uint8_t glyphData_</xsl:text>
			<xsl:value-of select="groupname"/>
			<xsl:text>[] = &#xa;{&#xa;</xsl:text>
			<xsl:for-each select="bitmap">
				<xsl:text>&#9;// Glyph nr </xsl:text>
				<xsl:value-of select="position()-1"/>
				<xsl:text>&#xa;&#9;</xsl:text>
				<xsl:value-of select="bitmapwidth"/>
				<xsl:text> /* glyph width */, </xsl:text>
				<xsl:for-each select="byte">
					<xsl:value-of select="."/>
					<xsl:if test="position()!=last()">
						<xsl:text>,</xsl:text>
					</xsl:if>
				</xsl:for-each>
				<xsl:if test="position()!=last()">
					<xsl:text>,&#xa;</xsl:text>
				</xsl:if>
			</xsl:for-each>
			
		</xsl:if>
		<xsl:if test="contains(proportional,'False')">
		
			<!-- Make bitmaps for monospace fonts-->
			<xsl:text>static const uint8_t glyphData_</xsl:text>
			<xsl:value-of select="groupname"/>
			<xsl:text>[] = &#xa;{&#xa;</xsl:text>
			<xsl:for-each select="bitmap">
				<xsl:text>&#9;// Glyph nr </xsl:text>
				<xsl:value-of select="position()-1"/>
				<xsl:text>&#xa;&#9;</xsl:text>
				<xsl:for-each select="byte">
					<xsl:value-of select="."/>
					<xsl:if test="position()!=last()">
						<xsl:text>,</xsl:text>
					</xsl:if>
				</xsl:for-each>
				<xsl:if test="position()!=last()">
					<xsl:text>,&#xa;</xsl:text>
				</xsl:if>
			</xsl:for-each>
			
		</xsl:if>
		<xsl:text>&#xa;};&#xa;&#xa;</xsl:text>
			
		<!-- Make Header -->
		<xsl:text>static const FONT_HEADER fontHeader_</xsl:text>
		<xsl:value-of select="groupname"/>
		<xsl:text> = &#xa;{&#xa;</xsl:text>
		<xsl:text>&#9;// Font Header &#xa;</xsl:text>
		
		<xsl:text>&#9;</xsl:text>
		<xsl:value-of select="numberofbitmaps"/>
		<xsl:text>, // Number of glyphs&#xa;</xsl:text>
		
		<xsl:text>&#9;</xsl:text>
		<xsl:value-of select="height"/>
		<xsl:text>, // Height &#xa;</xsl:text>
		
		<xsl:if test="contains(proportional,'False')">
			<xsl:text>&#9;</xsl:text>
			<xsl:value-of select="width"/>
			<xsl:text>, // Width&#xa;</xsl:text>
			<xsl:text>&#9;NULL, // No indices&#xa;</xsl:text>
		</xsl:if>
		<xsl:if test="contains(proportional,'True')">
			<xsl:text>&#9;0, // Proportional width&#xa;</xsl:text>
			<xsl:text>&#9;glyphIndices_</xsl:text>
			<xsl:value-of select="groupname"/>
			<xsl:text>, // Glyph indices&#xa;</xsl:text>
		</xsl:if>
		<xsl:text>&#9;glyphData_</xsl:text>
		<xsl:value-of select="groupname"/>
		<xsl:text> // Glyph data&#xa;</xsl:text>
		<xsl:text>};&#xa;&#xa;</xsl:text>
	</xsl:template>

	
	<!-- Convert Dezimal to Hex --> 
	<xsl:variable name="hexDigits" select="'0123456789ABCDEF'" />
	<xsl:template name="toHex">
		<xsl:param name="decimalNumber" />
     	<xsl:if test="$decimalNumber >= 16">
			<xsl:call-template name="toHex">
				<xsl:with-param name="decimalNumber" select="floor($decimalNumber div 16)" />
			</xsl:call-template>
		</xsl:if>
		<xsl:value-of select="substring($hexDigits, ($decimalNumber mod 16) + 1, 1)" />
	</xsl:template>


	<!-- Make the index with start address of the bitmaps -->
	<xsl:template name="makeIndex">
		<xsl:for-each select="bitmap">
			<xsl:variable name="laenge">
				<xsl:call-template name="recursivLength">
					<xsl:with-param name="n">
						<xsl:value-of select="position()"/>
					</xsl:with-param>
				</xsl:call-template>
			</xsl:variable>
			<!-- length of data bytes, Index length, Header (5 Bytes but starting with 0 ->2)  -->
			<xsl:text>&#9;</xsl:text>
			<xsl:value-of select="$laenge"/>
			<xsl:if test="position()!=last()">
				<xsl:text>,</xsl:text>
			</xsl:if>
			<xsl:text> // Glyph nr </xsl:text>
				<xsl:value-of select="position()-1"/>
			<xsl:text>&#xa;</xsl:text>
		</xsl:for-each>
	</xsl:template>

	<!-- Calculate the bitmap bytes from bitmap 1 to bitmap n -->
	<!-- Remember: XSLT starts indexing with 1 and not with 0 -->
	<xsl:template name="recursivLength">
	   <xsl:param name="n"/>
	   <xsl:if test="$n > 1"> 
			<xsl:variable name="Width" >
				<xsl:call-template name="recursivLength">
					<xsl:with-param name="n" select="$n - 1"/>
				</xsl:call-template>
			</xsl:variable> 
			<xsl:variable name="BytesPerBitmap">
				<xsl:value-of select="../bitmap[$n]/bytesperbitmap" />
			</xsl:variable>
			<xsl:value-of select="$Width + $BytesPerBitmap + 1" />
	   </xsl:if>
	   <xsl:if test="$n = 1"> 
			<xsl:variable name="Width" >0</xsl:variable> 
			<xsl:value-of select="$Width" />
	   </xsl:if>
	</xsl:template>


 </xsl:stylesheet>