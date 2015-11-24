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
#include <util/string.h>
#include <util/utf8.h>

gc<String> String::build(const char *chars, size_t length)
{
	gc<char *> newchars = gcnew(char[length]);
	memcpy(newchars.get(), chars, length);
	return gc<String>(String(newchars, length));
}

gc<String> String::operator+(const String &s) const
{
	if (s.getLength() == 0)
		return *this;
	if (getLength() == 0)
		return s;
	gc<char *> newchars = gcnew(char[length + s.length]);
	memcpy(newchars.get(), chars, length);
	memcpy(newchars.get() + length, s.chars, s.length);
	return String(newchars, length + s.length);
}

gc<String> String::part(size_t from, size_t length) const
{
	assert(from + length <= this->length);
	if (!Kernel::isInRam(chars))
		return String(chars + from, length);
	if (length == 0)
		return String();
	if (from == 0 && (this->length - length < 10 || length > this->length * 3 / 4)) /* reuse same buffer if not too much space wasted */
		return String(chars, length);
	gc<char *> newchars = gcnew(char[length]);
	memcpy(newchars.get(), chars + from, length);
	return String(newchars, length);
}

int String::compare(const String &s) const
{
	size_t i = 0;
	size_t l = getLength();
	if (l > s.getLength())
		l = s.length;
	while (i < l) {
		if (chars[i] != s.chars[i])
			return chars[i] < s.chars[i] ? -1 : 1;
		i++;
	}
	if (length == s.length)
		return 0;
	return length < s.length ? -1 : 1;
}

int String::Iterator::getForward()
{
	UTF8Decoder decoder;
	int uchar;
	const char *c = string->getChars();
	for (;;) {
		if (pos == string->getLength())
			return -1;
		if (decoder.decode(c[pos++], uchar))
			return uchar;
	}
}

int String::Iterator::getBackward()
{
	UTF8Decoder decoder;
	int uchar;
	const char *c = string->getChars();
	for (;;) {
		if (pos == 0)
			return -1;
		if (decoder.decodeBackward(c[--pos], uchar))
			return uchar;
	}
}
