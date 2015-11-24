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
#include "../../../math/math.h"
#include "creal.h"

namespace Calc
{

bool CReal::buildString(StringBuffer &str, LocalVariables *locals) const
{
	return doubleToString(str, value, 0);
}

bool CReal::canParseString(int initialChar, String::AbstractIterator &nextChars)
{
	if (initialChar >= '0' && initialChar <= '9')
		return true;
	if (initialChar == '-' || initialChar == '+') {
		initialChar = nextChars.getForward();
		if (initialChar >= '0' && initialChar <= '9') {
			nextChars.getBackward();
			return true;
		}
		if (initialChar != -1)
			nextChars.getBackward();
	}
	return false;
}

gc<const CObject *> CReal::parseString(String::AbstractIterator &stri)
{
	double value = stringToDouble(stri);
	if (isnan(value))
		gcthrownew(EInvalidObjectValue(Message(InvalidRealValue)));
	return gcnew(CReal(value));
}

void CReal::serializeData(IO::OutputStream *stream) const
{
	stream->write(&value, sizeof(value));
}

gc<const CObject *> CReal::deserializeData(IO::InputStream *stream)
{
	double value;
	stream->read(&value, sizeof(value));
	return gcnew(CReal(value));
}

}