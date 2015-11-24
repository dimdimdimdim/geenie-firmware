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

#include <util/list.h>
#include <thread.h>
#include <criticalsection.h>
#include <waitableevent.h>
#include <application.h>
#include <events.h>

class Timer : public Event
{
	GC_INHERITS(Event);
	GC_FIELDS(app, handler);
public:
	typedef void (Object::*CallbackMethod)();
private:
	WeakRef<Application> app;
	WeakRef<Object> handler;
	CallbackMethod callback;
	long targetTime;
	long periodicDelay;
	bool canceled;
	friend class System;
public:
	Timer(Object *handler, CallbackMethod callback);
	void set(long delay, bool periodic);
	void cancel() { canceled = true; }
	virtual void handle();
};

class System : public Thread
{
	GC_INHERITS(Thread);
	GC_FIELDS(restartEvent, apps, timers, envs);

private:
	CriticalSection section;
	WaitableEvent restartEvent;
	List<Application *> apps;
	List<Timer *> timers;
	Environment *envs[1];
	Environment *activeEnv;

	bool queueTimer(Timer *timer);

	System();

protected:
	virtual int runProc();

public:
	static System *get();

	Environment *getActiveEnv()				{ return activeEnv; }
	void registerApp(Application *app);
	void registerTimer(Timer *timer);
};
