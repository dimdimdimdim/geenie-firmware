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

class GapStringBuffer : public Object
{
	GC_INHERITS(Object);

private:
	static const size_t bufferSize = 64;
	struct Buffer
	{
		Buffer *next, *prev;
		char chars[bufferSize];
	};
	Buffer *first, *gapBuf;
	size_t length, gapPos, gapLen;

public:
	virtual void gcMarkFields()	const
	{
		Buffer *buf = first;
		while (buf) {
			gcMarkObject(buf);
			buf = buf->next;
		}
	}

public:
	GapStringBuffer();
	GapStringBuffer(const String &string); // cursor is initially at the end of the string
	size_t getLength();
	size_t getCursorPos();

	void clear();

	char getByteAtCursor();
	char moveCursorRightSingleByte();
	char moveCursorLeftSingleByte();
	char eraseToRightSingleByte();
	char eraseToLeftSingleByte();
	void insert(const char *chars, size_t l);
	void insert(const String &string)			{ insert(string.getChars(), string.getLength()); }
	void insert(int unicodeChar)				{ UTF8Encoder enc(unicodeChar); insert(enc.getChars(), enc.getLength()); }

	class BytesIterator
	{
	private:
		GapStringBuffer *obj;
		Buffer *buf;
		size_t pos;
	public:
		BytesIterator()
		{
			buf = NULL;
		}
		BytesIterator(const BytesIterator &i)
		{
			obj = i.obj;
			buf = i.buf;
			pos = i.pos;
		}
		BytesIterator(GapStringBuffer *obj, Buffer *buf, size_t pos)
		{
			this->obj = obj;
			this->buf = buf;
			this->pos = pos;
		}
		BytesIterator &operator=(const BytesIterator &i)
		{
			obj = i.obj;
			buf = i.buf;
			pos = i.pos;
			return *this;
		}

		int getForward();
		int getBackward();
		int getPosition() const				{ return pos == (size_t)-1 ? obj->getLength() : pos > obj->gapPos ? pos - obj->gapLen : pos; }
	};

// UTF-8 versions of the above

	int getCharAtCursor();
	int moveCursorRight();
	int moveCursorLeft();
	int eraseToRight();
	int eraseToLeft();

	class Iterator : public String::AbstractIterator
	{
	private:
		BytesIterator i;
	public:
		Iterator()						:i() { }
		Iterator(const Iterator &i)		:i(i.i) { }
		Iterator(GapStringBuffer *obj, Buffer *buf, size_t pos) :i(obj, buf, pos) { }

		Iterator &operator=(const Iterator &i) { this->i = i.i; return *this; }

		virtual int getForward();
		virtual int getBackward();
		virtual int getPosition() const				{ return i.getPosition(); }
		virtual size_t getObjectSize() const		{ return sizeof(Iterator); };
	};

	Iterator cursor()
	{
		return Iterator(this, gapBuf, gapPos);
	}

	Iterator start()
	{
		return Iterator(this, first, 0);
	}
};
