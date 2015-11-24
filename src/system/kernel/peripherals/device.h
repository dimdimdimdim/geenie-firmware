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

#include <util/list.h>
#include <io/device.h>
#include "../threading/waitableobject.h"

namespace Kernel
{

class Device;	
class DeviceDescriptor;

class DeviceClass : public Object
{
	GC_INHERITS(Object);
	GC_FIELDS(name);

private:
	String name;
	static List<DeviceClass *> &getRegistry();

public:
	DeviceClass(const String &name)								{ this->name = name; }

	static void registerClass(DeviceClass *devClass)			{ getRegistry().append(devClass); }
	static DeviceClass *getClass(const String &name);
	static const List<DeviceClass *> &getAll()					{ return getRegistry(); }

	const String &getName()										{ return name; }
	virtual gc<Device *> getDevice(const String &name) = 0;
};

class Device : public Object
{
	GC_INHERITS(Object);
	GC_FIELDS(devClass);

private:
	DeviceClass *devClass;

public:
	Device(DeviceClass *devClass)								{ this->devClass = devClass; }

	DeviceClass *getClass()										{ return devClass; }
	virtual gc<String> getName()								{ return devClass->getName(); }

	virtual gc<DeviceDescriptor *> open(unsigned long flags) = 0;
};

class DeviceDescriptor : public Object, public virtual WaitableObject
{
	GC_INHERITS(Object, WaitableObject);
	GC_FIELDS(device);

private:
	Device *device;

public:
	DeviceDescriptor(Device *device)							{ this->device = device; }

	Device *getDevice()											{ return device; }

	size_t control(IO::Ctl::Code request, ...);
	virtual size_t control(IO::Ctl::Code request, va_list args)	{ gcthrownew(EUnavailableIOOperation); }
	virtual size_t write(const void *data, size_t len)			{ gcthrownew(EUnavailableIOOperation); }
	virtual size_t read(void *data, size_t len)					{ gcthrownew(EUnavailableIOOperation); }
	virtual void close()										{ }
};

gc<DeviceDescriptor *> openDevice(const String &name, unsigned long flags);

}

