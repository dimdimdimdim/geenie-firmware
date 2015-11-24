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

#include "string.h"

class FormatSpecifier
{
public:
	static const int FLAG_APOST = 1;	//'
	static const int FLAG_MINUS = 2;	//-
	static const int FLAG_PLUS  = 4;	//+
	static const int FLAG_SPACE = 8;	//	
	static const int FLAG_ZERO = 16;	//0
	static const int FLAG_HASH = 32;	//#

private:
	int argIndex;
	int flags;
	int width;
	int precision;
	char type;
	
public:
	FormatSpecifier(int flags, int width, int precision, char type);
	FormatSpecifier(const class Formatter &formatter, size_t &fmtIndex, int &argsIndex);
	
	int getArgIndex()		{ return argIndex; }	
	bool hasFlag(int f)		{ return (flags & f) != 0; }
	int getWidth()			{ return width; }
	int getPrecision()		{ return precision; }
	char getType()			{ return type; }
};

class Formatter
{
public:

	class Argument;
	class Destination;

private:

	const String &fmtString;
	const Argument *firstArg, **lastArg;

	static void printfInt(unsigned long long n, bool negative, Destination *out, FormatSpecifier &spec);

public:
	
	class Destination
	{
	public:
		virtual void print(const char *s, size_t length) = 0;
		void print(const String &s) { print(s.getChars(), s.getLength()); }
	};

	class Argument
	{
	private:
		const Argument *next;
		union {
			struct {
				unsigned long long intValue;
				bool negative;
			};
			double doubleValue;
			const void *ptrValue;
			const String *stringRef;
		};
		enum {
			TYPE_INVALID,
			TYPE_INT,
			TYPE_DOUBLE,
			TYPE_POINTER,
			TYPE_POINTER_CHAR,
			TYPE_STRING
		} type;
		friend class Formatter;
	public:
		//template<class T> Argument(T a)		{ type = TYPE_INVALID; }
		Argument(signed char a)				{ type = TYPE_INT; intValue = a < 0 ? -a : a; negative = a < 0; }
		Argument(unsigned char a)			{ type = TYPE_INT; intValue = a; negative = false; }
		Argument(signed short a)			{ type = TYPE_INT; intValue = a < 0 ? -a : a; negative = a < 0; }
		Argument(unsigned short a)			{ type = TYPE_INT; intValue = a; negative = false; }
		Argument(signed int a)				{ type = TYPE_INT; intValue = a < 0 ? -a : a; negative = a < 0; }
		Argument(unsigned int a)			{ type = TYPE_INT; intValue = a; negative = false; }
		Argument(signed long a)				{ type = TYPE_INT; intValue = a < 0 ? -a : a; negative = a < 0; }
		Argument(unsigned long a)			{ type = TYPE_INT; intValue = a; negative = false; }
		Argument(signed long long a)		{ type = TYPE_INT; intValue = a < 0 ? -a : a; negative = a < 0; }
		Argument(unsigned long long a)		{ type = TYPE_INT; intValue = a; negative = false; }
		Argument(float a)					{ type = TYPE_DOUBLE; doubleValue = a; }
		Argument(double a)					{ type = TYPE_DOUBLE; doubleValue = a; }
		Argument(const String &a)			{ type = TYPE_STRING; stringRef = &a; }
		Argument(const void *a)				{ type = TYPE_POINTER; ptrValue = a; }
		Argument(const char *a)				{ type = TYPE_POINTER_CHAR; ptrValue = a; }

		virtual void print(Destination *out, FormatSpecifier &spec) const;
		virtual int asInt() const			{ return type == TYPE_INT ? (int)intValue : 0; }
	};

	Formatter(const String &fmtString) :fmtString(fmtString)	{ firstArg = NULL; lastArg = &firstArg; }

	const String &getFormatString() const	{ return fmtString; }
	const Argument *getArg(int n) const		{ const Argument *x = firstArg; while (x && n--) x = x->next; return x; }

	void addArg(const Argument &arg)
	{
		*lastArg = &arg;
		lastArg = (const Argument **)&arg.next;
	}

	void print(Destination *out) const;
	template<typename... Args> void print(Destination *out, const Argument &firstArg, Args... moreArgs)
	{
		addArg(firstArg);
		print(out, moreArgs...);
	}
};

