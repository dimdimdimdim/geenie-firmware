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
#include <criticalsection.h>

void CriticalSection::lock()
{
	if (status != 0 && thread == Kernel::getCurrentThread()) {
		// just increment status
		InterlockedIncrement(&status);
	}
	else {
		if (InterlockedCompareExchange(&status, 1, 0) != 0) {
			SysCalls::lockCriticalSection(this);
		}
		registerThreadLock();
	}
}
	
void CriticalSection::unlock()
{
	assert(status != 0 && thread == Kernel::getCurrentThread());
	if ((status & ~WAITERS_FLAG) > 1) {
		// just decrement status
		InterlockedDecrement(&status);
	}
	else {
		unregisterThreadLock();
		if (InterlockedCompareExchange(&status, 0, 1) != 1) {
			SysCalls::unlockCriticalSection(this);
		}
	}
}
