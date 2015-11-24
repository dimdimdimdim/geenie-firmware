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

#include <criticalsection.h>

class Display;
class Application;

namespace Graphics
{
	class DisplayItem;
}

class Environment : public Object
{
	GC_INHERITS(Object);
	GC_FIELDS(display, menuContents, headerContents);

private:
	CriticalSection section;
	Display *display;
	Graphics::DisplayItem *menuContents;
	Graphics::DisplayItem *headerContents;
	Application *activeApplication;

public:
	Environment(Display *display);

	int getDisplayWidth();
	int getDisplayHeight();
	void draw(Graphics::DisplayItem *contents, int top, int height);
	void draw(Graphics::DisplayItem *contents) { draw(contents, 0, getDisplayHeight()); }

	void activate(Application *app);	
	Application *getActiveApp() { return activeApplication; }
};
