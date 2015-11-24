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
#include <util/formatter.h>

int getFlags(const String &fmtString, size_t &fmtIndex)
{
	int flags = 0;
	const char *fmt = fmtString.getChars();
	size_t len = fmtString.getLength();
	while (fmtIndex < len) {
		switch (fmt[fmtIndex]) {
		case '\'':
			flags |= FormatSpecifier::FLAG_APOST;
			break;
		case '-':
			flags |= FormatSpecifier::FLAG_MINUS;
			break;
		case '+':
			flags |= FormatSpecifier::FLAG_PLUS;
			break;
		case ' ':
			flags |= FormatSpecifier::FLAG_SPACE;
			break;
		case '#':
			flags |= FormatSpecifier::FLAG_HASH;
			break;
		case '0':
			flags |= FormatSpecifier::FLAG_ZERO;
			break;
		default:
			return flags;
		}
		fmtIndex++;
	}
	return flags;
}

static int getInt(const String &fmt, size_t &fmtIndex)
{
	size_t len = fmt.getLength();
	const char *pc = fmt.getChars() + fmtIndex;
	if (fmtIndex < len && *pc >= '0' && *pc <= '9') {
		int n = 0;
		do {
			n = n * 10 + *pc++ - '0';
		} while (++fmtIndex < len && *pc >= '0' && *pc <= '9');
		return n;
	}
	return -1;
}

FormatSpecifier::FormatSpecifier(int flags, int width, int precision, char type)
	:flags(flags), width(width), precision(precision), type(type)
{
}

FormatSpecifier::FormatSpecifier(const Formatter &formatter, size_t &fmtIndex, int &argsIndex)
{
	const String &fmt = formatter.getFormatString();
	argIndex = -1;
	size_t origIndex = fmtIndex;
	size_t len = formatter.getFormatString().getLength();
	int n = getInt(fmt, fmtIndex);
	if (n != -1 && fmtIndex < len && fmt[fmtIndex] == '$') {
		fmtIndex++;
		argIndex = n;
	}
	else {
		fmtIndex = origIndex; // go back
	}
	flags = getFlags(fmt, fmtIndex);
	if (fmtIndex < len && fmt[fmtIndex] == '*') {
		fmtIndex++;
		n = getInt(fmt, fmtIndex);
		const Formatter::Argument *a;
		if (n != -1 && fmtIndex < len && fmt[fmtIndex] == '$') {
			fmtIndex++;
			a = formatter.getArg(n - 1);
		}
		else
			a = formatter.getArg(argsIndex++);
		width = a ? a->asInt() : -1;
	}
	else
		width = getInt(fmt, fmtIndex);
	if (fmtIndex < len && fmt[fmtIndex] == '.') {
		fmtIndex++;
		if (fmtIndex < len && fmt[fmtIndex] == '*') {
			fmtIndex++;
			n = getInt(fmt, fmtIndex);
			const Formatter::Argument *a;
			if (n != -1 && fmtIndex < len && fmt[fmtIndex] == '$') {
				fmtIndex++;
				a = formatter.getArg(n - 1);
			}
			else
				a = formatter.getArg(argsIndex++);
			precision = a ? a->asInt() : -1;
		}
		else
			precision = getInt(fmt, fmtIndex);
	}
	else
		precision = -1;
	/* skip length specification : this does not make sense for a typesafe printf */
	if (fmtIndex < len && fmt[fmtIndex] == 'h') {
		fmtIndex++;
		if (fmtIndex < len && fmt[fmtIndex] == 'h')
			fmtIndex++;
	}
	else if (fmtIndex < len && fmt[fmtIndex] == 'l') {
		fmtIndex++;
		if (fmtIndex < len && fmt[fmtIndex] == 'l')
			fmtIndex++;
	}
	else if (fmtIndex < len && (fmt[fmtIndex] == 'j' || fmt[fmtIndex] == 'z' || fmt[fmtIndex] == 't' || fmt[fmtIndex] == 'L'))
		fmtIndex++;
	if (fmtIndex < len)
		type = fmt[fmtIndex++];
	else
		type = '\0';
	if (argIndex == -1)
		argIndex = argsIndex++;
}

void Formatter::print(Destination *out) const
{
	int argsIndex = 0;
	size_t fmtIndex = 0;
	const char *fmt = fmtString.getChars();
	size_t len = fmtString.getLength();
	while (fmtIndex < len) {
		size_t index2 = fmtIndex;
		while (index2 < len && fmt[index2] != '%')
			index2++;
		out->print(fmt + fmtIndex, index2 - fmtIndex);
		fmtIndex = index2;
		if (fmtIndex < len)
			fmtIndex++;
		if (fmtIndex < len) {
			if (fmt[fmtIndex] == '%') {
				out->print("%");
				fmtIndex++;
			}
			else {
				FormatSpecifier fmtspec(*this, fmtIndex, argsIndex);
				const Argument *a = getArg(fmtspec.getArgIndex());
				if (!a)
					out->print("[missing format arg]");
				else
					a->print(out, fmtspec);
			}
		}
	}
}

static const char *const zeroes = "0000000000000000";
static const char *const blanks = "                ";

