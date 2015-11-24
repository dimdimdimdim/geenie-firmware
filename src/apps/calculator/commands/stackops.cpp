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
#include "../calculator.h"
#include "../objects/cobject.h"
#include "../objects/creal.h"
#include "../objects/cinteger.h"
#include "../objects/cstring.h"

namespace Calc
{

void cmdSwap(Calculator *calculator)	{ calculator->swap(); }
void cmdDrop(Calculator *calculator)	{ calculator->drop(); }
void cmdDrop2(Calculator *calculator)	{ calculator->drop2(); }
void cmdDropN(Calculator *calculator)	{ calculator->dropn(); }
void cmdDup(Calculator *calculator)		{ calculator->dup(); }
void cmdDupDup(Calculator *calculator)	{ calculator->dupdup(); }
void cmdDup2(Calculator *calculator)	{ calculator->dup2(); }
void cmdDupN(Calculator *calculator)	{ calculator->dupn(); }
void cmdOver(Calculator *calculator)	{ calculator->over(); }
void cmdPick(Calculator *calculator)	{ calculator->pick(); }
void cmdPick3(Calculator *calculator)	{ calculator->pick3(); }
void cmdRoll(Calculator *calculator)	{ calculator->roll(); }
void cmdRollD(Calculator *calculator)	{ calculator->rolld(); }
void cmdRot(Calculator *calculator)		{ calculator->rot(); }

}
