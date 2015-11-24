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
#include <util/string.h>

#define DEF_LANG_EN(x)
#define DEF_LANG_FR(x)

#define ___(x) DEF_##x
#define __(x) ___(x)
#define _(l, x) __(LANG_##l(x))
#define DEF_1(x) x

#undef Message
#define Message(name, strings)		const String MSG_##name(strings);
#include "messages.i"
