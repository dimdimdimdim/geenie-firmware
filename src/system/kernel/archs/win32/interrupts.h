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

#include "../../peripherals/interrupts.h"

namespace Kernel
{

namespace Win32
{

class InterruptSource
{
private:
	InterruptSource *next;
	bool enabled;
	bool triggered;
	IrqHandler handler;
	friend class Dispatcher;

public:
	InterruptSource(IrqHandler handler = NULL);

	bool isEnabled() { return enabled; }
	void enable();
	void disable();

	bool isTriggered() { return triggered; }
	void trigger();
	void clear();

	virtual void runHandler() { handler(); }
};

}

}