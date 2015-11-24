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

#include <util/string.h>
#include "../../../../res/messages.h"

template<class T> T *gcTrackException(T *ex)
{
	Kernel::getCurrentThread()->lastThrownException = ex;
	return ex;
}

template<class T> T *gcTrackException(gc<T *> ex)
{
	Kernel::getCurrentThread()->lastThrownException = ex;
	return ex;
}

#define gcthrow(x)		throw(gcTrackException(ex))
#define gcthrownew(x)	throw(gcTrackException(gcnew(x)))

// Exceptions are garbage collected.
// They must be thrown using gcthrow/gcthrownew and catched by pointer.
// Note that the SysThread object always keeps a reference to the last thrown Exception
// so that it is not wiped by the garbage collector. However, we can't determine
// when it has been catched, so the reference is maintained even if the exception
// object is not used anymore. Therefore, be careful if the exception object maintains
// garbage-collected references to some other object/structures, it can lead to temporary
// memory leaks (until some other exception is thrown or the thread terminates).
class Exception : public Object
{
	GC_INHERITS(Object);
	GC_FIELDS(message);
private:
	String message;
public:
	Exception(const String &message)	{ this->message = message; }
	const String &getMessage()			{ return message; }
};

#define DECLARE_EXCEPTION(type, base, defaultMsg) \
	class type : public base \
	{ \
	public: \
		type() :base(Message(defaultMsg)) { } \
		type(const String &message) :base(message) { } \
	}

#define DECLARE_EXCEPTION_NOMSG(type, base) \
	class type : public base \
	{ \
	public: \
		type(const String &message) :base(message) { } \
	}

DECLARE_EXCEPTION(ENotEnoughMemory, Exception, NotEnoughMemory);
DECLARE_EXCEPTION(EAssertionFailed, Exception, AssertionFailed);
DECLARE_EXCEPTION(EInvalidArgument, Exception, InvalidArgument);

DECLARE_EXCEPTION(ESecurity, Exception, SecurityError);
DECLARE_EXCEPTION(EOperationNotPermitted, ESecurity, OperationNotPermitted);

DECLARE_EXCEPTION(EInputOutput, Exception, InputOutputError);
DECLARE_EXCEPTION(EDeviceNotFound, EInputOutput, DeviceNotFound);
DECLARE_EXCEPTION(EFileNotFound, EDeviceNotFound, FileNotFound);
DECLARE_EXCEPTION(EEndOfFile, EInputOutput, EndOfFile);
DECLARE_EXCEPTION(EUnavailableIOOperation, EInputOutput, UnavailableIOOperation);
