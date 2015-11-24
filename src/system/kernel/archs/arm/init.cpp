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
#include <util/list.h>
#include <util/string.h>
#include <thread.h>
#include <debugging.h>
#include <system.h>
namespace Kernel
{
struct MEMPOOL ramPools[] = {
	#define BLOCK(origin, length)	{ origin, length },
	#define FLASH(...)
	#define RAM(...)				__VA_ARGS__
	#include EXPAND_AND_STRINGIFY(MCU_MEMORY_MAP)
	{ 0, 0 }
	#undef BLOCK
	#undef FLASH
	#undef RAM
};
Allocator ram(ramPools);
}
#include <sysutils.h>

#if __ARM_ARCH_PROFILE == 'M'
#include "cortexm/vectors.h"
#include "cortexm/itm.h"
#endif
#include "semihost.h"


using namespace Kernel;

/* provided by the linker.  */
extern dummy_t __system_stack_top;
extern dummy_t __system_relocate_source;
extern dummy_t __system_relocate_dest_start;
extern dummy_t __system_relocate_dest_end;
extern dummy_t __system_bss_start;
extern dummy_t __system_bss_end;
extern dummy_t __system_code_start;
extern dummy_t __system_code_end;
extern dummy_t __system_ramcode_start;
extern dummy_t __system_ramcode_end;
extern void (*__system_preinit_array_start []) (void) __attribute__((weak));
extern void (*__system_preinit_array_end []) (void) __attribute__((weak));
extern void (*__system_init_array_start []) (void) __attribute__((weak));
extern void (*__system_init_array_end []) (void) __attribute__((weak));

int hlog;

/*
irqHandler(USART1_IRQn)
{
	Semihost::sysWrite(hlog, "IRQ\n", 4);
	__REG(USART1_BASE + 0x04) = __REG(USART1_BASE + 0x04);
}
*/

/* main entry point. */
extern "C" void __reset__()
{
/*	itm_trace_enable();
	itm_trace("Running\n");
	for (;;);*/
	__enable_irq();
	hlog = Semihost::sysOpen(":tt", 8);
	Semihost::sysWrite(hlog, "Startup\n", 8);

	NVIC_SetPriorityGrouping(4);
	NVIC_SetPriority(USART1_IRQn, 0x10);
	NVIC_EnableIRQ(USART1_IRQn);
	__REG(RCC_BASE + 0x44) |= 1 << 4; // enable USART1
	__REG(USART1_BASE + 0x0C) |= (1 << 5) | (1 << 2) | (1 << 3) | (1 << 13); // enable USART1 CR1_RXNEIE, CR1_RE, CR1_TE and CR1_UE
	__REG(USART1_BASE + 0x04) = 'A'; // send char
	__REG(USART1_BASE + 0x04) = 'B'; // send char
	__REG(USART1_BASE + 0x04) = 'C'; // send char
	__REG(USART1_BASE + 0x04) = '\n'; // send char

	
	
	for (;;) {
		__WFE();
	}

	Semihost::sysClose(hlog);
	

	// copy relocate segment
	if (&__system_relocate_source != &__system_relocate_dest_start)
		memcpy(&__system_relocate_dest_start, &__system_relocate_source, (size_t)&__system_relocate_dest_end - (size_t)&__system_relocate_dest_start);
	// init bss segment
	memset(&__system_bss_start, 0, (size_t)&__system_bss_end - (size_t)&__system_bss_start);
	// call c++ constructors
	size_t count;
	size_t i;
	count = __system_preinit_array_end - __system_preinit_array_start;
	for (i = 0; i < count; i++)
		__system_preinit_array_start[i] ();
	count = __system_init_array_end - __system_init_array_start;
	for (i = 0; i < count; i++)
		__system_init_array_start[i] ();
/*
	// system init
	System *system;
	{ // In this block, we fake the existence of a current thread
	  // This allows us to use gcnew
		uint8_t threadBytes[sizeof(SysThread)] = { 0 };
		Scheduler::_currentThread = (SysThread *)threadBytes; 
		DeviceClass::registerClass(gcnew(Win32::DbgConsoleClass));
		getDebugOut();
		DeviceClass::registerClass(gcnew(Win32::DisplayDeviceClass));
		gc<Win32::KeyboardDeviceClass *> kbdClass = gcnew(Win32::KeyboardDeviceClass);
		calcKeyboard = kbdClass->getDevice(String()).staticCast<Win32::CalcKeyboard *>();
		computerKeyboard = kbdClass->getDevice("aux").staticCast<Win32::ComputerKeyboard *>();
		DeviceClass::registerClass(kbdClass);
		DeviceClass::registerClass(gcnew(Win32::BuzzerDeviceClass));
		system = System::get();
	}
	// back to actual state
	Scheduler::_currentThread = NULL;
	// start the system thread
	Scheduler::init();
	system->start();*/

	// halt if main returns
/*
#if defined(SYSTEM_USE_SEMIHOST)
	semihost_ReportException(semihost_ADP_Stopped_ApplicationExit);
#endif
*/
	abort();
}

