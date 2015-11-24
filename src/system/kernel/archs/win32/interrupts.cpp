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
#include "interrupts.h"
#include "threading.h"

namespace Kernel
{

namespace Win32
{

InterruptSource::InterruptSource(IrqHandler handler)
{
	this->handler = handler;
	enabled = false;
	triggered = false;
	dispatcher.addInterrupt(this);
}

void InterruptSource::enable()
{
	if (!enabled) {
		enabled = true;
		memBarrier();
		if (triggered)
			dispatcher.notify();
	}
}

void InterruptSource::disable()
{
	enabled = false;
}

void InterruptSource::trigger()
{
	if (!triggered) {
		triggered = true;
		memBarrier();
		if (enabled)
			dispatcher.notify();
	}
}

void InterruptSource::clear()
{
	triggered = false;
}

}

}
