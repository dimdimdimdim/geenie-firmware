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

#include "../../peripherals/device.h"

namespace Kernel
{

namespace Win32
{

class BuzzerDesc : public DeviceDescriptor
{
	GC_INHERITS(DeviceDescriptor);

private:
	size_t seekPos;

public:
	BuzzerDesc(Device *device) :DeviceDescriptor(device) { seekPos = 0; }

	virtual size_t write(const void *data, size_t len);
};

class Buzzer : public Device
{
	GC_INHERITS(Device);

public:
	Buzzer(DeviceClass *devClass)  :Device(devClass) { }

	virtual gc<DeviceDescriptor *> open(unsigned long flags) { return gcnew(BuzzerDesc(this)); }
};

class BuzzerDeviceClass : public DeviceClass
{
	GC_INHERITS(DeviceClass);
	GC_FIELDS(buzzer);

private:
	Buzzer *buzzer;

public:
	BuzzerDeviceClass() :DeviceClass("buzzer") { buzzer = gcnew(Buzzer(this)); }

	virtual gc<Device *> getDevice(const String &name)
	{
		if (name)
			gcthrownew(EDeviceNotFound);
		return buzzer;
	}
};

}

}
