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

#include "../peripherals/systimer.h"

class Thread;

namespace Kernel
{

class SysThread;
class SysThreadGroup;
class ThreadOperation;
class WaitOperation;

namespace Scheduler
{

//////////* private part */
extern int _threadCount;

extern int _irqNestingLevel;
extern bool _needsReschedule;
extern bool _threadSwitchDisabled;

extern SysThread *_getThreadToRun();
void _createThreadContext(SysThread *thread);
void _deleteThreadContext(SysThread *thread);
extern SysThread *_currentThread;

extern SysTimer _schedulerTimer, _sleepTimer;
//////////////////

void init();

/* Note: this only disables reschedules. Interrupts are still
	allowed to be triggered but when the interrupt returns, the task is
	guaranteed to be resumed. Also note that even when task switching is
	disabled, task lists may be updated (e.g. tasks can switch from waiting
	to running).
	Must not be called from IRQ handler. */
void disableContextSwitch();
void enableContextSwitch();

/* Will not take effect until reschedule is called.
	Can be called from IRQ handler, or anywhere. */
void setThreadRunningRaw(SysThread *thread); // must be called when IRQs are disabled
void setThreadRunning(SysThread *thread); // must be called when IRQs are enabled
void setThreadWaiting(SysThread *thread, ThreadOperation *operation);
void removeThread(SysThread *thread);

inline int getThreadCount() { return _threadCount; }
/* use getThreadCount to build an array big enough
   returns -1 if the array if too small, or the number of threads
   placed in the array if succeeded */
int getThreadList(SysThread **destArray, int arraySize);

/* Triggers rescheduling of the threads. currentThread must have
	been set prior to calling this.
	Actual thread switching may be deferred to a later time if atomic
	operation is currently being executed. */
void reschedule();

};

class ContextSwitchLock
{
public:
	ContextSwitchLock()		{ Scheduler::disableContextSwitch(); }
	~ContextSwitchLock()	{ Scheduler::enableContextSwitch(); }
};

inline SysThread *getCurrentThread() { return Scheduler::_currentThread; }

void wakeup(WaitOperation *op);

}
