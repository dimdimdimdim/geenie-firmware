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
#include "dbgconsole.h"

// from Geenie.cpp
extern HANDLE hConsoleScreen;
extern HANDLE hConsoleInput;

namespace Kernel
{

namespace Win32
{

gc<DeviceDescriptor *> DbgConsole::open(unsigned long flags)
{
	return gcnew(DbgConsoleDesc(this));
}

size_t DbgConsoleDesc::write(const void *data, size_t len)
{
	if (hConsoleScreen) {
		DWORD dwWritten;
		if (!WriteConsoleA(hConsoleScreen, data, len, &dwWritten, NULL))
			gcthrownew(EInputOutput);
		return dwWritten;
	}
	return len;
}

size_t DbgConsoleDesc::read(void *data, size_t len)
{
/*	DWORD dwEventsCount;
	if (!ReadConsole(hScreen, data, len, &dwWritten, NULL))
		gcthrownew(EInputOutput);
	return dwWritten;*/
	return 0;
}

}

}
