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

#include "util/string.h"
#include "util/stringbuffer.h"
#include "util/list.h"

class Calculator;

namespace Graphics
{
class Font;
class DisplayItem;
}

namespace IO
{
class InputStream;
class OutputStream;
}

namespace Calc
{

class LocalVariables;

class CObject : public Object
{
	GC_INHERITS(Object);
	GC_NO_FIELDS;

public:
	virtual gc<String> toString(LocalVariables *locals = NULL) const; // either toString or buildString must be overridden
	virtual bool buildString(StringBuffer &str, LocalVariables *locals = NULL) const; // returns false if string reached max StringBuffer length
	virtual bool buildAlgebraicString(StringBuffer &str, LocalVariables *locals = NULL) const { return buildString(str, locals); }
	virtual int getPrecedence() const		{ return 99; }

	virtual Graphics::DisplayItem *getDisplayItem(const Graphics::Font &font, LocalVariables *locals = NULL) const;

	virtual void enter(Calculator *calculator, LocalVariables *locals) const;
	virtual void execute(Calculator *calculator, LocalVariables *locals) const;

	virtual int getObjectIdNumber() const = 0;
	void serialize(IO::OutputStream *stream) const;
	virtual void serializeData(IO::OutputStream *stream) const = 0;
	static gc<const CObject *> deserialize(IO::InputStream *stream);

	static gc<List<const CObject *> *> parseSequence(String::AbstractIterator &stri, int endingChar = -1);
};

};