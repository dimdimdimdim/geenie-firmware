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
#include <util/gapstringbuffer.h>

GapStringBuffer::GapStringBuffer()
{
	first = gcnew(Buffer);
	first->next = NULL;
	first->prev = NULL;
	length = bufferSize;
	gapBuf = first;
	gapPos = 0;
	gapLen = bufferSize;
}

GapStringBuffer::GapStringBuffer(const String &string)
{
	size_t l = string.getLength();
	const char *chars = string.getChars();
	length = 0;
	first = NULL;
	gc<Buffer *> buf;
	Buffer *lastBuf = NULL;
	for (;;) {
		buf = gcnew(Buffer);
		buf->prev = lastBuf;
		buf->next = NULL;
		memBarrier();
		if (lastBuf)
			lastBuf->next = buf;
		if (!first)
			first = buf;
		lastBuf = buf;
		if (l < bufferSize) {
			memcpy(buf->chars, chars, l);
			gapBuf = buf;
			gapPos = length + l;
			gapLen = bufferSize - l;
			length += bufferSize;
			break;
		}
		else {
			memcpy(buf->chars, chars, bufferSize);
			l -= bufferSize;
			length += bufferSize;
		}
	}
}

void GapStringBuffer::clear()
{
	first->next = NULL;
	first->prev = NULL;
	length = bufferSize;
	gapBuf = first;
	gapPos = 0;
	gapLen = bufferSize;
}

size_t GapStringBuffer::getLength()
{
	return length - gapLen;
}

size_t GapStringBuffer::getCursorPos()
{
	return gapPos;
}

char GapStringBuffer::getByteAtCursor()
{
	assert(gapPos < length - gapLen);
	Buffer *nextCharBuf = gapBuf;
	size_t nextCharPos = (gapPos % bufferSize) + gapLen;
	while (nextCharPos >= bufferSize) {
		nextCharPos -= bufferSize;
		nextCharBuf = nextCharBuf->next;
	}
	return nextCharBuf->chars[nextCharPos];
}

char GapStringBuffer::moveCursorRightSingleByte()
{
	assert(gapPos < length - gapLen);
	size_t gapPosInBuf = gapPos % bufferSize;
	Buffer *nextCharBuf = gapBuf;
	size_t nextCharPos = gapPosInBuf + gapLen;
	while (nextCharPos >= bufferSize) {
		nextCharPos -= bufferSize;
		nextCharBuf = nextCharBuf->next;
	}
	char c = gapBuf->chars[gapPosInBuf] = nextCharBuf->chars[nextCharPos];
	if (gapPosInBuf == bufferSize - 1) {
		gapBuf = gapBuf->next;
		assert(gapBuf != NULL);
	}
	gapPos++;
	return c;
}

char GapStringBuffer::moveCursorLeftSingleByte()
{
	assert(gapPos > 0);
	gapPos--;
	size_t gapPosInBuf = gapPos % bufferSize;
	if (gapPosInBuf == bufferSize - 1) {
		gapBuf = gapBuf->prev;
		assert(gapBuf != NULL);
	}
	Buffer *nextCharBuf = gapBuf;
	size_t nextCharPos = gapPosInBuf + gapLen;
	while (nextCharPos >= bufferSize) {
		nextCharPos -= bufferSize;
		nextCharBuf = nextCharBuf->next;
	}
	return nextCharBuf->chars[nextCharPos] = gapBuf->chars[gapPosInBuf];
}

char GapStringBuffer::eraseToRightSingleByte()
{
	assert(gapPos < length - gapLen);
	int c = getByteAtCursor();
	gapLen++;
	return c;
}

char GapStringBuffer::eraseToLeftSingleByte()
{
	assert(gapPos > 0);
	gapPos--;
	size_t gapPosInBuf = gapPos % bufferSize;
	if (gapPosInBuf == bufferSize - 1) {
		gapBuf = gapBuf->prev;
		assert(gapBuf != NULL);
	}
	gapLen++;
	return gapBuf->chars[gapPosInBuf];
}

