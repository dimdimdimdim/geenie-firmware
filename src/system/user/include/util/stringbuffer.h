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
#include <util/utf8.h>

class StringBuffer : public Object
{
	GC_INHERITS(Object);

public:
	virtual void gcMarkFields()	const
	{
		gcMarkObject(first);
		AdditionalBuffer *buf = add;
		while (buf) {
			gcMarkObject(buf);
			buf = buf->next;
		}
	}

private:
	struct AdditionalBuffer
	{
		AdditionalBuffer *next;
		char chars[32];
	};
	AdditionalBuffer *add, **lastAdd;
	char *first;
	size_t firstSize;
	size_t length;
	size_t maxLen;

public:
	StringBuffer(int maxLen = -1)
	{
		first = gcnew(char[8]);
		firstSize = 8;
		add = NULL;
		lastAdd = &add;
		length = 0;
		this->maxLen = maxLen;
	}

	bool append(const char *chars, size_t l);

	bool append(const char *chars)
	{
		const char *e = chars;
		while (*e != '\0')
			e++;
		return append(chars, e - chars);
	}

	bool append(UTF8Encoder &utf8)
	{
		return append(utf8.getChars(), utf8.getLength());
	}

	bool append(char c)
	{
		return append(&c, 1);
	}
	
	bool append(const String &str)
	{
		return append(str.getChars(), str.getLength());
	}
	
	gc<String> getString();

	size_t getLength() const
	{
		return length;
	}

	char &operator[](size_t i) const
	{
		if (i < 32)
			return first[i];
		i -= 32;
		AdditionalBuffer *b = add;
		while (i >= 32) {
			i -= 32;
			b = b->next;
		}
		return b->chars[i];
	}

};
