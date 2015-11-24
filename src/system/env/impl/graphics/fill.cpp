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
#include <graphics/fill.h>

namespace Graphics
{

void Fill::initLineScanning(int scanLine, int drawOffsetX, int drawWidth)
{
	this->drawWidth = drawWidth;
}

void Fill::processLine(uint8_t *pixelData, int dataOffset)
{
	pixelData += dataOffset >> 3;
	dataOffset &= 7;

	int w = drawWidth;
	uint16_t clip;
	if (drawWidth <= 8)
		clip = 0xFF00 << (8 - drawWidth);
	else
		clip = 0xFFFF << (8 - (drawWidth & 7));
	clip >>= dataOffset;

	*(pixelData++) ^= (uint8_t)(clip >> 8);
	w -= 8 - dataOffset;
	while (w >= 8) {
		*(pixelData++) ^= 0xFF;
		w -= 8;
	}
	if (w > 0) {
		*(pixelData++) ^= (uint8_t)clip;
	}
}

}
