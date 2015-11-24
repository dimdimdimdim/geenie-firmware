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

#include <thread.h>

namespace Kernel
{
class MessageQueue;
}

namespace WaitCond
{
static const int GET_MSG_READY = 1;
static const int SEND_MSG_READY = 2;
}

template<class T> class MessageQueue : public Object
{
	GC_INHERITS(Object);
	GC_FIELDS(sysMsgQueue);

private:
	Kernel::MessageQueue *sysMsgQueue;

public:
	MessageQueue(size_t maxmsgs = 10)					{ sysMsgQueue = SysCalls::createMessageQueue(maxmsgs); }

	gc<T *> getMessage(bool nowait = false)				{ return gc<T *>((T *)SysCalls::getMessage(sysMsgQueue, nowait).get()); }
	bool sendMessage(T *message, bool nowait = false)	{ return SysCalls::sendMessage(sysMsgQueue, message, nowait); }

	bool isGetReady()									{ return SysCalls::isWaitFlagSet((Object *)sysMsgQueue, WaitCond::GET_MSG_READY); }
	bool isSendReady()									{ return SysCalls::isWaitFlagSet((Object *)sysMsgQueue, WaitCond::SEND_MSG_READY); }
	WaitItem onGetReady()								{ return WaitItem((Object *)sysMsgQueue, WaitCond::GET_MSG_READY); }
	WaitItem onSendReady()								{ return WaitItem((Object *)sysMsgQueue, WaitCond::SEND_MSG_READY); }
};