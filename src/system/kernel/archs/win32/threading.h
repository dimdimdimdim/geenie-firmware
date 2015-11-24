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

#include "interrupts.h"

namespace Kernel
{

namespace Win32
{

extern CRITICAL_SECTION win32Section;

class Dispatcher
{
private:
	InterruptSource *sources;

	void main();
	static DWORD WINAPI threadProc(LPVOID param);

public:
	Dispatcher();
	void start();

	void addInterrupt(InterruptSource *source);

	void notify();
};

extern Dispatcher dispatcher;

}

}