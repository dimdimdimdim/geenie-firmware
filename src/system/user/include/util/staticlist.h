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

template<class L, typename... Others> struct StaticListBuilder
{
	static constexpr int countItems()	{ return 0; }
	static void buildList(L *list)		{ }
};

template<class L, typename First, typename... Others> struct StaticListBuilder<L, First, Others...>
{
	static constexpr int countItems()	{ return StaticListBuilder<L, Others...>::countItems() + 1; }
	static void buildList(L *list)		{ list->append(First::get()); StaticListBuilder<L, Others...>::buildList(list); }
};

template<typename First, typename... Others> struct StaticList : private StaticListBuilder<typename First::ListType, First, Others...>
{
	static constexpr int getLength()
	{
		return StaticListBuilder<typename First::ListType, First, Others...>::countItems();
	}

	static gc<const typename First::ListType *> getList()
	{
		gc<typename First::ListType *> list = gcnew(typename First::ListType);
		StaticListBuilder<typename First::ListType, First, Others...>::buildList(list);
		return list;
	}
};
