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

#include "cobject.h"

namespace Calc
{

class CInteger : public CObject
{
	GC_INHERITS(CObject);

private:
	uint64_t value;

public:
	CInteger(uint64_t value) { this->value = value; }

	uint64_t getValue() const { return value; }

	virtual gc<String> toString(LocalVariables *locals = NULL) const;
	static bool canParseString(int initialChar, String::AbstractIterator &nextChars) { return initialChar == '#'; }
	static gc<const CObject *> parseString(String::AbstractIterator &stri);

	virtual int getObjectIdNumber() const;
	virtual void serializeData(IO::OutputStream *stream) const;
	static gc<const CObject *> deserializeData(IO::InputStream *stream);
};

};
