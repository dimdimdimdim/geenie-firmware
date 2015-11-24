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

template<class T, int size> class CircBuffer
{

private:
	T items[size];
	int start, length;

public:

	CircBuffer() :items()
	{
		start = 0;
		length = 0;
	}

	int getLength() const
	{
		return length;
	}

	bool isFull() const
	{
		return length == size;
	}

	bool isEmpty() const
	{
		return length == 0;
	}

	size_t write(const T *data, size_t length)
	{
		size_t written = 0, i = start + this->length;
		while (this->length < size && length) {
			if (i >= size)
				i -= size;
			items[i++] = *(data++);
			this->length++;
			length--;
			written++;
		}
		return written;
	}

	size_t read(T *data, size_t length)
	{
		size_t read = 0;
		while (this->length > 0 && length) {
			if (start >= size)
				start -= size;
			*(data++) = items[start++];
			this->length--;
			length--;
			read++;
		}
		return read;
	}

};
