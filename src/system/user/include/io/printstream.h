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

#include <io/streams.h>
#include <util/string.h>
#include <util/formatter.h>

namespace IO
{

class PrintStream : public Object, public Formatter::Destination
{
	GC_INHERITS(Object);
	GC_FIELDS(out);

protected:
	OutputStream *out;

public:
	PrintStream(OutputStream *out)
	{
		this->out = out;
	}
	
	virtual void printChar(int chr)
	{
		// BUG: encoding is ignored
		char c = chr;
		out->write(&c, 1);
	}
	
	virtual void print(const char *s, size_t length)
	{
		// BUG: encoding is ignored
		out->writeFully(s, length);
	}
	
	void print(const String &s)
	{
		print(s.getChars(), s.getLength());
	}

	void println(const String &s)
	{
		print(s);
		print("\n");
	}

	template<typename... Args> void printf(const String &fmt, Args... args)
	{
		Formatter(fmt).print(this, args...);
	}
};

}
