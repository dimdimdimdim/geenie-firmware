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

#include <object.h>

template<class T> class Array
{
private:
	T *items;
	size_t length;

public:

	void gcMarkFields()	const
	{
		if (items) {
			Kernel::Allocator::mark(items);
			for (size_t i = 0; i < length; i++)
				gcMarkObject(items[i]);
		}
	}

public:

	Array()
	{
		length = 0;
		items = NULL;
	}

	Array(size_t length)
	{
		this->length = length;
		items = length == 0 ? NULL : gcnew(T[length]);
	}
	
	Array(const Array<T> &obj)
	{
		length = obj.length;
		items = obj.items;
	}

	Array<T> &operator=(const Array<T> &obj)
	{
		length = obj.length;
		items = obj.items;
		return *this;
	}
	
	void alloc(size_t length)
	{
		this->length = length;
		items = length == 0 ? NULL : gcnew(T[length]);
	}

	size_t getLength() const
	{
		return length;
	}

	const T &operator[](size_t i) const
	{
		return items[i];
	}

	T &operator[](size_t i)
	{
		return items[i];
	}

};

template<class T> class GCMarker<Array<T>>				{ public: static void mark(const Array<T> &x)			{ x.gcMarkFields(); } };
template<class T> class GCMarker<const Array<T>>		{ public: static void mark(const Array<T> &x)			{ x.gcMarkFields(); } };
