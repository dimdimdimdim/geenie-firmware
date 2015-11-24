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
#include <stdafx.h>
#include <base.h>
#include <sysutils.h>
#include <criticalsection.h>
#include "keyboard.h"
#include "threading.h"

// from Geenie.cpp
extern uint8_t keyCode;
extern AuxKeyboardData auxKeyData;

namespace Kernel
{

namespace Win32
{

CalcKeyboardDesc::CalcKeyboardDesc(Device *device)
:DeviceDescriptor(device), WaitableObjectRedirect((CalcKeyboard *)device)
{
}

size_t CalcKeyboardDesc::read(void *data, size_t len)
{
	return ((CalcKeyboard *)getDevice())->read(data, len);
}

CalcKeyboard::CalcKeyboard(DeviceClass *devClass)
:Device(devClass)
{
	enable();
}

void CalcKeyboard::runHandler()
{
	bool wasEmpty = buffer.isEmpty();
	buffer.write(&keyCode, 1);
	if (wasEmpty) {
		__disable_irq();
		bool resch = setWaitFlagAndWakeupOne(WaitCond::READ_READY);
		__enable_irq();
		if (resch)
			Scheduler::reschedule();
	}
}

size_t CalcKeyboard::read(void *data, size_t len)
{
	size_t n;
	synchronized(section) {
		disable();
		n = buffer.read((uint8_t *)data, len);
		if (buffer.isEmpty())
			unsetWaitFlag(WaitCond::READ_READY);
		enable();
	}
	return n;
}

ComputerKeyboardDesc::ComputerKeyboardDesc(Device *device)
:DeviceDescriptor(device), WaitableObjectRedirect((ComputerKeyboard *)device)
{
}

size_t ComputerKeyboardDesc::read(void *data, size_t len)
{
	return ((ComputerKeyboard *)getDevice())->read(data, len);
}

ComputerKeyboard::ComputerKeyboard(DeviceClass *devClass)
:Device(devClass)
{
	enable();
}

void ComputerKeyboard::runHandler()
{
	bool wasEmpty = buffer.isEmpty();
	buffer.write(&auxKeyData, 1);
	if (wasEmpty) {
		__disable_irq();
		bool resch = setWaitFlagAndWakeupOne(WaitCond::READ_READY);
		__enable_irq();
		if (resch)
			Scheduler::reschedule();
	}
}

size_t ComputerKeyboard::read(void *data, size_t len)
{
	if (len != sizeof(AuxKeyboardData))
		gcthrownew(EInvalidArgument);
	size_t n;
	synchronized(section) {
		disable();
		n = buffer.read((AuxKeyboardData *)data, 1);
		if (buffer.isEmpty())
			unsetWaitFlag(WaitCond::READ_READY);
		enable();
	}
	return n * sizeof(AuxKeyboardData);
}

}

}
