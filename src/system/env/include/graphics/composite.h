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
#include <graphics/displayitem.h>

namespace Graphics
{

class Composite : public DisplayItem
{
	GC_INHERITS(DisplayItem);
	GC_FIELDS(items);

private:
	bool withinRelayout, needsFullRedraw;

protected:
	void setItemPosition(DisplayItem *item, int x, int y)			{ assert(item->parent == this); item->x = x; item->y = y; }
	void setItemSize(DisplayItem *item, int width, int height)		{ assert(item->parent == this); item->updateSize(width, height); }
	bool isItemVisible(DisplayItem *item)							{ /*assert(item->parent == this);*/ return (item->flags & VISIBLE) != 0; }

	virtual void sizeChanged()										{ relayout(); }
	virtual void visibleStateChanged();
	virtual void doRelayout() = 0;

	List<DisplayItem *> items; // better use the below methods
	void detachItem(DisplayItem *item); // also don't use this inappropriately, it doesn't remove the item from the list

	void addItem(DisplayItem *item);
	void insertItemAt(int i, DisplayItem *item);
	void setItemAt(int i, DisplayItem *item);
	void removeItem(DisplayItem *item);
	void removeItemAt(int i);
	const List<DisplayItem *> &getItems()							{ return items; }

public:
	Composite(int width = -1, int height = -1, bool visible = true) :DisplayItem(width, height, visible) { withinRelayout = false; needsFullRedraw = false; }

	virtual void redraw(int fromLine, int height);
	void redraw()													{ redraw(0, height); }
	virtual void relayout();
};

	/* 
class Composite : public DisplayItem
{
	GC_INHERITS(DisplayItem);
	GC_FIELDS(items);

private:

	// note: to speed up drawing, items are ordered by their y location
	class ItemData : public Object
	{
		GC_INHERITS(Object);
		GC_FIELDS(item);
	public:
		int x, y;
		ItemData *nextVisible;
		DisplayItem *item;
		ItemData() { item = NULL; }
		ItemData(int x, int y, DisplayItem *item) { this->x = x; this->y = y; this->item = item; }
	};

	int offsetX, offsetY;
	List<ItemData *> items;
	int scanLine, drawOffsetX, drawWidth;
	ItemData *visibleItems;
	List<ItemData *>::Iterator nextHiddenItem;

	void addVisibleItems();

public:
	Composite(int width, int height) :DisplayItem(width, height) { offsetX = 0; offsetY = 0; }

	virtual void initLineScanning(int scanLine, int drawOffsetX, int drawWidth);
	virtual void processLine(uint8_t *pixelData, int dataOffset);

	void addItem(DisplayItem *item, int x, int y);
	void removeItem(DisplayItem *item);
	void moveItem(DisplayItem *item, int x, int y);
};
*/

}
