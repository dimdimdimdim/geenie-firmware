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
#include <stdafx.h>
#include <mmsystem.h>

inline void *stdMalloc(size_t s) { return malloc(s); }
inline void stdFree(void *p) { free(p); }

#include <base.h>
#include <debugging.h>
#include "../../peripherals/systimer.h"
#include "interrupts.h"
#include "threading.h"

namespace Kernel
{

struct SysTimerData : public Win32::InterruptSource
{
	SysTimerData(IrqHandler handler) :InterruptSource(handler) { }

	UINT timerId;
	unsigned long lastDelay;
};

SysTimer::SysTimer(IrqHandler handler, unsigned long msec)
{
	data = new(stdMalloc(sizeof(SysTimerData))) SysTimerData(handler);
	data->lastDelay = msec;
	data->timerId = NULL;
}

void CALLBACK timerProc(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
	SysTimerData *data = (SysTimerData *)dwUser;
	data->timerId = NULL;
	data->trigger();
}

void SysTimer::trigger()
{
	trigger(data->lastDelay);
}

void SysTimer::trigger(unsigned long msec)
{
	EnterCriticalSection(&Win32::win32Section);
	data->lastDelay = msec;
	if (data->timerId != NULL)
		timeKillEvent(data->timerId);
	data->timerId = timeSetEvent(msec, 5, timerProc, (DWORD_PTR)data, TIME_ONESHOT | TIME_CALLBACK_FUNCTION | TIME_KILL_SYNCHRONOUS);
	LeaveCriticalSection(&Win32::win32Section);
}

void SysTimer::cancel()
{
	EnterCriticalSection(&Win32::win32Section);
	if (data->timerId != NULL)
		timeKillEvent(data->timerId);
	data->timerId = NULL;
	LeaveCriticalSection(&Win32::win32Section);
}

void SysTimer::disableInterrupts()
{
	data->disable();
}

void SysTimer::enableInterrupts()
{
	data->enable();
}

unsigned long getTickCount()
{
	return GetTickCount();
}

}