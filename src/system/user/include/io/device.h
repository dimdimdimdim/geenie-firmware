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
#include <util/string.h>
#include <io/streams.h>

namespace Kernel
{
class Device;
}

namespace WaitCond
{
static const int READ_READY = 1;
static const int WRITE_READY = 2;
}

namespace IO
{

namespace Ctl
{

enum Code {
// file ops
	FLUSH,			// no args
	SEEK,			// takes a size_t arg
	GET_POS,		// no args
	GET_SIZE,		// no args
// display ops
	GET_WIDTH,		// no args
	GET_HEIGHT,		// no args
	GET_LINE_BYTES,	// no args
};

}

static const unsigned long NON_BLOCKING =	1;
static const unsigned long EXCLUSIVE =		2;

class Device : public Object, public virtual InputStream, public virtual OutputStream
{
	GC_INHERITS(Object, InputStream, OutputStream);
	GC_FIELDS(kdev);

private:
	Kernel::DeviceDescriptor *kdev;
	bool blocking;
	Device(Kernel::DeviceDescriptor *kdev, unsigned long flags) { this->kdev = kdev; blocking = (flags & NON_BLOCKING) == 0; }

public:
	static gc<Device *> open(const String &name, unsigned long flags = 0) { return gcnew(Device(SysCalls::open(name, flags), flags)); }

	gc<String> getName()								{ return SysCalls::getDeviceName(kdev); }
	size_t control(Ctl::Code request, va_list args)		{ return SysCalls::control(kdev, request, args); }

	size_t write(const void *data, size_t len)
	{
		if (blocking) {
			if (len == 0)
				return 0;
			for (;;) {
				if (!isWriteReady())
					wait(onWriteReady());
				size_t n = SysCalls::write(kdev, data, len);
				if (n != 0)
					return n;
			}
		}
		else
			return SysCalls::write(kdev, data, len);
	}

	size_t read(void *data, size_t len)
	{
		if (blocking) {
			if (len == 0)
				return 0;
			for (;;) {
				if (!isReadReady())
					wait(onReadReady());
				size_t n = SysCalls::read(kdev, data, len);
				if (n != 0)
					return n;
			}
		}
		else
			return SysCalls::read(kdev, data, len);
	}

	bool isWriteReady()									{ return SysCalls::isWaitFlagSet((Object *)kdev, WaitCond::WRITE_READY); }
	bool isReadReady()									{ return SysCalls::isWaitFlagSet((Object *)kdev, WaitCond::READ_READY); }
	WaitItem onWriteReady()								{ return WaitItem((Object *)kdev, WaitCond::WRITE_READY); }
	WaitItem onReadReady()								{ return WaitItem((Object *)kdev, WaitCond::READ_READY); }
	operator WaitItem()									{ return WaitItem((Object *)kdev); }
	void close()										{ return SysCalls::close(kdev); }

	size_t control(Ctl::Code request, ...)
	{
		va_list args;
		va_start(args, request);
		try {
			int ret = control(request, args);
			va_end(args);
			return ret;
		}
		catch (Exception *e) {
			va_end(args);
			throw e;
		}
	}

	void flush()			{ control(Ctl::FLUSH); }
	void seek(size_t pos)	{ control(Ctl::SEEK, pos); }
	size_t getPos()			{ return control(Ctl::GET_POS); }
};

}
