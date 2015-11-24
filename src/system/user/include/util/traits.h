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

template<bool cond, typename IfType, typename ElseType> struct TemplateIf
{
	typedef ElseType Value;
};

template<typename IfType, typename ElseType> struct TemplateIf<true, IfType, ElseType>
{
	typedef IfType Value;
};

template <typename T, typename T2> class IsTypeSameAs
{
public:
	static const bool value = false;
};

template <typename T> class IsTypeSameAs<T, T>
{
public:
	static const bool value = true;
};

template<typename T, typename B> struct IsTypeDerivedFrom
{
private:
	typedef char yes[1];
	typedef char no[2];
	static yes& ____test(B*);
	static no& ____test(...);
public:
	static const bool value = sizeof(____test(static_cast<T*>(0))) == sizeof(yes);
};
