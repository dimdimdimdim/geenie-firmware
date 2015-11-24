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

#include <util/array.h>
#include "cobject.h"
#include "command.h"

namespace Calc
{

class CAlgebraic : public CObject
{
	GC_INHERITS(CObject);
	GC_FIELDS(operatr, operands);

private:
	const Command *operatr;
	Array<const CObject *> operands;

	bool buildOperandString(StringBuffer &str, LocalVariables *locals, const CObject *operand) const;

public:
	CAlgebraic(const Command *operatr, const CObject * operand1);
	CAlgebraic(const Command *operatr, const CObject * operand1, const CObject *operand2);
	CAlgebraic(const Command *operatr, List<const CObject *> *operands);
	CAlgebraic(const Command *operatr, Array<const CObject *> &operands);

	const Command *getOperator()			{ return operatr; }
	virtual int getPrecedence() const		{ return operatr->getPrecedence(); }

	virtual bool buildString(StringBuffer &str, LocalVariables *locals = NULL) const;
	virtual bool buildAlgebraicString(StringBuffer &str, LocalVariables *locals = NULL) const;
	static bool canParseString(int initialChar, String::AbstractIterator &nextChars);
	static gc<const CObject *> parseString(String::AbstractIterator &stri);

	virtual int getObjectIdNumber() const;
	virtual void serializeData(IO::OutputStream *stream) const;
	static gc<const CObject *> deserializeData(IO::InputStream *stream);

	static gc<const CObject *> parseAlgebraic(String::AbstractIterator &stri, int minPrecedence);
};

};
