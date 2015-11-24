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
#include <graphics/menu.h>
#include <menu.h>

namespace Graphics
{

Menu::Menu(int width)
:Composite(width, Images::menuItem.getHeight())
{
}

void Menu::doRelayout()
{

}

void Menu::initLineScanning(int scanLine, int drawOffsetX, int drawWidth)
{
	this->scanLine = scanLine;
	this->drawOffsetX = drawOffsetX;
	this->drawWidth = drawWidth;
}

void Menu::processLine(uint8_t *pixelData, int dataOffset)
{
	int glyphWidth = Images::menuItem.getWidth();
	int itemWidth = glyphWidth + (getWidth() - MENU_PAGE_SIZE * glyphWidth) / (MENU_PAGE_SIZE - 1);
	//int itemIndex = drawOffsetX / itemWidth;
	int x = drawOffsetX % itemWidth;
	int w = drawWidth;
	while (w > 0) {
		Images::menuItem.drawLine(pixelData, dataOffset, scanLine, x, w);
		x = 0;
		dataOffset += itemWidth;
		w -= itemWidth;
	}
	scanLine++;
}

}
