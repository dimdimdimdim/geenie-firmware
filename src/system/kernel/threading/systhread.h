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

#include <object.h>
#include <archstructs.h>
#include "scheduler.h"
#include "waitableobject.h"

class Thread;
class ThreadGroup;
class Exception;
class CriticalSection;

namespace Kernel
{

class GCConstrItem;
class SysThread;
class ThreadOperation;

class ThreadOperation
{
protected:
	SysThread *waitingThread;
	bool aborted;
	
	/* doAbort is only called when the IRQs are disabled.
	   So this function will be run atomically (except for high-level IRQ handlers).
	   As a consequence: DO NOT globally disable/enable interrupts or disable/enable thread switch within this.
	   You may disable specific interrupts through the interrupt controller, though. */
	virtual void doAbort() = 0;
	
	friend class SysThread;
	
public:
	ThreadOperation();
	void abort(); // must be called only when IRQs are disabled
	SysThread *getThread()	{ return waitingThread; }
	bool isAborted()		{ return aborted; }
};

class SysThread;

class SysThreadGroup : public Object, public virtual WaitableObjectImpl
{
	GC_INHERITS(Object, WaitableObjectImpl);
	GC_FIELDS(userThreadGroup);

private:
	friend void Scheduler::setThreadRunningRaw(SysThread *thread);
	friend void Scheduler::removeThread(SysThread *thread);
	friend class SysThread;

private:
	ThreadGroup *userThreadGroup;
	SysThread *first;

public:
	SysThreadGroup(ThreadGroup *userThreadGroup) { this->userThreadGroup = userThreadGroup; first = NULL; }
};

class SysThread : public Object, public virtual WaitableObjectImpl
{
	GC_INHERITS(Object, WaitableObjectImpl);
	GC_FIELDS(userThread, group, stack);

public: // yeah, everything is public... everybody has to hack this class anyway... 
	friend void Scheduler::removeThread(SysThread *thread); // accesses WaitableObjectImpl protected methods

	SysThread *prev, *next;
	SysThread *nextInGroup, **prevInGroupRef;
	Thread *userThread;
	SysThreadGroup *group;

	static const uint32_t FLAG_HAS_RUN =		0x10000000;
	static const uint32_t FLAG_PRIVILEGED =		0x80000000;
	static const uint32_t FLAG_RUNNING =		0x40000000;
	static const uint32_t FLAG_SUSPENDED =		0x20000000;
	static const uint32_t FLAG_PRIORITY_MASK =	0x000000FF;
	uint32_t flags;

	size_t stackSize;
	void *stack;
	ThreadArchContext context;
	ThreadOperation *operation;
	CriticalSection *firstLocked;
	int retVal;

	GCStackItem *lastGCStackItem;
	GCConstrItem *lastGCConstrItem;
	Exception *lastThrownException;
	void **tmpStackObjRef;

	static void threadLauncher();
	
	void abortOperation() // must be called only when IRQs are disabled
	{
		if (operation) {
			operation->abort();
			operation = NULL;
		}
	}
	
public:
	SysThread(Thread *userThread, size_t stackSize, int priority, bool privileged);

	int getPriority()	{ return (flags & FLAG_PRIORITY_MASK) != 0; }
	bool isSuspended()	{ return (flags & FLAG_SUSPENDED) != 0; }
	bool isRunning()	{ return (flags & FLAG_RUNNING) != 0; }
	bool isPrivileged()	{ return (flags & FLAG_PRIVILEGED) != 0; }

	void start();
	void joinGroup(SysThreadGroup *group);

	Thread *getUserThread() { return userThread; }
	ThreadGroup *getUserThreadGroup() { if (group == NULL) return NULL; return group->userThreadGroup; }
};

}
