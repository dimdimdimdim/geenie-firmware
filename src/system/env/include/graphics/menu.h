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

#include <graphics/font.h>
#include <graphics/composite.h>

namespace Graphics
{

class Menu : public Composite
{
	GC_INHERITS(Composite);

protected:
	int scanLine;
	int drawOffsetX;
	int drawWidth;

public:
	Menu(int width);

	virtual void doRelayout();
	virtual void initLineScanning(int scanLine, int drawOffsetX, int drawWidth);
	virtual void processLine(uint8_t *pixelData, int dataOffset);
};

}