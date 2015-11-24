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
#include <thread.h>
#include <waitableevent.h>
#include "scheduler.h"
#include "waitableobject.h"

namespace Kernel
{

bool WaitableObjectImpl::setWaitFlagAndWakeupOne(unsigned int flag)
{
	flags |= flag;
	WaitItem *item = waiters;
	if (item) {
		while (item && (item->flags & flags) == 0)
			item = item->nextInObjChain;
		if (item) {
			wakeup(item->op);
			return true;
		}
	}
	return false;
}

bool WaitableObjectImpl::setWaitFlagAndWakeupAll(unsigned int flag)
{
	flags |= flag;
	WaitItem *item = waiters;
	bool needsReschedule = false;
	while (item) {
		WaitItem *nextItem = item->nextInObjChain;
		if ((item->flags & flags) != 0) {
			wakeup(item->op);
			needsReschedule = true;
		}
		item = nextItem;
	}
	return needsReschedule;
}

void WaitableObjectImpl::appendWaiter(WaitItem *waiter)
{
	assert(waiter->prevRefInObjChain == NULL);
	waiter->nextInObjChain = NULL;
	waiter->prevRefInObjChain = lastWaiterRef;
	*lastWaiterRef = waiter; 
	lastWaiterRef = &waiter->nextInObjChain;
}

void WaitableObjectImpl::removeWaiter(WaitItem *waiter)
{
	if (waiter->prevRefInObjChain) {
		*waiter->prevRefInObjChain = waiter->nextInObjChain;
		if (lastWaiterRef == &waiter->nextInObjChain) // if last
			lastWaiterRef = waiter->prevRefInObjChain;
		waiter->nextInObjChain = NULL;
		waiter->prevRefInObjChain = NULL;
	}
}

WaitableEvent::WaitableEvent(bool autoReset, bool wakeupAll, bool set)
:WaitableObjectImpl(set ? WaitCond::EVENT_SET : 0)
{
	this->wakeupAll = wakeupAll;
	this->autoReset = autoReset;
}

bool WaitableEvent::isWaitFlagSet(unsigned int flag)
{
	bool s = WaitableObjectImpl::isWaitFlagSet(flag);
	if (autoReset && s)
		unsetWaitFlag(flag);
	return s;
}

void WaitableEvent::set()
{
	bool resch;
	__disable_irq();
	if (wakeupAll)
		resch = setWaitFlagAndWakeupAll(WaitCond::EVENT_SET);
	else
		resch = setWaitFlagAndWakeupOne(WaitCond::EVENT_SET);
	if (autoReset && resch)
		unsetWaitFlag(WaitCond::EVENT_SET);
	__enable_irq();
	if (resch)
		Scheduler::reschedule();
}

void WaitableEvent::reset()
{
	unsetWaitFlag(WaitCond::EVENT_SET);
}

namespace SysCallsImpl
{

bool isWaitFlagSet(Object *object, unsigned int flag)
{
	WaitableObject *wo = object->dynCast<WaitableObject>();
	if (wo == NULL)
		gcthrownew(EInvalidArgument);
	return wo->isWaitFlagSet(flag);
}

gc<WaitableEvent *> createEvent(bool autoReset, bool wakeupAll, bool set)
{
	return gcnew(WaitableEvent(autoReset, wakeupAll, set));
}

void setEvent(WaitableEvent *evt)
{
	evt->set();
}

void resetEvent(WaitableEvent *evt)
{
	evt->reset();
}

}

}