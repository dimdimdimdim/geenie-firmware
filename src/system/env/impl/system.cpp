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
#include <util/list.h>
#include <io/device.h>
#include <io/printstream.h>
#include <graphics/font.h>
#include <graphics/text.h>
#include <display.h>
#include <system.h>
#include "../../../apps/calculator/calculator.h"

System::System()
:Thread(1024, maxPriority)
{
}

System *System::get()
{
	static gc<System *, GCGlobalItem> system = gcnew(System);
	return system;
}

void System::registerApp(Application *app)
{
	synchronized(section) {
		apps.append(app);
	}
}

int System::runProc()
{
	LOG(SYS, INFO, "Startup...\n");
	gc<Display *> display = Display::open(true);
	//gc<Graphics::Text *> text = gcnew(Graphics::Text(Graphics::Font::BIG, "Starting up..."));
	//display->draw(text);
	gc<IO::Device *> kbd = IO::Device::open("keyboard", IO::EXCLUSIVE | IO::NON_BLOCKING);
	gc<IO::Device *> kbdAux = NULL;
	try {
		kbdAux = IO::Device::open("keyboard:aux", IO::EXCLUSIVE | IO::NON_BLOCKING);
	}
	catch (...) {
	}
	activeEnv = NULL;
	for (size_t i = 0; i < sizeof(envs) / sizeof(envs[0]); i++) {
		envs[i] = gcnew(Environment(display));
		if (i == 0)
			activeEnv = envs[i];
		gc<Application *> app = gcnew(Calculator());
		app->run(envs[i]);
	}
	for (;;) {
		WaitList wlist;
		WaitItem waitRestart = restartEvent.onSet();
		wlist.append(waitRestart);
		WaitItem waitKbd = kbd->onReadReady();
		wlist.append(waitKbd);
		WaitItem waitKbdAux;
		if (kbdAux) {
			waitKbdAux = kbdAux->onReadReady();
			wlist.append(waitKbdAux);
		}
		List<Application *>::Iterator i;
		long timeOut = -1;
		synchronized(section) {
			for (i = apps.start(); i; i++) {
				(*i)->mainThreadWait = (*i)->mainThread->onTerminated();
				wlist.append((*i)->mainThreadWait);
			}
			while (timers.getLength() != 0) {
				gc<Timer *> timer = timers.getAt(0);
				timeOut = timer->targetTime - getTickCount();
				if (timeOut > 1)
					break;
				gc<Application *> app = timer->app.get();
				if (app != NULL && !timer->canceled && timer->periodicDelay != -1) {
					timer->targetTime += timer->periodicDelay;
					queueTimer(timer);
				}
				else {
					timers.removeAt(0);
				}
				if (!timer->canceled) {
					if (app != NULL)
						app->sendEvent(timer, true);
				}
			}
		}
		wait(wlist, timeOut);
		if (kbd->isReadReady()) {
			uint8_t keyCode;
			if (kbd->read(&keyCode, 1) == 1) {
				Application *app = activeEnv->getActiveApp();
				if (app)
					app->sendEvent(gcnew(KeyUpDownEvent(keyCode)));
			}
		}
		if (kbdAux && kbdAux->isReadReady()) {
			AuxKeyboardData keyData;
			if (kbdAux->read(&keyData, sizeof(keyData)) == sizeof(keyData)) {
				Application *app = activeEnv->getActiveApp();
				if (app)
					app->sendEvent(gcnew(KeyPressEvent(keyData.getKeyAction(), keyData.getMode())));
			}
		}
		synchronized(section) {
			i = apps.start();
			while (i) {
				if ((*i)->mainThread->isTerminated()) {
					gc<Application *> app = *i;
					Environment *env = app->getEnv();
					i.removeItem();
					if (app == env->getActiveApp()) {
						List<Application *>::Iterator j;
						for (j = apps.start(); j; j++) {
							if ((*j)->getEnv() == env) {
								env->activate(*j);
								break;
							}
						}
						if (!j) {
							LOG(SYS, ERROR, "No application on environment - restarting calculator");
							app = gcnew(Calculator());
							app->run(env);
						}
					}
				}
				else
					i++;
			}
		}
	}
	kbd->close();
	return 0;
}

Timer::Timer(Object *handler, Timer::CallbackMethod callback)
{
	this->app = getCurrentApplication();
	this->handler = handler;
	this->callback = callback;
	this->canceled = true;
}

void Timer::set(long delay, bool periodic)
{
	this->targetTime = getTickCount() + delay;
	this->periodicDelay = periodic ? delay : -1;
	this->canceled = false;
	System::get()->registerTimer(this);
}

void Timer::handle()
{
	if (!canceled) {
		gc<Object *> handler = this->handler.get();
		if (handler != NULL)
			((handler.get())->*callback)();
	}
}

bool System::queueTimer(Timer *timer)
{
	synchronized(section) {
		int l = timers.find(timer);
		if (l != -1) {
			// check if still at the right place in the list
			if ((l == 0 || timers.getAt(l - 1)->targetTime <= timer->targetTime) &&
				(l == timers.getLength() - 1 || timers.getAt(l + 1)->targetTime >= timer->targetTime)) {
				return l == 0; // restartEvent if it is the first in the list
			}
			// if not at the right place, remove the timer, we'll readd it
			timers.removeAt(l);
		}
		List<Timer *>::Iterator i;
		for (i = timers.start(); i; i++) {
			if ((*i)->targetTime > timer->targetTime) {
				i.insertItem(timer);
				return i == timers.start();
			}
		}
		if (!i) {
			timers.append(timer);
			return timers.getLength() == 1;
		}
	}
	return false;
}

void System::registerTimer(Timer *timer)
{
	if (queueTimer(timer))
		restartEvent.set();
}

