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
#include "../syscalls/syscalls.h"
#include "messagequeue.h"

namespace Kernel
{

MessageQueue::MessageQueue(size_t size)
:WaitableObjectImpl(WaitCond::SEND_MSG_READY)
{
	start = 0;
	length = 0;
	this->size = size;
	SysCallsImpl::gcAlloc(*(void **)&messages, sizeof(Object *) * size);
}

gc<Object *> MessageQueue::getMessage(bool nowait)
{
	if (length == 0 && nowait)
		return NULL;
	Scheduler::disableContextSwitch();
	while (length == 0) {
		Scheduler::enableContextSwitch();
		SysCallsImpl::wait(WaitList(WaitItem(this, WaitCond::GET_MSG_READY)), -1);
		Scheduler::disableContextSwitch();
	}
	bool wasFull = length == size;
	gc<Object *> obj = messages[start];
	messages[start++] = NULL;
	if (start == size)
		start = 0;
	length--;
	if (wasFull) {
		__disable_irq();
		bool resch = setWaitFlagAndWakeupAll(WaitCond::SEND_MSG_READY);
		__enable_irq();
		if (resch)
			Scheduler::reschedule();
	}
	if (length == 0)
		unsetWaitFlag(WaitCond::GET_MSG_READY);
	Scheduler::enableContextSwitch();
	return obj;
}

bool MessageQueue::sendMessage(Object *message, bool nowait)
{
	if (length == size && nowait)
		return false;
	Scheduler::disableContextSwitch();
	while (length == size) {
		Scheduler::enableContextSwitch();
		SysCallsImpl::wait(WaitList(WaitItem(this, WaitCond::SEND_MSG_READY)), -1);
		Scheduler::disableContextSwitch();
	}
	bool wasEmpty = length == 0;
	size_t i = start + length;
	if (i >= size)
		i -= size;
	assert(messages[i] == NULL);
	messages[i] = message;
	length++;
	if (wasEmpty) {
		__disable_irq();
		bool resch = setWaitFlagAndWakeupAll(WaitCond::GET_MSG_READY);
		__enable_irq();
		if (resch)
			Scheduler::reschedule();
	}
	if (length == size)
		unsetWaitFlag(WaitCond::SEND_MSG_READY);
	Scheduler::enableContextSwitch();
	return true;
}

namespace SysCallsImpl
{

gc<MessageQueue *> createMessageQueue(size_t maxmsgs)
{
	return gcnew(MessageQueue(maxmsgs));
}

bool sendMessage(MessageQueue *mq, Object *msg, bool nowait)
{
	return mq->sendMessage(msg, nowait);
}

gc<Object *> getMessage(MessageQueue *mq, bool nowait)
{
	return mq->getMessage(nowait);
}

}

}