void GapStringBuffer::insert(const char *chars, size_t l)
{
	size_t gapPosInBuf = gapPos % bufferSize;
	if (l >= gapLen) { // we must ensure that gapLen is never 0, otherwise, we might end up with a NULL gapBuf pointer when moving the cursor to the end of the string
		// add first buf
		gc<Buffer *> buf = gcnew(Buffer);
		buf->prev = gapBuf;
		buf->next = gapBuf->next;
		memBarrier();
		gapBuf->next = buf;
		if (buf->next)
			buf->next->prev = buf;
		// copy chars if they were in the same buffer as the start of the gap
		size_t nextCharPos = gapPosInBuf + gapLen;
		if (nextCharPos < bufferSize)
			memcpy(buf->chars + nextCharPos, gapBuf->chars + nextCharPos, bufferSize - nextCharPos);
		// increase gapLen to reflect gap increasing
		gapLen += bufferSize;
		length += bufferSize;
		// now add additional buffers without copying, if required
		while (l >= gapLen) {
			buf = gcnew(Buffer);
			buf->prev = gapBuf;
			buf->next = gapBuf->next;
			memBarrier();
			gapBuf->next = buf;
			buf->next->prev = buf;
			gapLen += bufferSize;
			length += bufferSize;
		}
	}
	// now we have enough room, copy the characters
	while (l) {
		size_t w = bufferSize - gapPosInBuf;
		if (l >= w) {
			memcpy(gapBuf->chars + gapPosInBuf, chars, w);
			gapPosInBuf = 0;
			gapPos += w;
			gapLen -= w;
			gapBuf = gapBuf->next;
			l -= w;
			chars += w;
		}
		else {
			memcpy(gapBuf->chars + gapPosInBuf, chars, l);
			gapPosInBuf += l;
			gapPos += l;
			gapLen -= l;
			l = 0;
		}
	}
}

int GapStringBuffer::getCharAtCursor()
{
	Iterator i(this, gapBuf, gapPos);
	return i.getForward();
}

int GapStringBuffer::moveCursorRight()
{
	UTF8Decoder decoder;
	char c;
	int uc;
	for (;;) {
		if (gapPos == length - gapLen)
			return -1;
		c = moveCursorRightSingleByte();
		if (decoder.decode(c, uc))
			return uc;
	}
}

int GapStringBuffer::moveCursorLeft()
{
	UTF8Decoder decoder;
	char c;
	int uc;
	for (;;) {
		if (gapPos == 0)
			return -1;
		c = moveCursorLeftSingleByte();
		if (decoder.decodeBackward(c, uc))
			return uc;
	}
}

int GapStringBuffer::eraseToRight()
{
	UTF8Decoder decoder;
	char c;
	int uc;
	for (;;) {
		if (gapPos == length - gapLen)
			return -1;
		c = eraseToRightSingleByte();
		if (decoder.decode(c, uc))
			return uc;
	}
}

int GapStringBuffer::eraseToLeft()
{
	UTF8Decoder decoder;
	char c;
	int uc;
	for (;;) {
		if (gapPos == 0)
			return -1;
		c = eraseToLeftSingleByte();
		if (decoder.decodeBackward(c, uc))
			return uc;
	}
}

int GapStringBuffer::BytesIterator::getForward()
{
	assert(buf != NULL);
	if (pos == (size_t)-1) // special case when we reach the end: we don't go till buf==NULL, so we can go back
		return -1;
	size_t posInBuf = pos % bufferSize;
	if (pos == obj->gapPos) {
		posInBuf += obj->gapLen;
		while (posInBuf >= bufferSize) {
			posInBuf -= bufferSize;
			if (buf->next == NULL) {
				pos = (size_t)-1;
				break;
			}
			buf = buf->next;
		}
		if (pos == (size_t)-1)
			return -1;
		pos += obj->gapLen;
	}
	int c = buf->chars[posInBuf] & 0xFF;
	pos++;
	if (posInBuf == bufferSize - 1) {
		if (buf->next == NULL)
			pos = (size_t)-1;
		else
			buf = buf->next;
	}
	return c;
}

int GapStringBuffer::BytesIterator::getBackward()
{
	assert(buf != NULL);
	if (pos == 0)
		return -1;
	bool noPrevBuf = false;
	if (pos == (size_t)-1) {
		pos = obj->length;
		noPrevBuf = true;
	}
	if (pos == obj->gapPos + obj->gapLen) {
		pos = obj->gapPos;
		buf = obj->gapBuf;
	}
	pos--;
	size_t posInBuf = pos % bufferSize;
	if (!noPrevBuf && posInBuf == bufferSize - 1)
		buf = buf->prev;
	return buf->chars[posInBuf] & 0xFF;
}

int GapStringBuffer::Iterator::getForward()
{
	UTF8Decoder decoder;
	int c, uc;
	for (;;) {
		c = i.getForward();
		if (c == -1)
			return -1;
		if (decoder.decode(c, uc))
			return uc;
	}
}

int GapStringBuffer::Iterator::getBackward()
{
	UTF8Decoder decoder;
	int c, uc;
	for (;;) {
		c = i.getBackward();
		if (c == -1)
			return -1;
		if (decoder.decodeBackward(c, uc))
			return uc;
	}
}

