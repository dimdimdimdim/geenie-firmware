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
#include "cstring.h"

namespace Calc
{

bool CString::buildString(StringBuffer &buf, LocalVariables *locals) const
{
	if (!buf.append('"'))
		return false;
	const char *pc = value.getChars();
	for (size_t i = 0; i < value.getLength(); i++) {
		char c = *(pc++);
		if (c == '"') {
			if (!buf.append("\\\""))
				return false;
		}
		else if (c == '\\') {
			if (!buf.append("\\\\"))
				return false;
		}
		else if (c == '\r') {
			if (!buf.append("\\\r"))
				return false;
		}
		else if (c == '\n') {
			if (!buf.append("\\\n"))
				return false;
		}
		else if (c == '\t') {
			if (!buf.append("\\\t"))
				return false;
		}
		else {
			if (!buf.append(c))
				return false;
		}
	}
	if (!buf.append('"'))
		return false;
	return buf.getString();
}

gc<const CObject *> CString::parseString(String::AbstractIterator &stri)
{
	if (stri.getForward() != '"')
		gcthrownew(EInvalidObjectValue);
	StringBuffer buf;
	for (;;) {
		char c = stri.getForward();
		if (c == -1)
			gcthrownew(EInvalidObjectValue(Message(UnterminatedStringValue)));
		if (c == '"')
			break;
		if (c == '\\') {
			c = stri.getForward();
			if (c == '"')
				buf.append('"');
			else if (c == '\\')
				buf.append('\\');
			else if (c == 'r')
				buf.append('\r');
			else if (c == 'n')
				buf.append('\n');
			else if (c == 't')
				buf.append('\t');
			else if (c == -1)
				gcthrownew(EInvalidObjectValue(Message(UnterminatedStringValue)));
			else
				gcthrownew(EInvalidObjectValue(Message(UnrecognizedStringEscapeSequence)));
		}
		else
			buf.append(c);
	}
	return gcnew(CString(buf.getString()));
}

void CString::serializeData(IO::OutputStream *stream) const
{
	int length = value.getLength();
	stream->write(&length, sizeof(length));
	stream->write(value.getChars(), length);
}

gc<const CObject *> CString::deserializeData(IO::InputStream *stream)
{
	int length;
	stream->read(&length, sizeof(length));
	if (length == 0)
		return gcnew(CString(String()));
	gc<char *> chars = gcnew(char[length]);
	stream->read(chars, length);
	return gcnew(CString(String::make(chars, length)));
}

}
