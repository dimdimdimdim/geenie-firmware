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

template<class T, int size> class CircArray : public Object
{
	GC_INHERITS(Object);

private:
	T items[size];
	int start, length;

public:

	virtual void gcMarkFields()	const
	{
		for (int i = 0; i < size; i++)
			gcMarkObject(items[i]);
	}

	virtual void gcClearWeakFields() const
	{
		for (int i = 0; i < size; i++)
			gcClearWeakField(items[i]);
	}

public:

	CircArray() :items()
	{
		start = 0;
		length = 0;
	}

	int getLength() const
	{
		return length;
	}

	const T &getAt(int i) const
	{
		assert(i >= 0 && i < length);
		i += start;
		if (i >= size)
			i -= size;
		return items[i];
	}

	T &getAt(int i)
	{
		assert(i >= 0 && i < length);
		i += start;
		if (i >= size)
			i -= size;
		return items[i];
	}

	void setAt(int i, const T &object)
	{
		assert(i >= 0 && i < length);
		i += start;
		if (i >= size)
			i -= size;
		items[i] = object;
	}

	void insertAt(int i, const T &object)
	{
		assert(length < size && i >= 0 && i <= length);
		if (i < length / 2) {
			// shift items from start to index
			i += start;
			if (i >= size)
				i -= size;
			if (--start < 0)
				start += size;
			int j = start;
			//assert(items[j] == NULL);
			int nj;
			while (j != i) {
				nj = j + 1;
				if (nj >= size)
					nj = 0;
				items[j] = items[nj];
				j = nj;
			}
			items[j] = object;
			length++;
		}
		else {
			// shift items from end to index
			i += start;
			if (i >= size)
				i -= size;
			int j = start + length;
			if (j >= size)
				j -= size;
			//assert(items[j] == NULL);
			int nj;
			while (j != i) {
				nj = j - 1;
				if (nj < 0)
					nj = size - 1;
				items[j] = items[nj];
				j = nj;
			}
			items[j] = object;
			length++;
		}
	}

	void removeAt(int i)
	{
		assert(length > 0 && i >= 0 && i < length);
		if (i < length / 2) {
			// shift items from index to start
			int j = start + i;
			if (j >= size)
				j -= size;
			i = start;
			int nj;
			while (j != i) {
				nj = j - 1;
				if (nj < 0)
					nj = size;
				items[j] = items[nj];
				j = nj;
			}
			items[j] = T();
			if (++start == size)
				start = 0;
			length--;
		}
		else {
			// shift items from index to end
			int j = start + i;
			if (j >= size)
				j -= size;
			i = start + length - 1;
			if (i >= size)
				i -= size;
			int nj;
			while (j != i) {
				nj = j + 1;
				if (nj >= size)
					nj = 0;
				items[j] = items[nj];
				j = nj;
			}
			items[j] = T();
			length--;
		}
	}

	void pushFront(const T &object)
	{
		assert(length < size);
		if (--start < 0)
			start += size;
		length++;
		items[start] = object;
	}

	const T &getFront() const
	{
		assert(length > 0);
		return items[start];
	}

	T &getFront()
	{
		assert(length > 0);
		return items[start];
	}

	void removeFront()
	{
		assert(length > 0);
		items[start] = T();
		if (++start == size)
			start = 0;
		length--;
	}

	void pushBack(const T &object)
	{
		assert(length < size);
		int i = start + length++;
		if (i >= size)
			i -= size;
		items[i] = object;
	}

	const T &getBack() const
	{
		assert(length > 0);
		int i = start + length - 1;
		if (i >= size)
			i -= size;
		return items[i];
	}

	T &getBack()
	{
		assert(length > 0);
		int i = start + length - 1;
		if (i >= size)
			i -= size;
		return items[i];
	}

	void removeBack()
	{
		assert(length > 0);
		int i = start + length - 1;
		if (i >= size)
			i -= size;
		items[i] = T();
		length--;
	}

	int findIndexOf(const T &object)
	{
		int n = 0;
		int i = start;
		int l = length;
		while (l--) {
			if (items[i] == object)
				return n;
			if (++i == size)
				i = 0;
			n++;
		}
		return -1;
	}

};
