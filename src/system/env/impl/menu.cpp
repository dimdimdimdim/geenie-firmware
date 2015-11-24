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
#include <menu.h>
#include <application.h>

void CommandMenuItem::onPress()
{
	Application *app = getCurrentApplication();
	KeyAction action = KeyAction::Command(getName());
	app->onKeyPress(&action, app->getKeyboardMode() | Kbd::FROM_MENU);
}

void Menu::onPress()
{
	Application *app = getCurrentApplication();
	app->setCurrentMenu(this);
}
