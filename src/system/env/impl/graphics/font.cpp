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
#include <graphics/font.h>

namespace Graphics
{

#include "../../../../res/fontdata.i"

static const Font fontBig(&fontHeader_Big);
static const Font fontMedium(&fontHeader_Medium);
static const Font fontSmall(&fontHeader_Small);

static constexpr Font fontMenu(&fontHeader_Menus);
static const Glyph glyphMenuItem(fontMenu.getGlyph(0));
static const Glyph glyphMenuFolder(fontMenu.getGlyph(1));

const Font &Font::BIG = fontBig;
const Font &Font::MEDIUM = fontMedium;
const Font &Font::SMALL = fontSmall;
const Glyph &Images::menuItem = glyphMenuItem;
const Glyph &Images::menuFolder = glyphMenuFolder;

int Font::getTextWidth(const String &str) const
{
	int c, len = 0;
	String::Iterator i = str.start();
	while ((c = i.getForward()) != -1) {
		int gid = getGlyphIndex(c);
		if (gid != -1)
			len += getGlyph(gid).getWidth();
	}
	return len;
}

int Glyph::drawLine(uint8_t *pixelData, int dataOffset, int y, int glyphOffset, int clipWidth) const
{
	if (font == NULL)
		return 0;

	assert(glyphOffset >= 0);
	// note: glyph data is aligned to the right

	pixelData += dataOffset >> 3;
	dataOffset &= 7;

	int glyphWidth;
	const uint8_t *d = data;
	if (font->isProportional())
		glyphWidth = *(d++);
	else
		glyphWidth = font->getCharWidth();
	if (glyphWidth == 0 || glyphWidth < glyphOffset)
		return 0;
	int n = (glyphWidth + 7) >> 3;
	d += n * (y + 1); // move to the end of the glyph data line (which contains the left data)

	int shift = 15 - ((glyphWidth - 1) & 7) + glyphOffset - dataOffset;
	if (shift >= 16) {
		d -= (shift >> 3) - 1;
		n -= (shift >> 3) - 1;
		shift = 8 + (shift & 7);
	}
	if (n <= 0)
		return 0;
	uint16_t shifted = *(--d) << shift;
	n--;
	if (shift >= 8 && n > 0) {
		shift -= 8;
		shifted |= (*(--d) << shift);
		n--;
	}

	int w = clipWidth;
	uint16_t clip;
	if (clipWidth <= 8)
		clip = 0xFF00 << (8 - clipWidth);
	else
		clip = 0xFFFF << (8 - (clipWidth & 7));
	clip >>= dataOffset;

	*(pixelData++) ^= (uint8_t)((clip >> 8) & (shifted >> 8));
	w -= 8 - dataOffset;
	while (w >= 8 && n > 0) {
		shifted = (*(--d) << shift) | (shifted << 8);
		*(pixelData++) ^= (uint8_t)(shifted >> 8);
		w -= 8;
		n--;
	}
	if (w > 0) {
		if (n > 0)
			shifted |= *(--d) >> (8 - shift);
		if (w >= 8)
			*(pixelData++) ^= (uint8_t)shifted;
		else
			*(pixelData++) ^= (uint8_t)(clip & shifted);
	}

	if (glyphWidth - glyphOffset > clipWidth)
		return clipWidth;
	return glyphWidth - glyphOffset;
}

}
