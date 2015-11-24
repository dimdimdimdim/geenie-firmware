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
#include <graphics/displayitem.h>
#include <graphics/composite.h>

namespace Graphics
{

DisplayItem::DisplayItem(int width, int height, bool visible)
{
	parent = NULL;
	x = 0;
	y = 0;
	this->width = width < 0 ? 0 : width;
	this->height = height < 0 ? 0 : height;
	this->flags = (visible ? VISIBLE : 0) |
	              (width == -1 ? AUTO_WIDTH : 0) |
				  (height == -1 ? AUTO_HEIGHT : 0);
}

bool DisplayItem::updateSize(int width, int height)
{
	if (width != this->width || height != this->height) {
		this->width = width < 0 ? 0 : width;
		this->height = height < 0 ? 0 : height;
		this->flags = (flags & ~(AUTO_WIDTH | AUTO_HEIGHT)) |
					  (width == -1 ? AUTO_WIDTH : 0) |
					  (height == -1 ? AUTO_HEIGHT : 0);
		sizeChanged();
		return true;
	}
	return false;
}

void DisplayItem::setSize(int width, int height)
{
	if (updateSize(width, height))
		if (parent)
			parent->relayout();
}

void DisplayItem::setVisible(bool visible)
{
	if (visible != ((flags & VISIBLE) != 0)) {
		flags = (flags & ~VISIBLE) | (visible ? VISIBLE : 0);
		if (parent && parent->isVisible())
			visibleStateChanged();
		if (parent)
			parent->relayout();
		if (visible)
			redraw();
		else
			parent->redraw();
	}
}

bool DisplayItem::isVisible()
{
	return ((flags & VISIBLE) != 0) && parent != NULL && parent->isVisible();
}

void DisplayItem::redraw(int fromLine, int height)
{
	if (isVisible())
		parent->redraw(y + fromLine, height);
}

}
