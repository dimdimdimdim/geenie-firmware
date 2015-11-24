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
#include <sysutils.h>
#include <debugging.h>
#include "scheduler.h"
#include <thread.h>

namespace Kernel
{

ThreadOperation::ThreadOperation()
{
	waitingThread = Scheduler::_currentThread;
	aborted = false;
}

void ThreadOperation::abort()
{
	doAbort();
	aborted = true;
	waitingThread->operation = NULL;
	Scheduler::setThreadRunning(waitingThread);
}

void SysThread::threadLauncher()
{
	SysThread *t = Scheduler::_currentThread;
	t->retVal = -1;
	try {
		t->retVal = t->userThread->runProc();
		t->lastThrownException = NULL;
	}
	catch (Exception *ex) {
		LOG(SYS, ERROR, "Thread %p died because of an uncatched exception: %s\n", t, ex->getMessage());
	}
	catch (...) {
		LOG(SYS, CRIT, "Thread %p died because an unsupported exception was throw\n", t);
	}
	Scheduler::removeThread(t);
	for (;;) ; /* never reached */
}

SysThread::SysThread(Thread *userThread, size_t stackSize, int priority, bool privileged)
{
	LOG(SYS, INFO, "Thread %p created\n", this);
	assert(priority <= Thread::maxPriority);
	this->userThread = userThread;
	group = NULL;
	if (getCurrentThread() != NULL)
		group = getCurrentThread()->group;
	nextInGroup = NULL;
	prevInGroupRef = NULL;
	prev = NULL;
	next = NULL;
	operation = NULL;
	firstLocked = NULL;
	this->stackSize = stackSize;
	if (stackSize == 0) 
		this->stack = NULL;
	else
		this->stack = gcnew(char[stackSize]);
	this->flags = (privileged ? FLAG_PRIVILEGED : 0) | FLAG_SUSPENDED | priority;
	lastGCStackItem = NULL;
	lastGCConstrItem = NULL;
	tmpStackObjRef = NULL;
	memset(&context, 0, sizeof(context));
}

void SysThread::start()
{
	flags &= ~FLAG_SUSPENDED;
	Scheduler::_createThreadContext(this);
	Scheduler::setThreadRunning(this);
}

void SysThread::joinGroup(SysThreadGroup *group)
{
	bool resch = false;
	__disable_irq();
	if (prevInGroupRef) {
		*prevInGroupRef = nextInGroup;
		if (this->group->first == NULL)
			resch = this->group->setWaitFlagAndWakeupAll(WaitCond::ALL_THREADS_TERMINATED);
	}
	this->group = group;
	if (group) {
		nextInGroup = group->first;
		if (nextInGroup)
			nextInGroup->prevInGroupRef = &nextInGroup;
		prevInGroupRef = &group->first;
		group->first = this;
	}
	__enable_irq();
	if (resch)
		Scheduler::reschedule();
}

namespace SysCallsImpl
{

gc<Kernel::SysThread *> createThread(Thread *userThread, size_t stackSize, int priority, bool privileged)
{
	if (stackSize == 0 || (privileged && !Kernel::getCurrentThread()->isPrivileged()))
		throw gcnew(EOperationNotPermitted);
	return gcnew(SysThread(userThread, stackSize, priority, privileged));
}

void startThread(SysThread *thread)
{
	thread->start();
}

gc<Kernel::SysThreadGroup *> createThreadGroup(ThreadGroup *userThreadGroup)
{
	return gcnew(SysThreadGroup(userThreadGroup));
}

void joinThreadGroup(SysThreadGroup *group)
{
	getCurrentThread()->joinGroup(group);
}

}
 
}
