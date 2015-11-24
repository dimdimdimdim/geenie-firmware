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

#include <thread.h>

namespace Kernel
{
class WaitableEvent;
}

namespace WaitCond
{
static const int EVENT_SET = 1;
}

class WaitableEvent : public Object
{
	GC_INHERITS(Object);
	GC_FIELDS(sysEvent);

private:
	Kernel::WaitableEvent *sysEvent;

public:
	WaitableEvent(bool autoReset = true, bool wakeupAll = false, bool set = false) { sysEvent = SysCalls::createEvent(autoReset, wakeupAll, set); }

	void set()											{ SysCalls::setEvent(sysEvent); }
	void reset()										{ SysCalls::resetEvent(sysEvent); }
	bool isSet()										{ return SysCalls::isWaitFlagSet((Object *)sysEvent, WaitCond::EVENT_SET); }
	WaitItem onSet()									{ return WaitItem((Object *)sysEvent, WaitCond::EVENT_SET); }
};
