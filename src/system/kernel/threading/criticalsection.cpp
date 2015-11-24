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
#include <thread.h>
#include "criticalsection.h"
#include "scheduler.h"
#include "systhread.h"

void CriticalSection::registerThreadLock()
{
	Kernel::SysThread *thread = Kernel::getCurrentThread();
	this->thread = thread;
	if (thread->firstLocked)
		thread->firstLocked->prevLockedForThreadRef = &nextLockedForThread;
	prevLockedForThreadRef = &thread->firstLocked;
	nextLockedForThread = thread->firstLocked;
	thread->firstLocked = this;
}

void CriticalSection::unregisterThreadLock()
{
	assert(thread == Kernel::getCurrentThread());
	*prevLockedForThreadRef = nextLockedForThread;
	thread = NULL;
}

namespace Kernel
{

CriticalSectionAcquisition::CriticalSectionAcquisition(CriticalSection *section)
{
	this->section = section;
	next = NULL;
	prevRef = section->lastWaiterRef;
	*section->lastWaiterRef = this;
	section->lastWaiterRef = &next;
	section->status |= CriticalSection::WAITERS_FLAG;
}

CriticalSectionAcquisition::~CriticalSectionAcquisition()
{
	// remove from waiter list
	doAbort();
	// update section status
	assert((section->status & ~CriticalSection::WAITERS_FLAG) == 0);
	section->status = 1;
}

void CriticalSectionAcquisition::doAbort()
{
	*prevRef = next;
	if (section->lastWaiterRef == &next) // if last
		section->lastWaiterRef = prevRef;
	if (section->waiters == NULL)
		section->status &= ~CriticalSection::WAITERS_FLAG;
}

void CriticalSectionAcquisition::lock(CriticalSection *section)
{
	Scheduler::disableContextSwitch();
	SysThread *thread = Kernel::getCurrentThread();
	if (section->status > 0 && section->thread == thread) {
		section->status++;
	}
	else if (section->status == 0) {
		section->status = 1;
	}
	else {
		CriticalSectionAcquisition op(section);
		Scheduler::setThreadWaiting(thread, &op);
		Scheduler::enableContextSwitch();
		// we get stuck here until critical section is released
		Scheduler::disableContextSwitch();
		// op gets destroyed here and performs actual acquisition
	}
	Scheduler::enableContextSwitch();
}

void CriticalSectionAcquisition::unlock(CriticalSection *section)
{
	Scheduler::disableContextSwitch();
	if ((section->status & ~CriticalSection::WAITERS_FLAG) > 1) {
		section->status--;
	}
	else if (section->status == 1) {
		assert(section->waiters == NULL);
		section->status = 0;
	}
	else {
		assert(section->waiters != NULL);
		section->status &= CriticalSection::WAITERS_FLAG;
		Scheduler::setThreadRunning(section->waiters->getThread());
	}
	Scheduler::enableContextSwitch();
}

namespace SysCallsImpl
{

void lockCriticalSection(CriticalSection *section)
{
	CriticalSectionAcquisition::lock(section);
}

void unlockCriticalSection(CriticalSection *section)
{
	CriticalSectionAcquisition::unlock(section);
}

}

}