static void printfPad(Formatter::Destination *out, const char *padding, int count)
{
	for (;;) {
		if (count > 16) {
			out->print(padding, 16);
			count -= 16;
		}
		else {
			out->print(padding, count);
			return;
		}
	}
}

void Formatter::printfInt(unsigned long long n, bool negative, Destination *out, FormatSpecifier &spec)
{
	// special case for 'c' type specifier
	if (spec.getType() == 'c') {
		int blanksL = spec.getWidth();
		if (blanksL == -1)
			blanksL = 0;
		else if (blanksL > 0)
			blanksL--;
		char c = negative ? (char)-(signed char)n : (char)n;
		if (spec.hasFlag(FormatSpecifier::FLAG_MINUS)) {
			out->print(&c, 1);
			printfPad(out, blanks, blanksL);
		}
		else {
			printfPad(out, blanks, blanksL);
			out->print(&c, 1);
		}
		return;
	}
	
	// check specifier type
	static const char *const upperChars = "0123456789ABCDEF";
	static const char *const lowerChars = "0123456789abcdef";
	const char *chars = lowerChars;
	int base;
	if (spec.getType() == 'd' || spec.getType() == 'i' || spec.getType() == 'u')
		base = 10;
	else if (spec.getType() == 'o')
		base = 8;
	else if (spec.getType() == 'x')
		base = 16;
	else if (spec.getType() == 'X') {
		base = 16;
		chars = upperChars;
	}
	else {
		out->print("[inconsistent format arg]");
		return;
	}
	
	// compute prefix (characters to be included before the 'precision' zero-padding)
	// this includes the sign and eventually the 0x prefix
	char prefix[4];
	char *pos = prefix;
	if (negative)
		*pos++ = '-';
	else {
		if (spec.hasFlag(FormatSpecifier::FLAG_PLUS))
			*pos++ = '+';
		else if (spec.hasFlag(FormatSpecifier::FLAG_SPACE))
			*pos++ = ' ';
	}
	if (spec.hasFlag(FormatSpecifier::FLAG_HASH)) {
		if (spec.getType() == 'x') {
			*pos++ = '0';
			*pos++ = 'x';
		}
		else if (spec.getType() == 'X') {
			*pos++ = '0';
			*pos++ = 'X';
		}
	}
	int prefixL = pos - prefix; // prefix length
	*pos++ = '\0';
	
	// convert integer to string (do it backward)
	char valueBuf[80];
	char *value = valueBuf + sizeof(valueBuf);
	*(--value) = '\0';
	pos = value; // to compute length
	while (n) {
		*(--value) = chars[n % base];
		n /= base;
	}
	if (spec.hasFlag(FormatSpecifier::FLAG_HASH) && spec.getType() == 'o')
		*(--value) = '0';
	int valueL = pos - value; // value string length
	
	// now apply padding and output whole stuff
	int zeroesL = (spec.getPrecision() == -1 ? 1 : spec.getPrecision()) - valueL;
	if (zeroesL < 0)
		zeroesL = 0;
	int blanksL = (spec.getWidth() == -1 ? 0 : spec.getWidth()) - zeroesL - valueL - prefixL;
	if (blanksL < 0)
		blanksL = 0;
	if (spec.hasFlag(FormatSpecifier::FLAG_MINUS)) {
		out->print(prefix, prefixL);
		printfPad(out, zeroes, zeroesL);
		out->print(value, valueL);
		printfPad(out, blanks, blanksL);
	}
	else if (spec.hasFlag(FormatSpecifier::FLAG_ZERO)) {
		out->print(prefix, prefixL);
		printfPad(out, zeroes, zeroesL + blanksL);
		out->print(value, valueL);
	}
	else {
		printfPad(out, blanks, blanksL);
		out->print(prefix, prefixL);
		printfPad(out, zeroes, zeroesL);
		out->print(value, valueL);
	}
}

void Formatter::Argument::print(Destination *out, FormatSpecifier &spec) const
{
	if (type == TYPE_INT) {
		printfInt(intValue, negative, out, spec);
	}
	else if (type == TYPE_POINTER || type == TYPE_POINTER_CHAR) {
		if (spec.getType() == 'p') {
			FormatSpecifier spec2(spec.hasFlag(FormatSpecifier::FLAG_MINUS) ? FormatSpecifier::FLAG_MINUS : 0, spec.getWidth(), sizeof(void *) * 2, 'x');
			printfInt((unsigned long)ptrValue, false, out, spec2);
		}
/*		else if (spec.getType() == 's' && type == TYPE_POINTER_CHAR) {
			out->print(String((const char *)ptrValue));
		}*/
		else if (spec.getType() == 'c' && type == TYPE_POINTER_CHAR) {
			out->print((const char *)ptrValue, 1);
		}
		else {
			out->print("[inconsistent format arg]");
		}
	}
	else if (type == TYPE_STRING) {
		if (spec.getType() == 's') {
			out->print(*stringRef);
		}
		else if (spec.getType() == 'c') {
			out->print(stringRef->getLength() > 0 ? stringRef->getChars() : " ", 1);
		}
		else {
			out->print("[inconsistent format arg]");
		}
	}
	else {
		out->print("[unsupported format arg]");
	}
}
