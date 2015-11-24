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
#include <application.h>
#include <system.h>
#include <keyboard.h>
#include <menu.h>
#include <graphics/displayitem.h>
#include <graphics/menu.h>

#define HEADER_HEIGHT	31

static Graphics::Menu *menuDisp = NULL;

int AppMainThread::runProc()
{
	app->join();
	try {
		app->onInit();
		gc<Event *> evt;
		for (;;) {
			evt = app->mqueue->getMessage();
			evt->handle();
		}
	}
	catch (Exception *ex) {
		LOG(SYS, ERROR, "Application %s main thread (%p) died because of an uncatched exception: %s\n", app->getName(), this, ex->getMessage());
	}
	catch (...) {
		LOG(SYS, CRIT, "Application %s main thread (%p) died because an unsupported exception was throw\n", app->getName(), this);
	}
	return 0;
}

class ApplicationCanvas : public Graphics::DisplayItem
{
	GC_INHERITS(DisplayItem);
	GC_FIELDS(contents);

private:
	DisplayItem *contents;
	Application *app;
	Environment *env;

public:
	ApplicationCanvas(Environment *env, Application *app)
	{
		this->env = env;
		this->app = app;
		if (menuDisp == NULL) {
			static gc<Graphics::Menu *, GCGlobalItem> menuInstance = gcnew(Graphics::Menu(env->getDisplayWidth()));
			menuInstance->y = env->getDisplayHeight() - menuInstance->getHeight();
			menuDisp = menuInstance;
		}
	}

	virtual void initLineScanning(int line, int drawOffset, int drawWidth) { }
	virtual void processLine(uint8_t *pixelData, int dataOffset) { }

	void setContents(Graphics::DisplayItem *item)
	{
		assert(item->parent == NULL);
		if (contents)
			contents->parent = NULL;
		contents = item;
		item->parent = this;
		item->x = 0;
		item->y = app->isHeaderVisible() ? HEADER_HEIGHT : 0;
		item->setSize(env->getDisplayWidth(), env->getDisplayHeight() - (app->isMenuVisible() ? menuDisp->getHeight() : 0) - (app->isHeaderVisible() ? HEADER_HEIGHT : 0));
		redraw(0, env->getDisplayHeight());
	}

	virtual bool isVisible()
	{
		return System::get()->getActiveEnv()->getActiveApp() == getCurrentApplication();
	}

	virtual void redraw(int top, int height)
	{
		env->draw(contents, top, height);
		if (app->isMenuVisible())
			env->draw(menuDisp, top, height);
	}
};

Application::Application(const String &name, bool headerVisible, bool menuVisible)
{
	this->name = name;
	this->menuVisible = menuVisible;
	this->headerVisible = headerVisible;
	mqueue = gcnew(MessageQueue<Event>());
	kbdMode = 0;
}

void Application::run(Environment *env)
{
	this->env = env;
	System::get()->registerApp(this);
	getEnv()->activate(this);
	mainThread = gcnew(AppMainThread(this));
	canvas = gcnew(ApplicationCanvas(env, this));
	mainThread->start();
}

void Application::setCurrentMenu(const Menu *menu)
{
	currentMenu = menu;
	currentMenuPage = 0;
	if (isMenuVisible())
		env->draw(menuDisp);
}

void Application::setDisplayContents(Graphics::DisplayItem *item)
{
	canvas->setContents(item);
}

void Application::onRedraw(int top, int height)
{
	canvas->redraw(top, height);
}

void Application::onKeyDown(uint8_t keyCode)
{
	const KeyAction *action = getKeyAction(keyCode, kbdMode);
	onKeyPress(action, kbdMode);
	if (action->isSpecial()) {
		if (action->getId() == KeyAction::SHIFT)
			kbdMode |= Kbd::SHIFT;
		else if (action->getId() == KeyAction::ALPHA)
			kbdMode |= Kbd::ALPHA;
		else if (action->getId() == KeyAction::ON)
			kbdMode |= Kbd::META;
	}
}

void Application::onKeyUp(uint8_t keyCode)
{
	const KeyAction *action = getKeyAction(keyCode, kbdMode);
	if (action->isSpecial()) {
		if (action->getId() == KeyAction::SHIFT)
			kbdMode &= ~Kbd::SHIFT;
		else if (action->getId() == KeyAction::ALPHA)
			kbdMode &= ~Kbd::ALPHA;
		else if (action->getId() == KeyAction::ON)
			kbdMode &= ~Kbd::META;
	}
}

