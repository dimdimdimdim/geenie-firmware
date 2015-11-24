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
#include <thread.h>
#include <debugging.h>
#include "criticalsection.h"
#include "scheduler.h"
#include "systhread.h"

namespace Kernel
{

void _sleepTimerHandler();

namespace Scheduler
{

int _threadCount = 0;

void _schedulerTimerHandler();

static SysThread *_runningQueues[::Thread::maxPriority + 1] = {};
static SysThread *_waitingQueue = NULL;

SysTimer _schedulerTimer;
SysTimer _sleepTimer;
int _irqNestingLevel = 0;
bool _needsReschedule = false;
bool _threadSwitchDisabled = false;
SysThread *_currentThread = NULL;

void init()
{
	_schedulerTimer = SysTimer(_schedulerTimerHandler, 50);
	_schedulerTimer.enableInterrupts();
	_sleepTimer = SysTimer(_sleepTimerHandler);
	_sleepTimer.enableInterrupts();
}

SysThread *_getThreadToRun()
{
	for (int i = ::Thread::maxPriority; i >= 0; i--) {
		if (_runningQueues[i] != NULL)
			return _runningQueues[i];
	}
	return NULL;
}

void disableContextSwitch()
{
	assert(_threadSwitchDisabled == false);
	_threadSwitchDisabled = true;
	memBarrier();
}

void enableContextSwitch()
{
	_threadSwitchDisabled = false;
	memBarrier();
	if (_needsReschedule)
		reschedule();
}

int getThreadList(SysThread **destArray, int arraySize)
{
	__disable_irq();
	int n = 0;
	for (int p = 0; p < ::Thread::maxPriority + 1; p++) {
		SysThread *thread = _runningQueues[p];
		while (thread) {
			if (arraySize == 0) {
				__enable_irq();
				return -1;
			}
			*destArray = thread;
			arraySize--; n++;
			thread = thread->next;
		}
	}
	SysThread *thread = _waitingQueue;
	while (thread) {
		if (arraySize == 0) {
			__enable_irq();
			return -1;
		}
		*destArray = thread;
		arraySize--; n++;
		thread = thread->next;
	}
	__enable_irq();
	return n;
}

void _schedulerTimerHandler()
{
	if (_currentThread == NULL)
		return;
	__disable_irq();
	_runningQueues[_currentThread->getPriority()] = _currentThread->next;
	__enable_irq();
	reschedule();
}

void setThreadRunningRaw(SysThread *thread)
{
	if (!thread->isRunning()) {
		if (thread->next != NULL) {
			assert(thread->prev != NULL);
			SysThread *&root = _waitingQueue;
			if (thread->next == thread) {
				assert(thread->prev == thread);
				assert(root == thread);
				root = NULL;
			}
			else {
				if (root == thread)
					root = thread->next;
				thread->prev->next = thread->next;
				thread->next->prev = thread->prev;
			}
		}
		else {
			assert(thread->prev == NULL);
		}
		{
			SysThread *&root = _runningQueues[thread->getPriority()];
			if (root == NULL) {
				thread->next = thread;
				thread->prev = thread;
				root = thread;
			}
			else {
				if (root->next == root) {
					// if there is only one thread on this queue
					// put on front, otherwise scheduler will never trigger timer
					// because root will never change
					thread->next = root->next->next;
					thread->prev = root->next;
					root->next = thread;
					root->next->next->prev = thread;
					root = thread;
				}
				else {
					// default: put at the end of the queue
					thread->next = root;
					thread->prev = root->prev;
					root->prev->next = thread;
					root->prev = thread;
				}
			}
		}
		if ((thread->flags & SysThread::FLAG_HAS_RUN) == 0) {
			thread->flags |= SysThread::FLAG_HAS_RUN;
			_threadCount++;
			if (thread->group) {
				thread->nextInGroup = thread->group->first;
				if (thread->nextInGroup)
					thread->nextInGroup->prevInGroupRef = &thread->nextInGroup;
				thread->prevInGroupRef = &thread->group->first;
				thread->group->first = thread;
			}
		}
		thread->flags |= SysThread::FLAG_RUNNING;
		thread->operation = NULL;
	}
}

void removeThread(SysThread *thread)
{
	assert((thread->flags & SysThread::FLAG_HAS_RUN) != 0);
	LOG(SYS, INFO, "Thread %p terminated\n", thread);
	__disable_irq();
	_threadCount--;
	SysThread *&root = thread->isRunning() ? _runningQueues[thread->getPriority()] : _waitingQueue;
	if (thread->next != NULL) {
		assert(thread->prev != NULL);
		if (thread->next == thread) {
			assert(thread->prev == thread);
			assert(root == thread);
			root = NULL;
		}
		else {
			if (root == thread)
				root = thread->next;
			thread->prev->next = thread->next;
			thread->next->prev = thread->prev;
		}
		thread->next = NULL;
		thread->prev = NULL;
	}
	else {
		assert(thread->prev == NULL);
	}
	thread->flags &= ~SysThread::FLAG_RUNNING;
	thread->abortOperation();
	CriticalSection *section = thread->firstLocked;
	while (section) {
		assert(section->status != 0 && section->thread == thread);
		section->thread = NULL;
		section->status &= CriticalSection::WAITERS_FLAG;
		if (section->waiters)
			setThreadRunningRaw(section->waiters->getThread());
		section = section->nextLockedForThread;
	}
	thread->firstLocked = NULL;
	_deleteThreadContext(thread);
	thread->setWaitFlagAndWakeupAll(WaitCond::THREAD_TERMINATED);
	if (thread->group) {
		*thread->prevInGroupRef = thread->nextInGroup;
		if (thread->group->first == NULL)
			thread->group->setWaitFlagAndWakeupAll(WaitCond::ALL_THREADS_TERMINATED);
	}
	__enable_irq();
	reschedule();
}

void setThreadRunning(SysThread *thread)
{
	if (!thread->isRunning()) {
		__disable_irq();
		setThreadRunningRaw(thread);
		__enable_irq();
		reschedule();
	}
}

void setThreadWaiting(SysThread *thread, ThreadOperation *operation)
{
	__disable_irq();
	if (thread->isRunning()) {
		//LOG(KERNEL, DEBUG, "Thread %p suspended\n", thread);
		if (thread->next != NULL) {
			assert(thread->prev != NULL);
			SysThread *&root = _runningQueues[thread->getPriority()];
			if (thread->next == thread) {
				assert(thread->prev == thread);
				assert(root == thread);
				root = NULL;
			}
			else {
				if (root == thread)
					root = thread->next;
				thread->prev->next = thread->next;
				thread->next->prev = thread->prev;
			}
		}
		else {
			assert(thread->prev == NULL);
		}
		{
			SysThread *&root = _waitingQueue;
			if (root == NULL) {
				thread->next = thread;
				thread->prev = thread;
				root = thread;
			}
			else {
				thread->next = root;
				thread->prev = root->prev;
				root->prev->next = thread;
				root->prev = thread;
			}
		}
		thread->flags &= ~SysThread::FLAG_RUNNING;
		thread->operation = operation;
	}
	__enable_irq();
	reschedule();
}

void reschedule()
{
	_needsReschedule = true;
	if (_irqNestingLevel == 0 && !_threadSwitchDisabled)
		SysCalls::reschedule();
}

}

// sleeping mechanism

static class SleepOperation *sleepers = NULL;

class SleepOperation : public ThreadOperation
{
private:
	friend void _sleepTimerHandler();
	SleepOperation *next;
	SleepOperation **ref;
	unsigned long target;
public:
	SleepOperation(long msec)
	{
		if (msec <= 0) {
			ref = NULL;
			return;
		}

		target = getTickCount() + msec;
		SleepOperation **insertAt;
		if (sleepers == NULL) {
			insertAt = &sleepers;
			Scheduler::_sleepTimer.trigger(msec);
		}
		else {
			insertAt = &sleepers;
			while (*insertAt != NULL && (signed long)(target - (*insertAt)->target) > 0)
				insertAt = &(*insertAt)->next;
			if (insertAt == &sleepers)
				Scheduler::_sleepTimer.trigger(msec);
		}

		ref = insertAt;
		next = *insertAt;
		*insertAt = this;
	}
protected:
	virtual void doAbort()
	{
		if (ref) {
			*ref = next;
			if (next)
				next->ref = ref;
			ref = NULL;
		}
	}
	virtual void timeout() { }
};

void _sleepTimerHandler()
{
	unsigned long now = getTickCount();
	__disable_irq();
	while (sleepers && (signed long)(sleepers->target - now) <= 2) {
		Scheduler::setThreadRunningRaw(sleepers->getThread());
		sleepers->timeout();
		sleepers->ref = NULL;
		sleepers = sleepers->next;
	}
	if (sleepers) {
		Scheduler::_sleepTimer.trigger(sleepers->target - now);
	}
	__enable_irq();
	Scheduler::reschedule();
}

class WaitOperation : public SleepOperation
{
private:
	const WaitList &list;

