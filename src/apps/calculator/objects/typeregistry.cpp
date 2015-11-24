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
#include <exception.h>
#include <util/stringbuffer.h>
#include <io/streams.h>
#include "cobject.h"
#include "command.h"
#include "../calculator.h"

#define NEEDS_HEADERS
#define REGISTER(x)
#include "alltypes.i"
#undef REGISTER
#undef NEEDS_HEADERS

namespace Calc
{

enum IdNumber
{
	IDNUM_NULL = 0,
#define REGISTER(x) IDNUM_##x,
#include "alltypes.i"
#undef REGISTER
	IDNUM_END
};

typedef gc<const CObject *> (* DeserializeDataFunc)(IO::InputStream *stream);
static DeserializeDataFunc deserializeFuncs[] = {
#define REGISTER(x) &x::deserializeData,
#include "alltypes.i"
#undef REGISTER
};

#define REGISTER(x) \
	int x::getObjectIdNumber() const { return IDNUM_##x - 1; }
#include "alltypes.i"
#undef REGISTER

void CObject::serialize(IO::OutputStream *stream) const
{
	int i = getObjectIdNumber();
	stream->writeFully(&i, sizeof(int));
	serializeData(stream);
}

gc<const CObject *> CObject::deserialize(IO::InputStream *stream)
{
	int i;
	stream->readFully(&i, sizeof(int));
	if (i >= IDNUM_FIRST_COMMAND)
		return findCommand(i);
	if (i == IDNUM_NULL || i >= IDNUM_END)
		gcthrownew(EInvalidObjectFormat);
	return deserializeFuncs[i - 1](stream);
}

gc<const Command *> parseCommand(int initialChar, String::AbstractIterator &nextChars, bool algebraicMode, int minPrecedence)
{
	gc<StringBuffer> strbuf;
	int un = 1;
	bool optr = algebraicMode && isOperator(initialChar);
	while (initialChar != -1 && (optr ? isOperator(initialChar) : !isSeparator(initialChar))) {
		UTF8Encoder utf8(initialChar);
		strbuf.append(utf8);
		initialChar = nextChars.getForward();
		if (algebraicMode && initialChar == '-')
			break;
	}
	if (initialChar != -1)
		nextChars.getBackward();
	gc<const Command *> cmd = findCommand(strbuf.getString());
	if (cmd && cmd->getPrecedence() >= minPrecedence)
		return cmd;
	for (int i = 0; i < un; i++)
		nextChars.getBackward();
	if (cmd)
		return NULL;
	gcthrownew(ECommandNotFound);
}

gc<const CObject *> parseSingleObject(int initialChar, String::AbstractIterator &nextChars, bool algebraicMode, int minPrecedence)
{
#define REGISTER(x) \
	if (x::canParseString(initialChar, nextChars)) { nextChars.getBackward(); return x::parseString(nextChars); }
#include "alltypes.i"
#undef REGISTER
	return parseCommand(initialChar, nextChars, algebraicMode, minPrecedence);
}

}
