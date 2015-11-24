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

#include "../../kernel/threading/scheduler.h"

namespace Kernel
{
class SysThread;
class WaitOperation;
class WaitableObject;
class WaitableObjectImpl;
}

class WaitItem
{
private:
	friend class Kernel::WaitOperation;
	friend class Kernel::WaitableObjectImpl;
	friend class WaitList;
	WaitItem *nextInList;
	WaitItem *nextInObjChain;
	WaitItem **prevRefInObjChain;
	Object *waitObject;
	unsigned int flags;
	Kernel::WaitOperation *op;

public:
	WaitItem()									{ waitObject = NULL; prevRefInObjChain = NULL; }
	WaitItem(Object *obj)						{ waitObject = obj; flags = (unsigned int)-1; prevRefInObjChain = NULL; }
	WaitItem(Object *obj, unsigned int flags)	{ waitObject = obj; this->flags = flags; prevRefInObjChain = NULL; }
	WaitItem(const WaitItem &wi)				{ waitObject = wi.waitObject; flags = wi.flags; prevRefInObjChain = NULL; }
	WaitItem &operator=(const WaitItem &wi)		{ waitObject = wi.waitObject; flags = wi.flags; return *this; }
};

namespace WaitCond
{
static const unsigned int THREAD_TERMINATED = 1;
static const unsigned int ALL_THREADS_TERMINATED = 1;
}

class ThreadGroup : public Object
{
	GC_INHERITS(Object);
	GC_FIELDS(sysThreadGroup);

private:
	Kernel::SysThreadGroup *sysThreadGroup;

public:
	ThreadGroup()					{ sysThreadGroup = SysCalls::createThreadGroup(this); }

	void join()						{ SysCalls::joinThreadGroup(sysThreadGroup); }

	bool isTerminated()				{ return SysCalls::isWaitFlagSet(sysThreadGroup, WaitCond::ALL_THREADS_TERMINATED); }
	WaitItem onTerminated()			{ return WaitItem(sysThreadGroup, WaitCond::ALL_THREADS_TERMINATED); }
	operator WaitItem()				{ return WaitItem(sysThreadGroup); }
};

class Thread : public Object
{
	GC_INHERITS(Object);
	GC_FIELDS(sysThread);

private:
	Kernel::SysThread *sysThread;
	friend class Kernel::SysThread;

public:
	static const size_t defaultStackSize = 4096;
	static const int maxPriority = 5;
	static const int defaultPriority = 2;

protected:
	virtual int runProc() = 0;

public:
	Thread(size_t stackSize = defaultStackSize, int priority = defaultPriority, bool privileged = false)
		{ sysThread = SysCalls::createThread(this, stackSize, priority, privileged); }

	ThreadGroup *getGroup()									{ return sysThread->getUserThreadGroup(); }
	void detachFromGroup()									{ SysCalls::joinThreadGroup(NULL); }

	void start()											{ SysCalls::startThread(sysThread); }
	
	bool isTerminated()										{ return SysCalls::isWaitFlagSet(sysThread, WaitCond::THREAD_TERMINATED); }
	WaitItem onTerminated()									{ return WaitItem(sysThread, WaitCond::THREAD_TERMINATED); }
	operator WaitItem()										{ return WaitItem(sysThread); }
};

inline void sleep(long msec)								{ SysCalls::sleep(msec); }
inline Thread *getCurrentThread()							{ return Kernel::getCurrentThread()->getUserThread(); }
inline ThreadGroup *getCurrentThreadGroup()					{ return Kernel::getCurrentThread()->getUserThreadGroup(); }

class WaitList
{
private:
	friend class Kernel::WaitOperation;
	WaitItem *items;

public:
	WaitList()														{ items = NULL; }
	WaitList(const WaitItem &item)									{ WaitItem *itm = const_cast<WaitItem *>(&item); itm->nextInList = NULL; items = itm; }

	// note: NO COPY of the WaitItem arguments is made internally
	// only the references are maintained.
	// you must ensure that the lifetime of the items passed to those functions persist
	// at least during the entire wait() function call.
	WaitList &operator,(const WaitItem &item)						{ WaitItem *itm = const_cast<WaitItem *>(&item); itm->nextInList = items; items = itm; return *this; }
	void append(const WaitItem &item)								{ WaitItem *itm = const_cast<WaitItem *>(&item); itm->nextInList = items; items = itm; }
};

inline bool wait(const WaitList &objects, long msecTimeout = -1)	{ return SysCalls::wait(objects, msecTimeout); }
inline bool wait(const WaitItem &item, long msecTimeout = -1)		{ return SysCalls::wait(WaitList(item), msecTimeout); }

inline unsigned long getTickCount()									{ return SysCalls::getTickCount(); }
