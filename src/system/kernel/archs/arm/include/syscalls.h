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
#include "../../../syscalls/fwddecls.h"

/*	SVC calls do NOT end up by executing the syscall handlers within the SVC handler (in SVC mode).
	Instead, they trigger the execution of the syscall handlers in system mode, within the context
	of the calling thread (interruptions are enabled).
	The only exception is SVCID_RESCHEDULE, which do not call any handler but just trigger a reschedule of threads.
	SVC calls are reentrant.
	SVC instruction must NOT be used from IRQ handler.
*/

namespace SysCalls
{

enum class Id : uint8_t
{
#define SYSCALL_DECLARE(n, r, p)	n,
#include "../../../syscalls/syscalls.i"
#undef SYSCALL_DECLARE
	COUNT,
	reschedule = 0xFF
};

#define SYSCALL_DECLARE(n, r, p)	EXPAND r n p;
#include "../../../syscalls/syscalls.i"
#undef SYSCALL_DECLARE

__attribute__((always_inline)) inline void reschedule() { asm volatile ("SVC %0" : : "I"((int)Id::reschedule): "r0", "r1", "r2", "r3", "r12", "cc", "memory"); }

}
