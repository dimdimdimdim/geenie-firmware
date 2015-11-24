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
#include <util/macros.h>
#include "fwddecls.h"

namespace Kernel
{

namespace SysCallsImpl
{

#define SYSCALL_DECLARE(n, r, p)	EXPAND r n p;
#include "syscalls.i"
#undef SYSCALL_DECLARE

#ifdef _WIN32
void reschedule();
#endif

}

}
