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
#include <util/macros.h>
#include <util/statichashtable.h>
#include <util/list.h>
#include <menu.h>

/*	Oh my god ! That file is full of ugly C preprocessor trickery !
	Yeah, I'll most certainly go to hell just because of this. I don't care. It does the job.
*/

class BuiltinMenu : public Menu
{
	GC_INHERITS(Menu);
	GC_FIELDS(name);

private:
	String name;
	int itemsCount;
	const MenuItem * const *items;

public:
	constexpr BuiltinMenu(const String &name, int itemsCount, const MenuItem * const *items) :name(name), itemsCount(itemsCount), items(items) { }
	virtual const String &getName()						{ return name; }
	virtual int getItemCount()							{ return itemsCount; }
	virtual gc<const List<const MenuItem *> *> getItems()
	{
		gc<List<const MenuItem *> *> list = gcnew(List<const MenuItem *>);
		for (int i = 0; i < itemsCount; i++)
			list->append(items[i]);
		return list.get();
	}
};

class BuiltinCommandMenuItem : public CommandMenuItem
{
	GC_INHERITS(CommandMenuItem);
	GC_FIELDS(name);

private:
	String name;

public:
	constexpr BuiltinCommandMenuItem(const String &name) :name(name) { }
	virtual const String &getName()						{ return name; }
};

struct Menus
{
#define REGISTER_MENU(id, s, ...)		static const BuiltinMenu menu_##id;
#include "allmenus.i"
#undef REGISTER_MENU
};

#define EXPAND_MENU(x)							T_MENU, x
#define EXPAND_COMMAND(x)						T_COMMAND, x
#define ITEMDECL_T_MENU(index, menuid, x)		static const MenuItem *const menuItem_##menuid##_##index = &Menus::menu_##x;
#define ITEMDECL_T_COMMAND(index, menuid, x)	static const BuiltinCommandMenuItem menuCmdObj_##menuid##_##index(x); \
												static const MenuItem *const menuItem_##menuid##_##index = &menuCmdObj_##menuid##_##index;
#define ITEMDECL4(index, menuid, t, x)			ITEMDECL_##t(index, menuid, x)
#define ITEMDECL3(...)							EXPAND(ITEMDECL4 LPAREN __VA_ARGS__ RPAREN)
#define ITEMDECL2(index, menuid, x)				ITEMDECL3(index, menuid, x)
#define ITEMDECL(index, menuid, x)				ITEMDECL2(index, menuid, EXPAND_##x)

#define ITEMENTRY(index, menuid, x)				menuItem_##menuid##_##index,

#define REGISTER_MENU(id, s, ...) \
	ITERATECOUNTERPARAM(ITEMDECL, id, __VA_ARGS__) \
	static const MenuItem * const menuItems_##id[] = { ITERATECOUNTERPARAM(ITEMENTRY, id, __VA_ARGS__) }; \
	const BuiltinMenu Menus::menu_##id(s, PP_NARGS(__VA_ARGS__), menuItems_##id);
#include "allmenus.i"
#undef REGISTER_MENU

#define DECLARE_MENU_CLASS(s, id, ret) \
	struct MenuHashEntry_##id { \
		typedef gc<const Menu *> ValueType; \
		static constexpr int keyHash = String(s).getHash(); \
		static bool checkKey(const String &key) { return key == s; } \
		static gc<const Menu *> getValue() { return ret; } \
	};

#define REGISTER_MENU(id, s, ...) \
	DECLARE_MENU_CLASS(s, id, &Menus::menu_##id)
#include "allmenus.i"
#undef REGISTER_MENU

struct DummyHashEntry {
	typedef gc<const Menu *> ValueType;
	static constexpr int keyHash = -1;
	static constexpr bool checkKey(const String &key) { return false; }
	static gc<const Menu *> getValue() { return NULL; }
};

typedef StaticHashTable<
#define REGISTER_MENU(id, s, ...)			MenuHashEntry_##id,
#include "allmenus.i"
#undef REGISTER_MENU
	DummyHashEntry
> MenuTable;

namespace Calc
{

gc<const Menu *> findMenu(const String &name)
{
	return MenuTable::get(name);
}

}
