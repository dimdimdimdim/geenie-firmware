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

#include <io/device.h>
#include <events.h>

#define DISPLAY_INVERT	false

namespace Graphics
{
class DisplayItem;
}

class Display : public Object
{
	GC_INHERITS(Object);
	GC_FIELDS(scanLine, device);

private:
	IO::Device *device;
	int width, height, lineBytes;
	uint8_t *scanLine;

	Display(IO::Device *device, int width, int height, int lineBytes);

public:
	static gc<Display *> open(bool exclusive = false);
	void close()		{ device->close(); }

	int getHeight()		{ return height; }
	int getWidth()		{ return width; }

	void draw(Graphics::DisplayItem *contents, int top, int height);
	void draw(Graphics::DisplayItem *contents) { draw(contents, 0, height); }
};

class RedrawEvent : public Event
{
	GC_INHERITS(Object);
	GC_NO_FIELDS;
private:
	int top;
	int height;
public:
	RedrawEvent(int top, int height) { this->top = top; this->height = height; }
	virtual void handle();
};

