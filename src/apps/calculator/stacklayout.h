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

#include <util/list.h>
#include <graphics/composite.h>
#include <graphics/textinput.h>
#include <graphics/font.h>
#include "objects/cobject.h"

namespace Calc
{

class StackLayout : public Graphics::Composite
{
	GC_INHERITS(Graphics::DisplayItem);
	GC_FIELDS(stack);

private:
	Stack<const Calc::CObject *> *stack;
	int firstLevelVisible;
	const Graphics::Font *font;
	int drawOffsetX, drawWidth;
	int itemScanLine, itemNr;
	int upperItemScanStart, upperItemNr;
	int currentNumberWidth;

	int getItemHeight(int i);
	int getNumberWidth(int i);
	void drawChar(uint8_t *pixelData, int dataOffset, int c, int &rightPos);
	void drawNumber(uint8_t *pixelData, int dataOffset, int i);

protected:
	virtual void doRelayout();

public:
	StackLayout(Stack<const Calc::CObject *> *stack, Graphics::TextInput *inputField);

	virtual void initLineScanning(int scanLine, int drawOffsetX, int drawWidth);
	virtual void processLine(uint8_t *pixelData, int dataOffset);

/*	void reflectPush();
	void reflectPop();
	void reflectSwap();*/
	void clearCachedItems();
};

}