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

#define STR_PI						"\xCF\x80"
#define STR_ANGLE					"\xE2\x88\xA0"
#define STR_SQRT					"\xE2\x88\x9A"
#define STR_MICRO					"\xC2\xB5"
#define STR_ARROWRIGHT				"\xE2\x86\x92"
#define STR_NOTEQUAL				"\xE2\x89\xA0"
#define STR_LESSOREQUAL				"\xE2\x89\xA4"
#define STR_GREATEROREQUAL			"\xE2\x89\xA5"
#define STR_OPENAQUOTE				"\xC2\xAB"
#define STR_CLOSEAQUOTE				"\xC2\xBB"
#define STR_ALPHA					"\xCE\xB1"
#define STR_SHIFT					"\xE2\x87\xA7"
#define STR_DEGREE					"\xC2\xB0"
#define STR_SUM						"\xE2\x88\x91"
#define STR_INTEGRAL				"\xE2\x88\xAB"
#define STR_DERIVATIVE				"\xE2\x88\x82"

#define CHAR_OPENAQUOTE				0xAB
#define CHAR_CLOSEAQUOTE			0xBB
#define CHAR_NOTEQUAL				0x2260
#define CHAR_LESSOREQUAL			0x2264
#define CHAR_GREATEROREQUAL			0x2265
#define CHAR_ELLIPSIS				0x2026

inline bool isWhiteSpace(int uchar)
{
	return uchar == ' ' || uchar == '\n' || uchar == '\r' || uchar == '\t';
}

inline bool isOperator(int uchar)
{
	return uchar == '+' || uchar == '-' || uchar == '*' || uchar == '/' || uchar == '^'
		|| uchar == '=' || uchar == '<' || uchar == '>'
		|| uchar == CHAR_LESSOREQUAL || uchar == CHAR_GREATEROREQUAL || uchar == CHAR_NOTEQUAL;
}

inline bool isSeparator(int uchar)
{
	return isWhiteSpace(uchar) || uchar == ',' || uchar == ':' || uchar == ';' || uchar == '_'
		|| uchar == '\'' || uchar == '"'
		|| uchar == '(' || uchar ==')' || uchar == '{' || uchar == '}' || uchar == '[' || uchar == ']'
		|| uchar == CHAR_OPENAQUOTE || uchar == CHAR_CLOSEAQUOTE;
}
