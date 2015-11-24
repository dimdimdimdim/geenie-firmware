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

/* This part replaces eh_alloc.o from libsupc++, because it uses
   an emergency heap that takes too much space in RAM.
   We don't need such a feature. */

#define sizeof_cxa_refcounted_exception 128
#define sizeof_cxa_dependent_exception 120
extern "C" void _Unwind_DeleteException(_Unwind_Exception *exc);

extern "C" __cxa_eh_globals *__cxa_get_globals()
{
	return &Kernel::getCurrentThread()->context.exceptionState;
}

extern "C" __cxa_eh_globals *__cxa_get_globals_fast()
{
	return &Kernel::getCurrentThread()->context.exceptionState;
}

extern "C" void free_eh_globals(__cxa_eh_globals *g)
{
	if (g) {
		__cxa_exception* exn = g->caughtExceptions;
		__cxa_exception* next;
		while (exn) {
			next = exn->nextException;
			_Unwind_DeleteException(&exn->unwindHeader);
			exn = next;
		}
	}
}

extern "C" void *__cxa_allocate_exception(size_t thrown_size) throw()
{
	void *ret;

	thrown_size += sizeof_cxa_refcounted_exception;
	ret = SysCalls::malloc(thrown_size);
	if (!ret)
		abort();
	
	__cxa_eh_globals *globals = __cxa_get_globals();
	globals->uncaughtExceptions += 1;

	memset(ret, 0, sizeof_cxa_refcounted_exception);

	return (void *)((char *)ret + sizeof_cxa_refcounted_exception);
}

extern "C" void __cxa_free_exception(void *vptr) throw()
{
    SysCalls::free((char *)vptr - sizeof_cxa_refcounted_exception);
}

extern "C" void *__cxa_allocate_dependent_exception() throw()
{
	void *ret;

	ret = SysCalls::malloc(sizeof_cxa_dependent_exception);
	if (!ret)
		abort();

	__cxa_eh_globals *globals = __cxa_get_globals ();
	globals->uncaughtExceptions += 1;

	memset(ret, 0, sizeof_cxa_dependent_exception);

	return ret;
}

extern "C" void __cxa_free_dependent_exception(void *vptr) throw()
{
	SysCalls::free(vptr);
}

extern "C" void __cxa_pure_virtual()
{
	LOG(SYS, ERROR, "Pure virtual function call");
	abort();
}

extern "C" int __cxa_atexit(void (*f)(void *), void *objptr, void *dso)
{
	// we do nothing... yeah, the kernel global objects destructors are not registered,
	// so they are never released. The kernel does not exit anyway, except for when a reset occurs
	// Implementing this in the kernel is also difficult, because __cxa_atexit requires
	// dynamic allocation, which is not yet available when the OS starts.
	return 0; // return success...
}

namespace __gnu_cxx
{

void __verbose_terminate_handler()
{
	LOG(SYS, ERROR, "Terminate called");
	abort();
}

};

void *__dso_handle = NULL;

/* provided by the linker */
extern dummy_t __system_code_start;
extern dummy_t __system_code_end;
extern dummy_t __system_ramcode_start;
extern dummy_t __system_ramcode_end;
extern dummy_t __exidx_start;
extern dummy_t __exidx_end;

extern "C" void *__gnu_Unwind_Find_exidx(void *pc, int *count)
{
	if (	((size_t)pc >= (size_t)&__system_code_start && (size_t)pc < (size_t)&__system_code_end) ||
			((size_t)pc >= (size_t)&__system_ramcode_start && (size_t)pc < (size_t)&__system_ramcode_end)) {
		*count = ((size_t)&__exidx_end - (size_t)&__exidx_start) / 8;
		return (void *)&__exidx_start;
	}
	return NULL;
}
