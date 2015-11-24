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

#include <io/printstream.h>

typedef enum
{
	LOGLEVEL_DEBUG,
	LOGLEVEL_INFO,
	LOGLEVEL_WARN,
	LOGLEVEL_ERROR,
	LOGLEVEL_CRIT,
} LogLevel;

typedef enum
{
	LOGFACILITY_SYS = 1,
	LOGFACILITY_APP = 2,

	LOGFACILITY_ALL = 0xFFFF
} LogFacility;

extern LogLevel logLevel;
extern LogFacility logFacilities;

//#ifdef DEBUG
#define LOG(facility, level, ...) \
        do \
        { \
			if (LOGLEVEL_##level >= logLevel && (logFacilities & LOGFACILITY_##facility) != 0) \
                getDebugOut()->printf(__VA_ARGS__); \
        } \
        while (0)
//#else
//#define LOG(x)
//#endif

IO::PrintStream *getDebugOut();

