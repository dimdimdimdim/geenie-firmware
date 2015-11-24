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

#include <util/circbuffer.h>
#include <criticalsection.h>
#include <keyboard.h>
#include "interrupts.h"
#include "../../peripherals/device.h"

namespace Kernel
{

namespace Win32
{

class CalcKeyboardDesc : public DeviceDescriptor, virtual public WaitableObjectRedirect
{
	GC_INHERITS(DeviceDescriptor, WaitableObjectRedirect);
	GC_NO_FIELDS;

public:
	CalcKeyboardDesc(Device *device);
	virtual size_t read(void *data, size_t len);
};

class CalcKeyboard : public Device, virtual public WaitableObjectImpl, public InterruptSource
{
	GC_INHERITS(Device, WaitableObjectImpl);
	GC_NO_FIELDS;

private:
	CriticalSection section;
	CircBuffer<uint8_t, 16> buffer;

protected:
	virtual void runHandler();

public:
	CalcKeyboard(DeviceClass *devClass);
	size_t read(void *data, size_t len);
	virtual gc<DeviceDescriptor *> open(unsigned long flags) { return gcnew(CalcKeyboardDesc(this)); }
};

class ComputerKeyboardDesc : public DeviceDescriptor, virtual public WaitableObjectRedirect
{
	GC_INHERITS(DeviceDescriptor, WaitableObjectRedirect);
	GC_NO_FIELDS;

public:
	ComputerKeyboardDesc(Device *device);
	virtual size_t read(void *data, size_t len);
};

class ComputerKeyboard : public Device, virtual public WaitableObjectImpl, public InterruptSource
{
	GC_INHERITS(Device, WaitableObjectImpl);
	GC_NO_FIELDS;

private:
	CriticalSection section;
	CircBuffer<AuxKeyboardData, 16> buffer;

protected:
	virtual void runHandler();

public:
	ComputerKeyboard(DeviceClass *devClass);
	size_t read(void *data, size_t len);
	virtual gc<DeviceDescriptor *> open(unsigned long flags) { return gcnew(ComputerKeyboardDesc(this)); }
	virtual gc<String> getName() { return getClass()->getName() + ":aux"; }
};

class KeyboardDeviceClass : public DeviceClass
{
	GC_INHERITS(DeviceClass);
	GC_FIELDS(calcKeyboard, computerKeyboard);

private:
	CalcKeyboard *calcKeyboard;
	ComputerKeyboard *computerKeyboard;

public:
	KeyboardDeviceClass() :DeviceClass("keyboard")
	{
		calcKeyboard = gcnew(CalcKeyboard(this));
		computerKeyboard = gcnew(ComputerKeyboard(this));
	}

	virtual gc<Device *> getDevice(const String &name)
	{
		if (name) {
			if (name == "aux")
				return computerKeyboard;
			gcthrownew(EDeviceNotFound);
		}
		return calcKeyboard;
	}
};

}

}
