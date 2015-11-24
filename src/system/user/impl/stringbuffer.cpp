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
#include <util/stringbuffer.h>

bool StringBuffer::append(const char *chars, size_t l)
{
	if (l == 0)
		return true;
	if (maxLen != (size_t)-1 && length + l > maxLen) {
		append(chars, maxLen - length);
		return false;
	}
	assert(first != NULL);
	if (length + l <= firstSize) {
		memcpy(first + length, chars, l);
		length += l;
	}
	else if (length + l <= 32) {
		size_t newFirstSize = (length + l + 15) & ~15;
		gc<char *> newFirst = gcnew(char[newFirstSize]);
		memcpy(newFirst, first, length);
		memcpy(newFirst + length, chars, l);
		first = newFirst;
		firstSize = newFirstSize;
		length += l;
	}
	else if (length < 32) {
		int l2 = 32 - length - l;
		append(chars, l2);
		append(chars + l2, l - l2);
	}
	else {
		for (;;) {
			if (*lastAdd == NULL)
				*lastAdd = gcnew(AdditionalBuffer);
			size_t avail = 32 - length % 32;
			if (l > avail) {
				memcpy((*lastAdd)->chars + length % 32, chars, avail);
				length += avail;
				chars += avail;
				l -= avail;
				lastAdd = &(*lastAdd)->next;
			}
			else {
				memcpy((*lastAdd)->chars + length % 32, chars, l);
				length += l;
				if (length % 32 == 0)
					lastAdd = &(*lastAdd)->next;
				break;
			}
		}
	}
	return true;
}

gc<String> StringBuffer::getString()
{
	assert(first != NULL);
	if (add == NULL) {
		gc<char *> chars = first;
		first = NULL;
		return String::make(chars, length);
	}
	else {
		assert(length > 32);
		gc<char *> copy = gcnew(char[length]);
		memcpy(copy, first, 32);
		size_t l = length - 32;
		char *c = copy + 32;
		for (;;) {
			assert(add != NULL);
			if (l > 32) {
				memcpy(c, add->chars, 32);
				l -= 32;
				c += 32;
				add = add->next;
			}
			else {
				memcpy(c, add->chars, l);
				first = NULL;
				return String::make(copy, length);
			}
		}
	}
}
