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

namespace Kernel
{
class SysThread;
class CriticalSectionAcquisition;

namespace Scheduler
{
void removeThread(SysThread *thread);
}

}

/* this is a recursive critical section */
class CriticalSection
{
private:
	// status is 0 when free, 1 when acquired, n when acquired multiple times by the same thread
	// status is ored with WAITERS_FLAG when there are waiters
	static const long WAITERS_FLAG = 0x80000000;
	volatile long status;
	Kernel::SysThread *thread;
	Kernel::CriticalSectionAcquisition *waiters, **lastWaiterRef;
	CriticalSection *nextLockedForThread, **prevLockedForThreadRef;

	friend class Kernel::CriticalSectionAcquisition;
	friend void Kernel::Scheduler::removeThread(SysThread *thread);

	void registerThreadLock();
	void unregisterThreadLock();

public:
	CriticalSection()
	{
		status = 0;
		thread = NULL;
		waiters = NULL;
		lastWaiterRef = &waiters;
	}
	
	~CriticalSection()
	{
		/* Results are unpredictable if the object is released while the section is locked */
		assert(status == 0);
	}
	
	void lock();
	void unlock();
};

struct SynchronizedBlock
{
	CriticalSection &section;
	bool done;
	SynchronizedBlock(CriticalSection &s)			:section(s) { done = false; section.lock(); }
	~SynchronizedBlock()							{ section.unlock(); }
};

#define ___synchronized(x, c) \
	for (SynchronizedBlock __lock##c(x); !__lock##c.done; __lock##c.done = true)

#define __synchronized(x, c) ___synchronized(x, c)

#define synchronized(x) __synchronized(x, __COUNTER__)