extern "C" __attribute__((interrupt)) void handler_default()
{
	abort();
}

/* whaaaaaaat ?? */

EXTERN_C void __attribute__((weak, alias("handler_default"))) _Z12__irqHandlerILin14EEvv();
EXTERN_C void __attribute__((weak, alias("handler_default"))) _Z12__irqHandlerILin13EEvv();
EXTERN_C void __attribute__((weak, alias("handler_default"))) _Z12__irqHandlerILin12EEvv();
EXTERN_C void __attribute__((weak, alias("handler_default"))) _Z12__irqHandlerILin11EEvv();
EXTERN_C void __attribute__((weak, alias("handler_default"))) _Z12__irqHandlerILin10EEvv();
EXTERN_C void __attribute__((weak, alias("handler_default"))) _Z12__irqHandlerILin9EEvv();
EXTERN_C void __attribute__((weak, alias("handler_default"))) _Z12__irqHandlerILin8EEvv();
EXTERN_C void __attribute__((weak, alias("handler_default"))) _Z12__irqHandlerILin7EEvv();
EXTERN_C void __attribute__((weak, alias("handler_default"))) _Z12__irqHandlerILin6EEvv();
EXTERN_C void __attribute__((weak, alias("handler_default"))) _Z12__irqHandlerILin5EEvv();
EXTERN_C void __attribute__((weak, alias("handler_default"))) _Z12__irqHandlerILin4EEvv();
EXTERN_C void __attribute__((weak, alias("handler_default"))) _Z12__irqHandlerILin3EEvv();
EXTERN_C void __attribute__((weak, alias("handler_default"))) _Z12__irqHandlerILin2EEvv();
EXTERN_C void __attribute__((weak, alias("handler_default"))) _Z12__irqHandlerILin1EEvv();
#define DECLARE_IRQHANDLER(n)	EXTERN_C void __attribute__((weak, alias("handler_default"))) _Z12__irqHandlerILi##n##EEvv();
ENUMERATE(MCU_NUM_IRQn, DECLARE_IRQHANDLER)

/* vector table */
typedef void (* IrqHandler)();
__attribute__ ((section(".vectors")))
IrqHandler vectorTable[] = {
	(IrqHandler)&__system_stack_top,
	__reset__,
	_Z12__irqHandlerILin14EEvv,
	_Z12__irqHandlerILin13EEvv,
	_Z12__irqHandlerILin12EEvv,
	_Z12__irqHandlerILin11EEvv,
	_Z12__irqHandlerILin10EEvv,
	_Z12__irqHandlerILin9EEvv,
	_Z12__irqHandlerILin8EEvv,
	_Z12__irqHandlerILin7EEvv,
	_Z12__irqHandlerILin6EEvv,
	_Z12__irqHandlerILin5EEvv,
	_Z12__irqHandlerILin4EEvv,
	_Z12__irqHandlerILin3EEvv,
	_Z12__irqHandlerILin2EEvv,
	_Z12__irqHandlerILin1EEvv,
#define IRQHANDLER_VECTOR(n)	_Z12__irqHandlerILi##n##EEvv,
ENUMERATE(MCU_NUM_IRQn, IRQHANDLER_VECTOR)
};
