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
#include <chars.h>
#include "../calculator.h"
#include "../../../math/math.h"
#include "cprogram.h"

namespace Calc
{

bool CProgram::buildString(StringBuffer &str, LocalVariables *locals) const
{
	if (!str.append(STR_OPENAQUOTE " "))
		return false;
	for (List<const CObject *>::Iterator i = cmds->start(); i; i++) {
		if (!(*i)->buildString(str, locals))
			return false;
		if (!str.append(" "))
			return false;
	}
	if (!str.append(STR_CLOSEAQUOTE))
		return false;
	return true;
}

bool CProgram::canParseString(int initialChar, String::AbstractIterator &nextChars)
{
	return initialChar == CHAR_OPENAQUOTE;
}

gc<const CObject *> CProgram::parseString(String::AbstractIterator &stri)
{
	int uchar = stri.getForward();
	if (uchar != CHAR_OPENAQUOTE)
		gcthrownew(EInvalidObjectValue);
	gc<List<const CObject *> *> cmds = CObject::parseSequence(stri, CHAR_CLOSEAQUOTE);
	return gcnew(CProgram(cmds));
}

void CProgram::serializeData(IO::OutputStream *stream) const
{
	int length = cmds->getLength();
	stream->write(&length, sizeof(length));
	for (List<const CObject *>::Iterator i = cmds->start(); i; i++)
		(*i)->serialize(stream);
}

gc<const CObject *> CProgram::deserializeData(IO::InputStream *stream)
{
	int length;
	stream->read(&length, sizeof(length));
	gc<List<const CObject *> *> cmds = gcnew(List<const CObject *>);
	for (int i = 0; i < length; i++)
		cmds->append(CObject::deserialize(stream));
	return gcnew(CProgram(cmds));
}

}