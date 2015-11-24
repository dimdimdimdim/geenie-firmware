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
#include <graphics/composite.h>

namespace Graphics
{

void Composite::relayout()
{
	if (withinRelayout)
		return;
	withinRelayout = true;
	doRelayout();
	withinRelayout = false;
	if (isVisible())
		needsFullRedraw = true;
}

void Composite::visibleStateChanged()
{
	for (List<DisplayItem *>::Iterator i = items.start(); i; i++) {
		if (*i != NULL && isItemVisible(*i))
			(*i)->visibleStateChanged();
	}
}

void Composite::addItem(DisplayItem *item)
{
	items.append(item);
	if (item) {
		assert(item->parent == NULL);
		item->parent = this;
		if (isItemVisible(item))
			item->visibleStateChanged();
	}
}

void Composite::insertItemAt(int i, DisplayItem *item)
{
	items.insertAt(i, item);
	if (item) {
		assert(item->parent == NULL);
		item->parent = this;
		if (isItemVisible(item))
			item->visibleStateChanged();
	}
}

void Composite::setItemAt(int i, DisplayItem *item)
{
	gc<DisplayItem *> oldItem = items.getAt(i);
	items.setAt(i, item);
	if (item) {
		assert(item->parent == NULL);
		item->parent = this;
		if (isItemVisible(item))
			item->visibleStateChanged();
	}
	if (oldItem) {
		oldItem->parent = NULL;
		if (isItemVisible(oldItem))
			oldItem->visibleStateChanged();
	}
}

void Composite::removeItem(DisplayItem *item)
{
	assert(item->parent == this);
	item->parent = NULL;
	if (isItemVisible(item))
		item->visibleStateChanged();
	items.remove(item);
}

void Composite::detachItem(DisplayItem *item)
{
	if (item) {
		item->parent = NULL;
		if (isItemVisible(item))
			item->visibleStateChanged();
	}
}

void Composite::removeItemAt(int i)
{
	DisplayItem *item = items.getAt(i);
	if (item) {
		assert(item->parent == this);
		item->parent = NULL;
		if (isItemVisible(item))
			item->visibleStateChanged();
	}
	items.removeAt(i);
}

void Composite::redraw(int fromLine, int height)
{
	if (isVisible()) {
		if (needsFullRedraw) {
			needsFullRedraw = false;
			parent->redraw(y, this->height);
		}
		else {
			parent->redraw(y + fromLine, height);
		}
	}
}

	/*
void Composite::addVisibleItems()
{
	while (nextHiddenItem && (*nextHiddenItem)->y <= scanLine) {
		ItemData *i = *nextHiddenItem;
		if (i->y + i->item->getHeight() > scanLine && i->x < drawOffsetX + drawWidth && i->x + i->item->getWidth() > drawOffsetX) {
			i->nextVisible = visibleItems;
			visibleItems = i;
			i->item->initLineScanning(scanLine - i->y, max(0, drawOffsetX - i->x), min(i->item->getWidth(), getWidth() + drawOffsetX - i->x));
		}
		++nextHiddenItem;
	}
}

void Composite::initLineScanning(int scanLine, int drawOffsetX, int drawWidth)
{
	this->scanLine = scanLine - offsetY;
	this->drawOffsetX = drawOffsetX - offsetY;
	this->drawWidth = drawWidth;
	visibleItems = NULL;
	nextHiddenItem = items.start();
	addVisibleItems();
}

void Composite::processLine(uint8_t *pixelData, int dataOffset)
{
	ItemData **ppi = &visibleItems;
	while (*ppi) {
		ItemData *i = *ppi;
		if (i->y + i->item->getHeight() <= scanLine) {
			// item now outside scanLine, remove it from visible list
			*ppi = i->nextVisible;
		}
		else {
			// process item
			i->item->processLine(pixelData, dataOffset + max(0, i->x - drawOffsetX));
			ppi = &i->nextVisible;
		}
	}
	scanLine++;
	addVisibleItems();
}

void Composite::addItem(DisplayItem *item, int x, int y)
{
	List<ItemData *>::Iterator i = items.start();
	while (i && (*i)->y < y)
		++i;
	items.insertAt(i, gcnew(ItemData(x, y, item)));
}

void Composite::removeItem(DisplayItem *item)
{
	List<ItemData *>::Iterator i = items.start();
	while (i) {
		if ((*i)->item == item) {
			items.removeAt(i);
			break;
		}
		++i;
	}
}

void Composite::moveItem(DisplayItem *item, int x, int y)
{
	List<ItemData *>::Iterator i = items.start();
	while (i) {
		if ((*i)->item == item) {
			gc<ItemData *> data = *i;
			data->x = x;
			// update y and reorder
			if (data->y < y) {
				data->y = y;
				List<ItemData *>::Iterator io = i;
				--i;
				if (i && (*i)->y > y) {
					while (i && (*i)->y > y)
						--i;
					items.removeAt(io);
					if (!i)
						items.insertAt(0, data);
					else
						items.insertAt(++i, data);
				}
			}
			else if (data->y > y) {
				data->y = y;
				List<ItemData *>::Iterator io = i;
				++i;
				if (i && (*i)->y < y) {
					while (i && (*i)->y < y)
						++i;
					items.insertAt(i, data);
					items.removeAt(io);
				}
			}
		}
	}
}
*/

}