/*******************************************************************************
 * Copyright 2015 Dimitri L. <dimdimdimdim at gmx dot fr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************/
#include <base.h>
#include <graphics/text.h>
#include <graphics/font.h>
#include <chars.h>

namespace Graphics
{

Text::Text(int width, int height, const Font &font, int flags, const String &string)
:DisplayItem(width, height)
{
	this->font = &font;
	this->flags = flags;
	this->string = string;
}

Text::Text(int width, int height, const Font &font, const String &string)
:DisplayItem(width, height)
{
	this->font = &font;
	this->flags = 0;
	this->string = string;
}

Text::Text(const Font &font, int flags, const String &string)
:DisplayItem(font.getTextWidth(string), font.getLineHeight())
{
	this->font = &font;
	this->flags = flags;
	this->string = string;
}

Text::Text(const Font &font, const String &string)
:DisplayItem(font.getTextWidth(string), font.getLineHeight())
{
	this->font = &font;
	this->flags = 0;
	this->string = string;
}

void Text::sizeChanged()
{
	textX = 0;
	ellipsisStop = String::Iterator();
	if (isHeightAuto())
		height = font->getLineHeight();
	if (isWidthAuto())
		width = font->getTextWidth(string);
	else {
		if ((flags & (ALIGN_RIGHT | ALIGN_CENTER | ELLIPSIS)) != 0) {
			int w = 0;
			String::Iterator i = string.start();
			int c = -1;
			while (w <= getWidth() && (c = i.getForward()) != -1)
				w += font->getGlyph(Font::getGlyphIndex(c)).getWidth();
			if ((flags & ELLIPSIS) != 0 && w > getWidth()) {
				Glyph ellipsis = font->getCharGlyph(CHAR_ELLIPSIS);
				ellipsisStop = i;
				while (w > width - ellipsis.getWidth() && (c = ellipsisStop.getBackward()) != -1)
					w -= font->getGlyph(Font::getGlyphIndex(c)).getWidth();
				if (c == -1)
					ellipsisStop = string.start();
				w += ellipsis.getWidth();
			}
			if (w < getWidth()) {
				if ((flags & ALIGN_RIGHT) != 0)
					textX = getWidth() - w;
				else if ((flags & ALIGN_CENTER) != 0)
					textX = (getWidth() - w) / 2;
			}
		}
	}
}

void Text::initLineScanning(int scanLine, int drawOffsetX, int drawWidth)
{
	drawOffsetX -= textX;
	int c;
	strIndex = string.start();
	while (drawOffsetX > 0 && strIndex != ellipsisStop && (c = strIndex.getForward()) != -1) {
		int charWidth = font->getGlyph(Font::getGlyphIndex(c)).getWidth();
		if (drawOffsetX >= charWidth) {
			drawOffsetX -= charWidth;
		}
		else {
			strIndex.getBackward();
			break;
		}
	}
	if (drawOffsetX > 0 && strIndex == ellipsisStop) {
		int charWidth = font->getCharGlyph(CHAR_ELLIPSIS).getWidth();
		if (drawOffsetX >= charWidth)
			strIndex = string.end();
	}
	this->drawWidth = drawWidth;
	this->drawOffsetX = drawOffsetX;
	this->scanLine = scanLine;
}

void Text::processLine(uint8_t *pixelData, int dataOffset)
{
	if (scanLine >= font->getLineHeight())
		return;
	int w = drawWidth;
	String::Iterator i = strIndex;
	int ofsX;
	if (drawOffsetX < 0) {
		ofsX = 0;
		dataOffset += -drawOffsetX;
		w -= -drawOffsetX;
	}
	else {
		ofsX = drawOffsetX;
	}
	while (w) {
		if (i == ellipsisStop) {
			font->getCharGlyph(CHAR_ELLIPSIS).drawLine(pixelData, dataOffset, scanLine, ofsX, w);
			break;
		}
		int c = i.getForward();
		if (c == -1)
			break;
		int gw = font->getGlyph(Font::getGlyphIndex(c)).drawLine(pixelData, dataOffset, scanLine, ofsX, w);
		w -= gw;
		dataOffset += gw;
		ofsX = 0;
	}
	scanLine++;
}

}
