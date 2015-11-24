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

class WaitItem;

namespace Kernel
{

class WaitableObject : public virtual Interface
{
	GC_INHERITS(Interface);
	GC_NO_FIELDS;

public:
	virtual bool isWaitFlagSet(unsigned int flag)		{ return true; }
	virtual void appendWaiter(WaitItem *waiter)			{ }
	virtual void removeWaiter(WaitItem *waiter)			{ }
};

class WaitableObjectRedirect : public virtual WaitableObject
{
	GC_INHERITS(WaitableObject);
	GC_FIELDS(target);

private:
	WaitableObject *target;

public:
	WaitableObjectRedirect(WaitableObject *target)		{ this->target = target; }
	virtual bool isWaitFlagSet(unsigned int flag)		{ return target->isWaitFlagSet(flag); }
	virtual void appendWaiter(WaitItem *waiter)			{ target->appendWaiter(waiter); }
	virtual void removeWaiter(WaitItem *waiter)			{ target->removeWaiter(waiter); }
};

class WaitableObjectImpl : public virtual WaitableObject
{
	GC_INHERITS(WaitableObject);
	GC_NO_FIELDS;

private:
	unsigned int flags;
	WaitItem *waiters, **lastWaiterRef;

protected:
	void initWaitFlags(unsigned int flags)	{ this->flags = flags; }
	bool setWaitFlagAndWakeupOne(unsigned int flag); // must be called only with IRQ disabled
	bool setWaitFlagAndWakeupAll(unsigned int flag); // must be called only with IRQ disabled
	void unsetWaitFlag(unsigned int flag)	{ this->flags &= ~flag; }

public:
	WaitableObjectImpl(unsigned int flags) { this->flags = flags; waiters = NULL; lastWaiterRef = &waiters; }
	WaitableObjectImpl() { flags = 0; waiters = NULL; lastWaiterRef = &waiters; }

	virtual bool isWaitFlagSet(unsigned int flag)	{ return (flags & flag) != 0; }
	virtual void appendWaiter(WaitItem *waiter);
	virtual void removeWaiter(WaitItem *waiter);
};

class WaitableEvent : public Object, public virtual WaitableObjectImpl
{
	GC_INHERITS(Object, WaitableObjectImpl);
	GC_NO_FIELDS;

private:
	bool wakeupAll;
	bool autoReset;

public:
	WaitableEvent(bool autoReset = true, bool wakeupAll = false, bool set = false);
	virtual bool isWaitFlagSet(unsigned int flag);
	void set();
	void reset();
};

}
