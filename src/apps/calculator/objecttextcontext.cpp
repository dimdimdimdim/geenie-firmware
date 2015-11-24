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
#include <chars.h>
#include <debugging.h>
#include "objecttextcontext.h"

namespace Calc
{

void ObjectTextContext::updateForward(int uchar, const String::AbstractIterator &it)
{
	bool checkNextChar = false;
	if (flags & WITHIN_STRING) {
		if (uchar == '"') {
			String::AbstractIterator &it2 = localCopy(it);
			if (it2.getBackward() != '\\') {
				flags &= ~WITHIN_STRING;
				LOG(APP, DEBUG, "Exit string\n");
			}
		}
	}
	else {
		if ((uchar == CHAR_OPENAQUOTE || uchar == '{') && (flags & (WITHIN_ALGEBRAIC | WITHIN_VECTMATRIX)) == 0) {
			flags &= ~(WITHIN_REAL | WITHIN_INTEGER | WITHIN_IDENTIFIER);
			checkNextChar = true;
			countProgList++;
			if (countProgList == 1)
				LOG(APP, DEBUG, "Enter list/prog\n");
		}
		else if ((uchar == CHAR_CLOSEAQUOTE || uchar == '}') && (flags & (WITHIN_ALGEBRAIC | WITHIN_VECTMATRIX)) == 0) {
			flags &= ~(WITHIN_REAL | WITHIN_INTEGER | WITHIN_IDENTIFIER);
			checkNextChar = true;
			countProgList--;
			if (countProgList == 0)
				LOG(APP, DEBUG, "Exit list/prog\n");
		}
		else if (uchar == '[') {
			flags &= ~(WITHIN_REAL | WITHIN_INTEGER | WITHIN_IDENTIFIER);
			checkNextChar = true;
			countDim++;
			if (countDim == 1) {
				flags |= WITHIN_VECTMATRIX;
				LOG(APP, DEBUG, "Enter vector/matrix\n");
			}
		}
		else if (uchar == ']') {
			flags &= ~(WITHIN_REAL | WITHIN_INTEGER | WITHIN_IDENTIFIER);
			checkNextChar = true;
			countDim--;
			if (countDim == 0) {
				flags &= ~WITHIN_VECTMATRIX;
				LOG(APP, DEBUG, "Exit vector/matrix\n");
			}
		}
		else if ((uchar == '\'') && (flags & WITHIN_VECTMATRIX) == 0) {
			flags &= ~(WITHIN_REAL | WITHIN_INTEGER | WITHIN_IDENTIFIER);
			checkNextChar = true;
			if (flags & WITHIN_ALGEBRAIC) {
				flags &= ~WITHIN_ALGEBRAIC;
				LOG(APP, DEBUG, "Exit algebraic\n");
			}
			else {
				flags |= WITHIN_ALGEBRAIC;
				LOG(APP, DEBUG, "Enter algebraic\n");
			}
		}
		else if (uchar == '"' && (flags & (WITHIN_ALGEBRAIC | WITHIN_VECTMATRIX)) == 0) {
			flags &= ~(WITHIN_REAL | WITHIN_INTEGER | WITHIN_IDENTIFIER);
			flags |= WITHIN_STRING;
			LOG(APP, DEBUG, "Enter string\n");
		}
		else if (isSeparator(uchar) || isOperator(uchar)) {
			flags &= ~(WITHIN_REAL | WITHIN_INTEGER | WITHIN_IDENTIFIER);
			checkNextChar = true;
			LOG(APP, DEBUG, "Exit real/int/id\n");
		}
		else if ((flags & (WITHIN_REAL | WITHIN_INTEGER | WITHIN_IDENTIFIER)) == 0) {
			if (uchar == '#') {
				flags |= WITHIN_INTEGER;
				itemStartPos = it.getPosition();
				LOG(APP, DEBUG, "Enter integer\n");
			}
			else if (uchar >= '0' && uchar <= '9') {
				flags |= WITHIN_REAL;
				itemStartPos = it.getPosition() - 1;
				LOG(APP, DEBUG, "Enter real\n");
			}
			else if (!isSeparator(uchar)) {
				flags |= WITHIN_IDENTIFIER;
				itemStartPos = it.getPosition() - 1;
				LOG(APP, DEBUG, "Enter identifier\n");
			}
		}
	}
	if (checkNextChar) {
		String::AbstractIterator &it2 = localCopy(it);
		int uc = it2.getForward();
		if (uc >= '0' && uc <= '9') {
			flags |= WITHIN_REAL;
			itemStartPos = it.getPosition();
			LOG(APP, DEBUG, "Enter real\n");
		}
		else if (uc != -1 && !isSeparator(uc) && !isOperator(uc) && uc != '#') {
			flags |= WITHIN_IDENTIFIER;
			itemStartPos = it.getPosition();
			LOG(APP, DEBUG, "Enter identifier\n");
		}
	}
}

void ObjectTextContext::updateBackward(int uchar, const String::AbstractIterator &it)
{
	bool checkPrevChar = false;
	if (flags & WITHIN_STRING) {
		if (uchar == '"') {
			String::AbstractIterator &it2 = localCopy(it);
			if (it2.getBackward() != '\\') {
				flags &= ~WITHIN_STRING;
				LOG(APP, DEBUG, "Exit string\n");
			}
		}
	}
	else {
		if ((uchar == CHAR_OPENAQUOTE || uchar == '{') && (flags & (WITHIN_ALGEBRAIC | WITHIN_VECTMATRIX)) == 0) {
			flags &= ~(WITHIN_REAL | WITHIN_INTEGER | WITHIN_IDENTIFIER);
			checkPrevChar = true;
			countProgList--;
			if (countProgList == 0)
				LOG(APP, DEBUG, "Exit list/prog\n");
		}
		else if ((uchar == CHAR_CLOSEAQUOTE || uchar == '}') && (flags & (WITHIN_ALGEBRAIC | WITHIN_VECTMATRIX)) == 0) {
			flags &= ~(WITHIN_REAL | WITHIN_INTEGER | WITHIN_IDENTIFIER);
			checkPrevChar = true;
			countProgList++;
			if (countProgList == 1)
				LOG(APP, DEBUG, "Enter list/prog\n");
		}
		else if (uchar == '[') {
			flags &= ~(WITHIN_REAL | WITHIN_INTEGER | WITHIN_IDENTIFIER);
			checkPrevChar = true;
			countDim--;
			if (countDim == 0) {
				flags &= ~WITHIN_VECTMATRIX;
				LOG(APP, DEBUG, "Exit vector/matrix\n");
			}
		}
		else if (uchar == ']') {
			flags &= ~(WITHIN_REAL | WITHIN_INTEGER | WITHIN_IDENTIFIER);
			checkPrevChar = true;
			countDim++;
			if (countDim == 1) {
				flags |= WITHIN_VECTMATRIX;
				LOG(APP, DEBUG, "Enter vector/matrix\n");
			}
		}
		else if ((uchar == '\'') && (flags & WITHIN_VECTMATRIX) == 0) {
			flags &= ~(WITHIN_REAL | WITHIN_INTEGER | WITHIN_IDENTIFIER);
			checkPrevChar = true;
			if (flags & WITHIN_ALGEBRAIC) {
				flags &= ~WITHIN_ALGEBRAIC;
				LOG(APP, DEBUG, "Exit algebraic\n");
			}
			else {
				flags |= WITHIN_ALGEBRAIC;
				LOG(APP, DEBUG, "Enter algebraic\n");
			}
		}
		else if (uchar == '"' && (flags & (WITHIN_ALGEBRAIC | WITHIN_VECTMATRIX)) == 0) {
			flags &= ~(WITHIN_REAL | WITHIN_INTEGER | WITHIN_IDENTIFIER);
			flags |= WITHIN_STRING;
			LOG(APP, DEBUG, "Enter string\n");
		}
		else if (uchar == '#' && (flags & WITHIN_INTEGER) != 0) {
			String::AbstractIterator &it2 = localCopy(it);
			int uc = it2.getBackward();
			if (uc == -1 || isSeparator(uc) || isOperator(uc)) {
				flags &= ~WITHIN_INTEGER;
				LOG(APP, DEBUG, "Exit integer\n");
			}
		}
		else if (isSeparator(uchar) || isOperator(uchar)) {
			flags &= ~(WITHIN_REAL | WITHIN_INTEGER | WITHIN_IDENTIFIER);
			checkPrevChar = true;
			LOG(APP, DEBUG, "Exit real/int/id\n");
		}
	}
	if (checkPrevChar) {
		String::AbstractIterator &it2 = localCopy(it);
		int uc = it2.getBackward();
		if (uc != -1 && !isSeparator(uc) && !isOperator(uc)) {
			while (uc != -1 && !isSeparator(uc) && !isOperator(uc))
				uc = it2.getBackward();
			if (uc != -1)
				uc = it2.getForward();
			uc = it2.getForward();
			if (uc == '#') {
				flags |= WITHIN_INTEGER;
				itemStartPos = it2.getPosition() + 1;
				LOG(APP, DEBUG, "Enter integer\n");
			}
			else if (uc >= '0' && uc <= '9') {
				flags |= WITHIN_REAL;
				itemStartPos = it2.getPosition();
				LOG(APP, DEBUG, "Enter real\n");
			}
		}
	}
}

}