	void removeFromObjectChains()
	{
		WaitItem *item = list.items;
		while (item != NULL) {
			item->waitObject->dynCast<WaitableObject>()->removeWaiter(item);
			item = item->nextInList;
		}
	}

protected:
	bool timedOut;
	virtual void doAbort()
	{
		SleepOperation::doAbort();
		removeFromObjectChains();
	}
	virtual void timeout()
	{
		timedOut = true;
		removeFromObjectChains();
	}

public:
	WaitOperation(const WaitList &list, long msecTimeout)
	:SleepOperation(msecTimeout), list(list)
	{
		timedOut = false;
		WaitItem *item = list.items;
		while (item != NULL) {
			item->op = this;
			item->waitObject->dynCast<WaitableObject>()->appendWaiter(item);
			item = item->nextInList;
		}
	}

	void wakeup()
	{
		SleepOperation::doAbort();
		removeFromObjectChains();
		Scheduler::setThreadRunningRaw(getThread());
	}

	bool hasTimedOut() { return timedOut; }

	static bool checkItemFlags(const WaitList &list)
	{
		WaitItem *item = list.items;
		while (item != NULL) {
			WaitableObject *wobj = item->waitObject->dynCast<WaitableObject>();
			if (wobj == NULL)
				gcthrownew(EInvalidArgument);
			if (wobj->isWaitFlagSet(item->flags))
				return true;
			item = item->nextInList;
		}
		return false;
	}
};

void wakeup(WaitOperation *op)
{
	op->wakeup();
}

namespace SysCallsImpl
{

void sleep(long msec)
{
	if (msec <= 0) {
		// just yield
		__disable_irq();
		if (Scheduler::_runningQueues[Scheduler::_currentThread->getPriority()] == Scheduler::_currentThread)
			Scheduler::_runningQueues[Scheduler::_currentThread->getPriority()] = Scheduler::_currentThread->next;
		__enable_irq();
		Scheduler::reschedule();
	}
	else {
		__disable_irq();
		SleepOperation op(msec);
		Scheduler::setThreadWaiting(op.getThread(), &op); // will reenable IRQs
	}
}

bool wait(const WaitList &list, long msec)
{
	if (WaitOperation::checkItemFlags(list))
		return true;
	if (msec == 0)
		return false;
	__disable_irq();
	// recheck when IRQs are disabled
	if (WaitOperation::checkItemFlags(list)) {
		__enable_irq();
		return true;
	}
	WaitOperation op(list, msec);
	Scheduler::setThreadWaiting(op.getThread(), &op); // will reenable IRQs
	return !op.hasTimedOut();
}

}

}

