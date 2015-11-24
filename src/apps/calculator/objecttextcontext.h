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

#include <graphics/text.h>
#include <graphics/font.h>

namespace Calc
{

class ObjectTextContext : public Graphics::TextContext
{
private:
	static const int WITHIN_STRING =		0x00000001;
	static const int WITHIN_ALGEBRAIC =		0x00000002;
	static const int WITHIN_VECTMATRIX =	0x00000004;
	static const int WITHIN_REAL =			0x00000010;
	static const int WITHIN_INTEGER =		0x00000020;
	static const int WITHIN_IDENTIFIER =	0x00000040;
	int flags;
	int countProgList, countDim;
	int itemStartPos;

public:
	ObjectTextContext() { reset(); }
	virtual void updateForward(int uchar, const String::AbstractIterator &it);
	virtual void updateBackward(int uchar, const String::AbstractIterator &it);
	virtual void reset() { flags = 0; countProgList = 0; countDim = 0; }

	bool isWithinAlgebraic()		{ return (flags & WITHIN_ALGEBRAIC) != 0; }
	bool isWithinProgList()			{ return countProgList > 0; }
	bool isWithinString()			{ return (flags & WITHIN_STRING) != 0; }
};

}
