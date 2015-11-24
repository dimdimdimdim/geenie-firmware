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
#include <sysutils.h>
#include "syscallstable.h"
#include "../../kernel/syscalls/syscalls.h"

namespace
{

template<typename FuncT> class USRSysCallHandler
{
};

template<typename Ret, typename... Args> class USRSysCallHandler<Ret(Args...)>
{
public:
    template<Ret (* func)(Args...)> static Ret wrapper(Args... args)
    {
		try {
			Ret r = func(args...);
			__switch_to_USR();
			return r;
		}
		catch (...) {
			__switch_to_USR();
			throw;
		}
    }
};

template<typename... Args> class USRSysCallHandler<void(Args...)>
{
public:
    template<void (* func)(Args...)> static void wrapper(Args... args)
    {
		try {
			func(args...);
			__switch_to_USR();
		}
		catch (...) {
			__switch_to_USR();
			throw;
		}
    }
};

}

namespace Kernel
{

namespace ARM
{

svcHandler svcHandlersTable[] = {
#define SYSCALL_ENTRY(func)			(svcHandler)func, (svcHandler)USRSysCallHandler<decltype(func)>::wrapper<func>
#define SYSCALL_DECLARE(n, r, p)	SYSCALL_ENTRY(Kernel::SysCallsImpl::n),
#include "../../syscalls/syscalls.i"
#undef SYSCALL_DECLARE
};

}

}

namespace SysCalls
{
#define SYSCALL_DECLARE(n, r, p)	__attribute__((naked, noinline)) EXPAND r n p { asm volatile ("SVC %0" : : "I"((int)Id::n): "r0", "r1", "r2", "r3", "r12", "cc", "memory"); }
#include "../../../syscalls/syscalls.i"
#undef SYSCALL_DECLARE
}