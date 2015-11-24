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
#pragma once

#include <util/string.h>
#include <graphics/displayitem.h>
#include <graphics/font.h>

namespace Graphics
{

class Font;

class TextContext
{
public:
	virtual int getGlyphIndex(int uchar) { return Font::getGlyphIndex(uchar); }
	virtual void updateForward(int uchar, const String::AbstractIterator &it) = 0;
	virtual void updateBackward(int uchar, const String::AbstractIterator &it) = 0;
	virtual void reset() = 0;
};

class Text : public DisplayItem
{
	GC_INHERITS(DisplayItem);
	GC_FIELDS(string);

protected:
	String string;
	const Font *font;
	int flags;
	int textX;
	int drawWidth;
	int drawOffsetX;
	String::Iterator ellipsisStop, strIndex;
	int scanLine;

protected:
	virtual void sizeChanged();

public:

	static const int ALIGN_LEFT = 0;
	static const int ALIGN_CENTER = 1;
	static const int ALIGN_RIGHT = 2;
	static const int ELLIPSIS = 4;

	Text(int width, int height, const Font &font, int flags, const String &string);
	Text(int width, int height, const Font &font, const String &string);
	Text(const Font &font, int flags, const String &string);
	Text(const Font &font, const String &string);

	void setText(const String &string) { this->string = string; }

	virtual void initLineScanning(int scanLine, int drawOffsetX, int drawWidth);
	virtual void processLine(uint8_t *pixelData, int dataOffset);
};

}
