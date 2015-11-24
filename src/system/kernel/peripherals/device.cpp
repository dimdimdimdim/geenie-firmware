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
#include "device.h"

namespace Kernel
{

List<DeviceClass *> &DeviceClass::getRegistry()
{
	static gc<List<DeviceClass *>, GCGlobalItem> registry;
	return registry;
}

DeviceClass *DeviceClass::getClass(const String &name)
{
	for (List<DeviceClass *>::ConstIterator i = getRegistry().start(); i; i++) {
		if ((*i)->getName() == name)
			return (*i);
	}
	return NULL;
}

size_t DeviceDescriptor::control(IO::Ctl::Code request, ...)
{
	va_list args;
	va_start(args, request);
	try {
		size_t ret = control(request, args);
		va_end(args);
		return ret;
	}
	catch (...) {
		va_end(args);
		throw;
	}
}

gc<DeviceDescriptor *> openDevice(const String &name, unsigned long flags)
{
	size_t n = name.findFirst(':');
	gc<String> className, deviceName;
	if (n == String::notFound) {
		className = name;
	}
	else {
		className = name.part(0, n);
		if (n < name.getLength())
			deviceName = name.part(n + 1);
	}
	DeviceClass *devClass = DeviceClass::getClass(className);
	if (devClass == NULL)
		gcthrownew(EDeviceNotFound);
	return devClass->getDevice(deviceName)->open(flags);
}

namespace SysCallsImpl
{

gc<DeviceDescriptor *> open(const String &name, unsigned long flags)
{
	return openDevice(name, flags);
}

gc<String> getDeviceName(DeviceDescriptor *dev)
{
	return dev->getDevice()->getName();
}

size_t control(DeviceDescriptor *dev, int request, va_list args)
{
	return dev->control((IO::Ctl::Code)request, args);
}

size_t write(DeviceDescriptor *dev, const void *data, size_t len)
{
	return dev->write(data, len);
}

size_t read(DeviceDescriptor *dev, void *data, size_t len)
{
	return dev->read(data, len);
}

void close(DeviceDescriptor *dev)
{
	dev->close();
}

}

}
