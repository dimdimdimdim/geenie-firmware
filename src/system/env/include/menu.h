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

#include <util/string.h>
#include <util/list.h>
#include <keyboard.h>

#define MENU_PAGE_SIZE	6

class MenuItem : public Object
{
	GC_INHERITS(Object);

public:
	virtual bool isFolder()				{ return false; }
	virtual const String &getName() = 0;

	virtual void onPress() = 0;
};

class CommandMenuItem : public MenuItem
{
	GC_INHERITS(MenuItem)

public:
	virtual void onPress();
};

class Menu : public MenuItem
{
	GC_INHERITS(MenuItem);

public:
	virtual bool isFolder()				{ return true; }

	virtual gc<const List<const MenuItem *> *> getItems() = 0;
	virtual int getItemCount()			{ return getItems()->getLength(); }
	int getPageCount()					{ return (getItemCount() + MENU_PAGE_SIZE - 1) / MENU_PAGE_SIZE; }

	virtual void onPress();
};
