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

#include <thread.h>
#include <environment.h>
#include <events.h>
#include <messagequeue.h>

class Thread;
class Application;
class ApplicationCanvas;
class KeyAction;
class Menu;

namespace Graphics
{
class DisplayItem;
}

class AppMainThread : public Thread
{
	GC_INHERITS(Thread);
	GC_FIELDS(app);
private:
	Application *app;
protected:
	virtual int runProc();
public:
	AppMainThread(Application *app) { this->app = app; }
};

class Application : public ThreadGroup
{
	GC_INHERITS(ThreadGroup);
	GC_FIELDS(env, name, mainThread);

private:
	Environment *env;
	String name;
	AppMainThread *mainThread;
	WaitItem mainThreadWait;
	MessageQueue<Event> *mqueue;
	ApplicationCanvas *canvas;
	int kbdMode;
	bool headerVisible, menuVisible;
	const Menu *currentMenu;
	int currentMenuPage;

	friend class AppMainThread;
	friend class System;

protected:
	void setDisplayContents(Graphics::DisplayItem *item);

public:
	Application(const String &name, bool headerVisible = true, bool menuVisible = true);
	
	void run(Environment *env);
	
	Environment *getEnv()								{ return env; }
	const String &getName()								{ return name; }

	bool isMenuVisible()								{ return menuVisible; }
	bool isHeaderVisible()								{ return headerVisible; }
	int getKeyboardMode()								{ return kbdMode; }
	const Menu *getCurrrentMenu()						{ return currentMenu; }
	const int getCurrentMenuPage()						{ return currentMenuPage; }
	void setCurrentMenu(const Menu *menu);

	void sendEvent(Event *evt, bool nowait = false)		{ mqueue->sendMessage(evt, nowait); }

	virtual void onInit() { }
	virtual void onRedraw(int top, int height);
	virtual void onKeyDown(uint8_t keyCode);
	virtual void onKeyUp(uint8_t keyCode);
	virtual void onKeyPress(const KeyAction *action, int mode) { }

};

inline Application *getCurrentApplication()
{
	ThreadGroup *group = getCurrentThreadGroup();
	if (group == NULL)
		return NULL;
	return group->dynCast<Application>();
}
