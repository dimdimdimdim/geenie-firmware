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

#include <stdincludes.h>

#include <util/traits.h>
#include "../../kernel/memory/allocator.h"
#include <object.h>
#include "../../kernel/threading/systhread.h"

#ifdef _WIN32
#include "../../kernel/syscalls/syscalls.h"
namespace SysCalls = Kernel::SysCallsImpl;
#else
#include <syscalls.h>
#endif

#include <objectimpl.h>

#include <exception.h>
