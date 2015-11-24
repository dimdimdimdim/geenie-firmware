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
#include <application.h>
#include <environment.h>
#include "stacklayout.h"
#include "objects/cobject.h"

namespace Calc
{

StackLayout::StackLayout(Stack<const Calc::CObject *> *stack, Graphics::TextInput *inputField)
:Graphics::Composite(0, 0)
{
	this->stack = stack;
	firstLevelVisible = 0;
	addItem(inputField);
	font = &Graphics::Font::BIG;
}

void StackLayout::doRelayout()
{
	const List<DisplayItem *> &items = getItems();
	DisplayItem *inputField = items.getAt(0);
	int bottom;
	if (isItemVisible(inputField)) {
		setItemSize(inputField, getWidth(), font->getLineHeight());
		bottom = getHeight() - inputField->getHeight();
		setItemPosition(inputField, 0, bottom);
	}
	else {
		bottom = getHeight();
	}
	int i = 0;
	upperItemNr = 0;
	while (bottom > 0 && i + firstLevelVisible < stack->getLength()) {
		upperItemNr++;
		DisplayItem *item;
		if (items.getLength() <= upperItemNr) {
			item = stack->getAt(i + firstLevelVisible)->getDisplayItem(*font);
			addItem(item);
		}
		else {
			item = items.getAt(upperItemNr);
			if (item == NULL) {
				item = stack->getAt(i + firstLevelVisible)->getDisplayItem(*font);
				setItemAt(upperItemNr, item);
			}
		}
		if (item) {
			int nw = getNumberWidth(upperItemNr + firstLevelVisible);
			setItemSize(item, getWidth() - nw, -1);
			int h = item->getHeight();
			if (h < font->getLineHeight())
				h = font->getLineHeight();
			bottom -= h;
			int w = item->getWidth();
			if (w < getWidth() - nw)
				setItemPosition(item, getWidth() - w, bottom);
			else
				setItemPosition(item, nw, bottom);
		}
		else {
			bottom -= font->getLineHeight();
		}
		i++;
	}
	while (items.getLength() > upperItemNr + 1) {
		removeItemAt(upperItemNr + 1);
	}
	while (bottom > 0) {
		upperItemNr++;
		bottom -= font->getLineHeight();
	}
	upperItemScanStart = -bottom;
}

int StackLayout::getItemHeight(int i)
{
	if (i >= getItems().getLength())
		return font->getLineHeight();
	int h = 0;
	DisplayItem *item = getItems().getAt(i);
	if (item)
		h = item->getHeight();
	if (h < font->getLineHeight())
		h = font->getLineHeight();
	return h;
}

int StackLayout::getNumberWidth(int i)
{
	int w = 0;
	if (i == 0)
		w += font->getCharGlyph('0').getWidth();
	else {
		while (i) {
			w += font->getCharGlyph('0' + i % 10).getWidth();
			i /= 10;
		}
	}
	w += font->getCharGlyph(':').getWidth();
	return w;
}

void StackLayout::drawChar(uint8_t *pixelData, int dataOffset, int c, int &rightPos)
{
	Graphics::Glyph glyph = font->getCharGlyph(c);
	rightPos -= glyph.getWidth();
	int pos = rightPos - drawOffsetX;
	if (pos >= 0)
		glyph.drawLine(pixelData, dataOffset + pos, itemScanLine, 0, drawWidth - pos);
	else
		glyph.drawLine(pixelData, dataOffset, itemScanLine, -pos, drawWidth);
}

void StackLayout::drawNumber(uint8_t *pixelData, int dataOffset, int i)
{
	int p = currentNumberWidth;
	drawChar(pixelData, dataOffset, ':', p);
	if (i == 0)
		drawChar(pixelData, dataOffset, '0', p);
	else {
		while (i) {
			drawChar(pixelData, dataOffset, '0' + i % 10, p);
			i /= 10;
		}
	}
}

void StackLayout::initLineScanning(int scanLine, int drawOffsetX, int drawWidth)
{
	itemNr = upperItemNr;
	itemScanLine = upperItemScanStart + scanLine;
	while (itemScanLine > getItemHeight(itemNr)) {
		itemScanLine -= getItemHeight(itemNr);
		itemNr--;
	}
	this->drawOffsetX = drawOffsetX;
	this->drawWidth = drawWidth;
	currentNumberWidth = getNumberWidth(itemNr + firstLevelVisible);
	DisplayItem *item = NULL;
	if (itemNr < getItems().getLength())
		item = getItems().getAt(itemNr);
	if (item) {
		if (drawOffsetX <= item->getX())
			item->initLineScanning(scanLine - item->getY(), 0, drawWidth - item->getX());
		else
			item->initLineScanning(scanLine - item->getY(), drawOffsetX - item->getX(), drawWidth);
	}
}

void StackLayout::processLine(uint8_t *pixelData, int dataOffset)
{
	if (itemNr < 0)
		return;
	if (itemScanLine >= getItemHeight(itemNr)) {
		itemScanLine = 0;
		if (--itemNr < 0)
			return;
		currentNumberWidth = getNumberWidth(itemNr + firstLevelVisible);
	}
	DisplayItem *item = NULL;
	if (itemNr < getItems().getLength())
		item = getItems().getAt(itemNr);
	if (itemScanLine == 0 && item) {
		if (drawOffsetX <= item->getX())
			item->initLineScanning(0, 0, drawWidth - item->getX());
		else
			item->initLineScanning(0, drawOffsetX - item->getX(), drawWidth);
	}

	if (itemNr > 0)
		drawNumber(pixelData, dataOffset, itemNr + firstLevelVisible);
	if (item != NULL && isItemVisible(item)) {
		if (itemScanLine < item->getHeight()) {
			if (drawOffsetX <= item->getX())
				item->processLine(pixelData, dataOffset + item->getX() - drawOffsetX);
			else
				item->processLine(pixelData, dataOffset);
		}
	}

	itemScanLine++;
}

/*
void StackLayout::reflectPush()
{
	if (items.getLength() > 1)
		detachItem(items.getAt(items.getLength() - 1));
	for (int i = items.getLength() - 1; i > 1; i--)
		items.setAt(i, items.getAt(i - 1));
	if (items.getLength() > 1)
		items.setAt(1, NULL);
}

void StackLayout::reflectPop()
{
	if (items.getLength() > 1)
		detachItem(items.getAt(1));
	for (int i = 1; i < items.getLength() - 1; i++)
		items.setAt(i, items.getAt(i + 1));
	if (items.getLength() > 1)
		items.setAt(items.getLength() - 1, NULL);
}

void StackLayout::reflectSwap()
{
	if (firstLevelVisible >= 2)
		return;
	if (firstLevelVisible == 0 && items.getLength() >= 3) {
		gc<DisplayItem *> i = items.getAt(1);
		items.setAt(1, items.getAt(2));
		items.setAt(2, i);
	}
	else if (items.getLength() > 1) {
		detachItem(items.getAt(1));
		items.setAt(1, NULL);
	}
}
*/
void StackLayout::clearCachedItems()
{
	for (int i = 1; i < items.getLength(); i++) {
		detachItem(items.getAt(i));
		items.setAt(i, NULL);
	}
}

}
