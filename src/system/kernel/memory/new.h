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

#ifdef _MSC_VER
#pragma push_macro("new")
#undef new
#endif

#ifdef _WIN32
#ifndef __PLACEMENT_NEW_INLINE
inline void *operator new(size_t, void *p)
{
	return p;
}
inline void operator delete(void *, void *)
{
}
#endif
#else
#include <new>
#endif
