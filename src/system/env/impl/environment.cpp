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
#include <environment.h>
#include <application.h>
#include <system.h>
#include <display.h>

Environment::Environment(Display *display)
{
	this->display = display;
}

int Environment::getDisplayWidth()
{
	return display->getWidth();
}

int Environment::getDisplayHeight()
{
	return display->getHeight();
}

void Environment::draw(Graphics::DisplayItem *contents, int top, int height)
{
	if (contents != NULL && System::get()->getActiveEnv() == this)
		display->draw(contents, top, height);
}

void Environment::activate(Application *app)
{
	if (activeApplication != app) {
		assert(!app || app->getEnv() == this);
		activeApplication = app;
		if (app && System::get()->getActiveEnv() == this)
			app->sendEvent(gcnew(RedrawEvent(0, display->getHeight())), true);
	}
}
