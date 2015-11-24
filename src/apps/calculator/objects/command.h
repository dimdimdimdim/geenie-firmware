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

#define IDNUM_FIRST_COMMAND		0x1000

namespace Calc
{

class Command : public CObject
{
	GC_INHERITS(CObject);

public:
	virtual gc<String> getName() const = 0;
	virtual int getAlgebraicArgumentsCount() const	{ return -1; }
	virtual bool isBinaryOp() const					{ return false; }
	virtual bool isUnaryOp() const					{ return false; }
	virtual void enter(Calculator *calculator, LocalVariables *locals) const { return execute(calculator, locals); }
};

}