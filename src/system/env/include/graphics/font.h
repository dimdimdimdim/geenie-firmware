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

namespace Graphics
{

struct FONT_HEADER
{
	int glyphCount;
	int height;
	int width;
	const int *glyphIndices;
	const uint8_t *glyphData;
};

class Font;

class Glyph
{
protected:
	const Font *font;
	const uint8_t* data;

public:
	constexpr Glyph()												:font(NULL), data(NULL) { }
	constexpr Glyph(const Glyph &glyph)								:font(glyph.font), data(glyph.data) { }
	constexpr Glyph(const Font &font, const uint8_t *data)			:font(&font), data(data) { }
	Glyph &operator=(const Glyph &glyph)							{ this->font = glyph.font; this->data = glyph.data; return *this; }

	constexpr int getHeight() const;
	constexpr int getWidth() const;
	constexpr const Font &getFont() const							{ return *font; }

	int drawLine(uint8_t *pixelData, int dataOffset, int y, int glyphOffset, int clipWidth) const;
};

class Font
{
protected:
	const FONT_HEADER *header;

public:
	static const Font &SMALL;
	static const Font &MEDIUM;
	static const Font &BIG;
	static const int GLYPH_HEXCHARS = 256;
	static const int GLYPH_CURSOR = 262;
	static const int GLYPH_REPLACEMENT = 269;

	static int getGlyphIndex(int unicodeChar);

	constexpr Font()								:header(NULL) { }
	constexpr Font(const Font &font)				:header(font.header) { }
	constexpr Font(const FONT_HEADER *header)		:header(header) { }
	Font &operator=(const Font &font)				{ this->header = font.header; return *this; }

	constexpr bool isProportional() const			{ return header->width == 0; }
	constexpr int getLineHeight() const 			{ return header->height; }
	constexpr int getCharWidth() const				{ return header->width; }
	constexpr int getGlyphCount() const				{ return header->glyphCount; }
	constexpr const FONT_HEADER *getHeader() const	{ return header; }

	constexpr Glyph getGlyph(int index) const;
	Glyph getCharGlyph(int unicodeChar) const		{ return getGlyph(getGlyphIndex(unicodeChar)); }

	int getTextWidth(const String &str) const;
};

inline constexpr int Glyph::getHeight() const
{
	return (font == NULL) ? 0 : font->getLineHeight();
}

inline constexpr int Glyph::getWidth() const
{
	return (font == NULL) ?
		0
	: font->isProportional() ?
		data[0]
	: font->getCharWidth();
}

inline constexpr Glyph Font::getGlyph(int index) const
{
	return (index < 0 || index >= getGlyphCount()) ?
		Glyph()
	: (isProportional()) ?
		Glyph(*this, header->glyphData + header->glyphIndices[index])
	:
		Glyph(*this, header->glyphData + index * (((header->width + 7) >> 3) * header->height));
}

struct Images
{
	static const Glyph &menuItem;
	static const Glyph &menuFolder;
};

}
