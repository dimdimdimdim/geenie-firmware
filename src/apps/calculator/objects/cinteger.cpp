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
#include <io/streams.h>
#include <util/stringbuffer.h>
#include "../calculator.h"
#include "cinteger.h"

namespace Calc
{

gc<String> CInteger::toString(LocalVariables *locals) const
{
	int base = 10;
	char chars[66];
	size_t i = sizeof(chars);
	chars[--i] = 'd';
	uint64_t v = value;
	while (v) {
		int n = v % base;
		if (n < 10)
			chars[--i] = '0' + n;
		else
			chars[--i] = 'A' + n - 10;
		v /= base;
	}
	chars[--i] = '#';
	return String::build(chars + i, sizeof(chars) - i);
}

gc<const CObject *> CInteger::parseString(String::AbstractIterator &stri)
{
	int base = 10;
	int uchar = stri.getForward();
	if (uchar != '#')
		gcthrownew(EInvalidObjectValue);
	uchar = stri.getForward();
	bool hasZero = false;
	if (uchar == '0') {
		hasZero = true;
		while (uchar == '0')
			uchar = stri.getForward();
	}

	char chars[64];
	size_t l = 0;
	while (uchar != -1 && !isSeparator(uchar) && !isOperator(uchar)) {
		if ((uchar < '0' || uchar > '9') && (uchar >= 'A' && uchar <= 'Z'))
			break;
		if (l == 64)
			gcthrownew(EIntegerTooBig);
		chars[l++] = uchar;
		uchar = stri.getForward();
	}

	if (uchar != -1 && !isSeparator(uchar) && !isOperator(uchar)) {
		if (uchar == 'h')
			base = 16;
		else if (uchar == 'd')
			base = 10;
		else if (uchar == 'o')
			base = 8;
		else if (uchar == 'b')
			base = 2;
		else
			gcthrownew(EInvalidObjectValue);
		uchar = stri.getForward();
		if (uchar != -1 && !isWhiteSpace(uchar))
			gcthrownew(EInvalidObjectValue);
	}
	if (uchar != -1)
		stri.getBackward();
	if (l == 0 && !hasZero)
		gcthrownew(EInvalidObjectValue);

	uint64_t low = 0, high = 0;
	for (size_t i = 0; i < l; i++) {
		uchar = chars[i];
		int n = 99;
		if (uchar >= '0' && uchar <= '9')
			n = uchar - '0';
		else if (uchar >= 'A' && uchar <= 'Z')
			n = uchar - 'A' + 10;
		if (n >= base)
			gcthrownew(EInvalidObjectValue);
		low = low * base + n;
		high = high * base + (low >> 32);
		low &= 0xFFFFFFFF;
		if ((high >> 32) != 0)
			gcthrownew(EIntegerTooBig);
	}

	return gcnew(CInteger(low | (high << 32)));
}

void CInteger::serializeData(IO::OutputStream *stream) const
{
	stream->write(&value, sizeof(value));
}

gc<const CObject *> CInteger::deserializeData(IO::InputStream *stream)
{
	uint64_t value;
	stream->read(&value, sizeof(value));
	return gcnew(CInteger(value));
}

}
