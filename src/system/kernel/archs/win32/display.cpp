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
#include <stdafx.h>
#include <base.h>
#include "display.h"

using namespace IO::Ctl;

#define LINE_BYTES	(((SCREEN_WIDTH + 31) >> 3) & ~3)

// from Geenie.cpp
extern void *pFrameBuffer;
void updateScreen();

namespace Kernel
{

namespace Win32
{

static HANDLE hScreen;
static HANDLE hInput;

gc<DeviceDescriptor *> Display::open(unsigned long flags)
{
	return gcnew(DisplayDesc(this));
}

size_t DisplayDesc::control(Code request, va_list args)
{
	switch (request) {
	case FLUSH:
		updateScreen();
		return 0;
	case SEEK:
		seekPos = va_arg(args, size_t);
		return 0;
	case GET_POS:
		return seekPos;
	case GET_SIZE:
		return LINE_BYTES * SCREEN_HEIGHT;
	case GET_WIDTH:
		return SCREEN_WIDTH;
	case GET_HEIGHT:
		return SCREEN_HEIGHT;
	case GET_LINE_BYTES:
		return LINE_BYTES;
	}
	gcthrownew(EUnavailableIOOperation);
}

size_t DisplayDesc::write(const void *data, size_t len)
{
	size_t max = LINE_BYTES * SCREEN_HEIGHT;
	if (seekPos >= max)
		gcthrownew(EEndOfFile);
	const char *dataBytes = (const char *)data;
	char *destBytes = (char *)pFrameBuffer;
	int rl = 0;
	while (len && seekPos < max) {
		destBytes[seekPos++] = *dataBytes++;
		len--;
		rl++;
	}
	return rl;
}

void DisplayDesc::close()
{
	updateScreen();
}

}

}
