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

#include "waitableobject.h"
#include <messagequeue.h>

namespace Kernel
{

class MessageQueue : public Object, public virtual WaitableObjectImpl
{
	GC_INHERITS(Object, WaitableObjectImpl);

protected:
	size_t start, length, size;
	Object **messages;

public:
	virtual void gcMarkFields()	const
	{
		for (size_t i = 0; i < size; i++)
			gcMarkObject(messages[i]);
	}

public:
	MessageQueue(size_t size);

	gc<Object *> getMessage(bool nowait = false);
	bool sendMessage(Object *message, bool nowait = false);
};

}
