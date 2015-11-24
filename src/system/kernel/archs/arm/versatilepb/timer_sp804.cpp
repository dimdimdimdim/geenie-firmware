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
#include "systimer.h"
#include "../../peripherals/interrupts.h"
#include "../../peripherals/systimer.h"

#define TMR_BASE_ADDR(n) ((n < 2 ? 0x101e2000 : 0x101e3000) + ((n & 1) ? 0x20 : 0))
#define TMR01_IRQ 4
#define TMR23_IRQ 5

#define TMR_REG_LOAD(ba) (*((volatile uint32_t *)(ba + 0x000)))
#define TMR_REG_VALUE(ba) (*((volatile uint32_t *)(ba + 0x004)))
#define TMR_REG_CONTROL(ba) (*((volatile uint32_t *)(ba + 0x008)))
#define TMR_REG_INTCLR(ba) (*((volatile uint32_t *)(ba + 0x00C)))
#define TMR_REG_RIS(ba) (*((volatile uint32_t *)(ba + 0x010)))
#define TMR_REG_MIS(ba) (*((volatile uint32_t *)(ba + 0x014)))
#define TMR_REG_BGLOAD(ba) (*((volatile uint32_t *)(ba + 0x018)))

#define TMR_FREQUENCY 1000000

struct SysTimerData
{
	size_t baseAddress;
	irqHandler handler;
	int lastPeriod;
};

static SysTimerData timers[] = {
	{ TMR_BASE_ADDR(0), NULL, 0 },
	{ TMR_BASE_ADDR(1), NULL, 0 },
	{ TMR_BASE_ADDR(2), NULL, 0 },
	{ TMR_BASE_ADDR(3), NULL, 0 },
};
	
static void timer01IRQHandler()
{
	size_t ba;
	ba = TMR_BASE_ADDR(0);
	if (TMR_REG_MIS(ba) != 0) {
		TMR_REG_INTCLR(ba) = 0;
		if (timerHandlers[0])
			timerHandlers[0]();
	}
	ba = TMR_BASE_ADDR(1);
	if (TMR_REG_MIS(ba) != 0) {
		TMR_REG_INTCLR(ba) = 0;
		if (timerHandlers[1])
			timerHandlers[1]();
	}
}

static void timer23IRQHandler()
{
	size_t ba;
	ba = TMR_BASE_ADDR(2);
	if (TMR_REG_MIS(ba) != 0) {
		TMR_REG_INTCLR(ba) = 0;
		if (timerHandlers[2])
			timerHandlers[2]();
	}
	ba = TMR_BASE_ADDR(3);
	if (TMR_REG_MIS(ba) != 0) {
		TMR_REG_INTCLR(ba) = 0;
		if (timerHandlers[3])
			timerHandlers[3]();
	}
}

SYSTIMER *initSchedulerTimer(unsigned long msec, irqHandler handler)
{
	size_t ba = TMR_BASE_ADDR(0);
	configureInterrupt(TMR01_IRQ, IRQ_PRIORITY_SCHEDULER_TIMER, timer01IRQHandler);
	timerHandlers[0] = handler;
	schedulerTimer.baseAddress = ba;
	schedulerTimer.lastPeriod = msec * TMR_FREQUENCY / 1000;
	enableInterrupt(TMR01_IRQ);
	return &schedulerTimer;
}

SYSTIMER *initSleepTimer(irqHandler handler)
{
	size_t ba = TMR_BASE_ADDR(2);
	configureInterrupt(TMR23_IRQ, IRQ_PRIORITY_SLEEP_TIMER, timer23IRQHandler);
	timerHandlers[2] = handler;
	sleepTimer.baseAddress = ba;
	sleepTimer.lastPeriod = 0;
	enableInterrupt(TMR23_IRQ);
	return &sleepTimer;
}

void triggerTimer(SYSTIMER *timer)
{
	size_t ba = timer->baseAddress;
	TMR_REG_INTCLR(ba) = 0;
	TMR_REG_CONTROL(ba) = 0;
	TMR_REG_LOAD(ba) = timer->lastPeriod;
	TMR_REG_CONTROL(ba) = (1 << 7) | (1 << 5) | (1 << 1) | (1 << 0);
}

void cancelTimer(SYSTIMER *timer)
{
	size_t ba = timer->baseAddress;
	TMR_REG_INTCLR(ba) = 0;
	TMR_REG_CONTROL(ba) = 0;
}

void triggerTimerDelay(SYSTIMER *timer, unsigned long msec)
{
	size_t ba = timer->baseAddress;
	timer->lastPeriod = msec * TMR_FREQUENCY / 1000;
	TMR_REG_INTCLR(ba) = 0;
	TMR_REG_CONTROL(ba) = 0;
	TMR_REG_LOAD(ba) = timer->lastPeriod;
	TMR_REG_CONTROL(ba) = (1 << 7) | (1 << 5) | (1 << 1) | (1 << 0);
}

void disableTimerInterrupts(SYSTIMER *timer)
{
	size_t ba = timer->baseAddress;
	TMR_REG_CONTROL(ba) = TMR_REG_CONTROL(ba) & ~(1 << 5);
}

void enableTimerInterrupts(SYSTIMER *timer)
{
	size_t ba = timer->baseAddress;
	TMR_REG_CONTROL(ba) = TMR_REG_CONTROL(ba) | (1 << 5);
}

unsigned long getRemainingDelay(SYSTIMER *timer)
{
	size_t ba = timer->baseAddress;
	return TMR_REG_VALUE(ba) * 1000 / TMR_FREQUENCY;
}



