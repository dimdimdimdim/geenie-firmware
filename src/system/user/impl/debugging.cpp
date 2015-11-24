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
#include <debugging.h>
#include <io/device.h>
#include <io/printstream.h>

LogLevel logLevel = LOGLEVEL_DEBUG;
LogFacility logFacilities = LOGFACILITY_ALL;

using namespace IO;

bool debugOutAvailable = false;

PrintStream *getDebugOut()
{
	static gc<PrintStream *, GCGlobalItem> out = NULL;
	if (out == NULL) {
		out = gcnew(PrintStream(Device::open("dbgconsole")));
		debugOutAvailable = true;
	}
	return out;
}

