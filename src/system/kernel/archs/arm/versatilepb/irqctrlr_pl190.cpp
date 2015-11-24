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

#ifndef CORTEX_M

#include "../../peripherals/interrupts.h"

#define VIC_BASE_ADDR 0x10140000
#define VIC_IRQSTATUS (*((volatile uint32_t *)(VIC_BASE_ADDR + 0x000)))
#define VIC_INTENABLE (*((volatile uint32_t *)(VIC_BASE_ADDR + 0x010)))
#define VIC_INTENCLEAR (*((volatile uint32_t *)(VIC_BASE_ADDR + 0x014)))
#define VIC_VADDR (*((volatile irqHandler *)(VIC_BASE_ADDR + 0x030)))
#define VIC_DEFVECTADDR (*((volatile irqHandler *)(VIC_BASE_ADDR + 0x034)))
#define VIC_VECTADDR(n) (*((volatile irqHandler *)(VIC_BASE_ADDR + 0x100 + n*4)))
#define VIC_VECTCNTL(n) (*((volatile uint32_t *)(VIC_BASE_ADDR + 0x200 + n*4)))

static bool defaultHandlerConfigured = false;
static irqHandler irqHandlersD[32] = {}; /* Only used within the default vector */

irqHandler getInterruptHandler()
{
	return VIC_VADDR;
}

void acknowledgeInterrupt()
{
	VIC_VADDR = NULL;
}

static void defaultPriorityHandler()
{
	uint32_t status = VIC_IRQSTATUS;
	if (status != 0) {
		int i = __builtin_ctz(status);
		irqHandler handler = irqHandlersD[i];
		if (handler != NULL)
			handler();
	}
}

void configureInterrupt(int irqid, int priority, irqHandler handler)
{
	if (!defaultHandlerConfigured) {
		VIC_DEFVECTADDR = defaultPriorityHandler;
		for (size_t i = 0; i < sizeof(irqHandlersD) / sizeof(void *); i++)
			irqHandlersD[i] = NULL;
		defaultHandlerConfigured = true;
	}
	
	assert(irqid < 32);
	irqHandlersD[irqid] = handler;
	if (priority < 16) {
		VIC_VECTCNTL(priority) = irqid | 0x20;
		VIC_VECTADDR(priority) = handler;
	}
}

void enableInterrupt(int irqid)
{
	VIC_INTENABLE = 1 << irqid;
}

void disableInterrupt(int irqid)
{
	VIC_INTENCLEAR = 1 << irqid;
}

#endif
