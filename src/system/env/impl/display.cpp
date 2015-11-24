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
#include <display.h>
#include <application.h>
#include <graphics/displayitem.h>

gc<Display *> Display::open(bool exclusive)
{
	gc<IO::Device *> device = IO::Device::open("display", exclusive ? IO::EXCLUSIVE : 0);
	int width;
	int height;
	int lineBytes;
	try {
		width = device->control(IO::Ctl::GET_WIDTH);
		height = device->control(IO::Ctl::GET_HEIGHT);
		lineBytes = device->control(IO::Ctl::GET_LINE_BYTES);
	}
	catch (...) {
		device->close();
		throw;
	}
	return gcnew(Display(device, width, height, lineBytes));
}

Display::Display(IO::Device *device, int width, int height, int lineBytes)
{
	this->device = device;
	this->width = width;
	this->height = height;
	this->lineBytes = lineBytes;
	scanLine = gcnew(uint8_t[lineBytes]);
}

void Display::draw(Graphics::DisplayItem *contents, int top, int height)
{
	if (top < 0)
		top = 0;
	if (top < contents->getY()) {
		height -= contents->getY() - top;
		top = contents->getY();
	}
	if (height + top > this->height)
		height = this->height - top;
	if (height + top > contents->getY() + contents->getHeight())
		height = contents->getY() + contents->getHeight() - top;
	if (top >= this->height || height <= 0)
		return;
	contents->initLineScanning(top - contents->getY(), 0, width);
	device->seek(lineBytes * top);
	for (int i = 0; i < height; i++) {
		memset(scanLine, DISPLAY_INVERT ? 0xFF : 0x00, lineBytes);
		contents->processLine(scanLine, 0);
		device->writeFully(scanLine, lineBytes);
	}
	device->flush();
}

void RedrawEvent::handle()
{
	getCurrentApplication()->onRedraw(top, height);
}
