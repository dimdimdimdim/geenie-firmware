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

inline void *stdMalloc(size_t s) { return malloc(s); }
inline void stdFree(void *p) { free(p); }

#include <base.h>
#include <debugging.h>
#include "threading.h"
#include "interrupts.h"
#include "../../threading/scheduler.h"

namespace Kernel
{

namespace Win32
{

CRITICAL_SECTION win32Section; // this is used to prevent the disptacher to SuspendThread() within some Win32 calls.

int interruptsEnabled = 0;
static HANDLE interruptEvent;
static HANDLE thread;
static HANDLE kernelThread;

CRITICAL_SECTION sleepSection;
LONGLONG sleepingCycles = 0;
LARGE_INTEGER lastSleepTimer = { 0, 0 };
bool withinSleep = false;

float getCPUUsage()
{
	static LARGE_INTEGER lastRequestTime = { 0, 0 };
	static LONGLONG lastRequestSleepingCycles = 0;
	static bool initialized = false;

	LARGE_INTEGER now;
	QueryPerformanceCounter(&now);
	LONGLONG totDiff = now.QuadPart - lastRequestTime.QuadPart;
	EnterCriticalSection(&sleepSection);
	LONGLONG slpAbs = sleepingCycles;
	if (withinSleep)
		slpAbs += now.QuadPart - lastSleepTimer.QuadPart;
	LeaveCriticalSection(&sleepSection);
	LONGLONG slpDiff = slpAbs - lastRequestSleepingCycles;
	lastRequestSleepingCycles = slpAbs;
	lastRequestTime = now;

	if (!initialized || totDiff == 0) {
		initialized = true;
		return 0;
	}
	else {
		if (lastSleepTimer.QuadPart == 0) // dispatcher not yet ready
			return 100;
		return 100 - (float)(slpDiff * 100) / totDiff;
	}
}

Dispatcher::Dispatcher()
{
	sources = NULL;
	interruptsEnabled = true;
	interruptEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	thread = CreateThread(NULL, 0, threadProc, this, CREATE_SUSPENDED, NULL);
	SetThreadPriority(thread, THREAD_PRIORITY_HIGHEST);
	InitializeCriticalSection(&sleepSection);
	InitializeCriticalSection(&win32Section);
}

void Dispatcher::addInterrupt(InterruptSource *source)
{
	source->next = sources;
	sources = source;
}

void Dispatcher::notify()
{
	SetEvent(interruptEvent);
}

void Dispatcher::start()
{
	ResumeThread(thread);
}

DWORD WINAPI Dispatcher::threadProc(LPVOID param)
{
	((Dispatcher *)param)->main();
	return 0;
}

void Dispatcher::main()
{
	for (;;) {
		// check if interrupts are pending
		InterruptSource *irq = NULL;
		irq = sources;
		while (irq) {
			if (irq->isEnabled() && irq->isTriggered())
				break;
			irq = irq->next;
		}
		if (irq == NULL && !Scheduler::_needsReschedule) {
			if (Scheduler::_currentThread != NULL)
				ResumeThread(Scheduler::_currentThread->context.handle);
			else {
				EnterCriticalSection(&sleepSection);
				withinSleep = true;
				QueryPerformanceCounter(&lastSleepTimer);
				LeaveCriticalSection(&sleepSection);
			}
			WaitForSingleObject(interruptEvent, INFINITE);
			if (Scheduler::_currentThread != NULL) {
				EnterCriticalSection(&win32Section);
				SuspendThread(Scheduler::_currentThread->context.handle);
				LeaveCriticalSection(&win32Section);
			}
			else {
				EnterCriticalSection(&sleepSection);
				withinSleep = false;
				LARGE_INTEGER end;
				QueryPerformanceCounter(&end);
				sleepingCycles += end.QuadPart - lastSleepTimer.QuadPart;
				LeaveCriticalSection(&sleepSection);
			}
		}
		else {
			while (irq && !interruptsEnabled || Scheduler::_needsReschedule && Scheduler::_threadSwitchDisabled) {
				assert(Scheduler::_currentThread != NULL);
				ResumeThread(Scheduler::_currentThread->context.handle);
				Sleep(5);
				EnterCriticalSection(&win32Section);
				SuspendThread(Scheduler::_currentThread->context.handle);
				LeaveCriticalSection(&win32Section);
			}
			if (irq && interruptsEnabled) {
				// interrupt current thread and service IRQ
				Scheduler::_irqNestingLevel++;
				irq->disable();
				irq->runHandler();
				irq->clear(); // auto acknowledge interrupt
				irq->enable();
				--Scheduler::_irqNestingLevel;
			}
			if (Scheduler::_needsReschedule && !Scheduler::_threadSwitchDisabled) {
				SysThread *threadToRun = Scheduler::_getThreadToRun();
				if (threadToRun == NULL || threadToRun->next == threadToRun)
					Scheduler::_schedulerTimer.cancel(); // if 0 or 1 thread running, pause the scheduler timer
				else if (threadToRun != Scheduler::_currentThread)
					Scheduler::_schedulerTimer.trigger();
				Scheduler::_currentThread = threadToRun;
				Scheduler::_needsReschedule = false;
			}
		}
	}
}

Dispatcher dispatcher;

}

namespace Scheduler
{

DWORD WINAPI userThreadProc(LPVOID param)
{
	SysThread::threadLauncher();
	return 0;
}

void _createThreadContext(SysThread *thread)
{
	thread->context.handle = CreateThread(NULL, 0, userThreadProc, NULL, CREATE_SUSPENDED, NULL);
	// Note that we don't actually use the stack that has been allocated by the SysThread constructor.
	// This is just wasted space. We can't use it because some system operations would fail when used
	// from a custom stack (mainly exception handling). We don't free it either: this way, the memory
	// usage is closer to what it actually is on the real hardware.
}

void _deleteThreadContext(SysThread *thread)
{
	CloseHandle(thread->context.handle);
}

}

namespace SysCallsImpl
{

void reschedule()
{
	Win32::dispatcher.notify();
	Sleep(5);
}

}

}

