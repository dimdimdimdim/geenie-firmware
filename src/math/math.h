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

#include <util/string.h>
#include <util/stringbuffer.h>
#include <exception.h>

#define REAL_MODE_STD	0x00000000
#define REAL_MODE_FIX	0x10000000
#define REAL_MODE_SCI	0x20000000
#define REAL_MODE_ENG	0x30000000
#define REAL_MODE_MASK	0x30000000
#define REAL_TEXTBOOK	0x40000000

gc<String> doubleToString(double d, int mode = 0);
bool doubleToString(StringBuffer &str, double d, int mode = 0); // returns false if string reached max StringBuffer length
double stringToDouble(String::AbstractIterator &stri); // returns NAN if parsing failed
