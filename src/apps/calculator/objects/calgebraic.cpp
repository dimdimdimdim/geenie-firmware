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
#include <io/streams.h>
#include <util/stringbuffer.h>
#include <chars.h>
#include "../calculator.h"
#include "../../../math/math.h"
#include "calgebraic.h"

namespace Calc
{

CAlgebraic::CAlgebraic(const Command *operatr, const CObject * operand1)
:operands(1)
{
	this->operatr = operatr;
	operands[0] = operand1;
}

CAlgebraic::CAlgebraic(const Command *operatr, const CObject * operand1, const CObject *operand2)
:operands(2)
{
	this->operatr = operatr;
	operands[0] = operand1;
	operands[1] = operand2;
}

CAlgebraic::CAlgebraic(const Command *operatr, List<const CObject *> *operands)
:operands(operands->getLength())
{
	this->operatr = operatr;
	int n = 0;
	for (List<const CObject *>::Iterator i = operands->start(); i; i++, n++)
		this->operands[n] = *i;
}

CAlgebraic::CAlgebraic(const Command *operatr, Array<const CObject *> &operands)
:operands(operands)
{
	this->operatr = operatr;
}

bool CAlgebraic::buildString(StringBuffer &str, LocalVariables *locals) const
{
	if (!str.append("'"))
		return false;
	if (!buildAlgebraicString(str, locals))
		return false;
	if (!str.append("'"))
		return false;
	return true;
}

bool CAlgebraic::buildOperandString(StringBuffer &str, LocalVariables *locals, const CObject *operand) const
{
	if (operand->getPrecedence() < getPrecedence()) {
		if (!str.append("("))
			return false;
		if (!operand->buildAlgebraicString(str, locals))
			return false;
		if (!str.append(")"))
			return false;
	}
	else {
		if (!operand->buildAlgebraicString(str, locals))
			return false;
	}
	return true;
}

bool CAlgebraic::buildAlgebraicString(StringBuffer &str, LocalVariables *locals) const
{
	if (operatr->isUnaryOp()) {
		assert(operands.getLength() == 1);
		gc<String> name = operatr->getName();
		if (name == "NEG")
			name = "-";
		if (!str.append(name))
			return false;
		if (!isOperator(name[0])) {
			if (!str.append(" "))
				return false;
		}
		if (!buildOperandString(str, locals, operands[0]))
			return false;
	}
	else if (operatr->isBinaryOp()) {
		assert(operands.getLength() == 2);
		if (!buildOperandString(str, locals, operands[0]))
			return false;
		gc<String> name = operatr->getName();
		if (!isOperator(name[0])) {
			if (!str.append(" "))
				return false;
		}
		if (!str.append(name))
			return false;
		if (!isOperator(name[0])) {
			if (!str.append(" "))
				return false;
		}
		if (!buildOperandString(str, locals, operands[1]))
			return false;
	}
	else {
		if (!str.append(operatr->getName()))
			return false;
		if (operands.getLength() > 0) {
			if (!str.append("("))
				return false;
			if (!operands[0]->buildAlgebraicString(str, locals))
				return false;
			for (size_t i = 1; i < operands.getLength(); i++) {
				if (!str.append(","))
					return false;
				if (!operands[i]->buildAlgebraicString(str, locals))
					return false;
			}
			if (!str.append(")"))
				return false;
		}
	}
	return true;
}

bool CAlgebraic::canParseString(int initialChar, String::AbstractIterator &nextChars)
{
	return initialChar == '\'';
}

gc<const CObject *> CAlgebraic::parseAlgebraic(String::AbstractIterator &stri, int minPrecedence)
{
	int uchar = stri.getForward();
	while (isWhiteSpace(uchar))
		uchar = stri.getForward();
	if (uchar == -1 || uchar == ')' || uchar == '\'' || uchar == ',')
		gcthrownew(EMissingOperand);
	gc<const CObject *> obj;
	if (uchar == '(') {
		obj = parseAlgebraic(stri, 0);
		uchar = stri.getForward();
		// if (uchar == ',') complex number within algebraic
		if (uchar != ')')
			gcthrownew(EMissingTerminatorCharacter);
	}
	else {
		obj = parseSingleObject(uchar, stri, true);
		const Command *cmd = obj->dynCast<Command>();
		if (cmd) {
			if (cmd->getName() == "-") { // special case for unary minus
				cmd = findCommand("NEG"); // we dont put it in gc<> because we know it comes from flash
			}
			if (cmd->isUnaryOp()) {
				gc<const CObject *> operand = parseAlgebraic(stri, cmd->getPrecedence());
				obj = gcnew(CAlgebraic(cmd, operand));
			}
			else {
				int nargs = cmd->getAlgebraicArgumentsCount();
				if (nargs == -1)
					gcthrownew(EAlgebraicNotSupported);
				int uchar = stri.getForward();
				while (isWhiteSpace(uchar))
					uchar = stri.getForward();
				if (uchar == '(') {
					if (nargs == 0)
						gcthrownew(EWrongNumberOfAlgebraicOperands);
					while (isWhiteSpace(uchar))
						uchar = stri.getForward();
					gc<List<const CObject *> *> list = gcnew(List<const CObject *>);
					while (uchar != ')') {
						list->append(parseAlgebraic(stri, 0));
						uchar = stri.getForward();
						while (isWhiteSpace(uchar))
							uchar = stri.getForward();
						if (uchar == -1 || uchar == '\'')
							gcthrownew(EMissingTerminatorCharacter);
						else if (uchar != ')' && uchar != ',')
							gcthrownew(ESyntaxError);
					}
					if (nargs != -2 && nargs != list->getLength())
						gcthrownew(EWrongNumberOfAlgebraicOperands);
					obj = gcnew(CAlgebraic(cmd, list));
				}
				else {
					if (nargs != 0 && nargs != -2)
						gcthrownew(EWrongNumberOfAlgebraicOperands);
				}
			}
		}
	}
	uchar = stri.getForward();
	while (isWhiteSpace(uchar))
		uchar = stri.getForward();
	while (uchar != ')' && uchar != '\'' && uchar != ',') {
		if (uchar == -1)
			gcthrownew(EMissingTerminatorCharacter);
		gc<const Command *> cmd = parseCommand(uchar, stri, true, minPrecedence);
		if (cmd == NULL)
			return obj;
		gc<const CObject *> obj2 = parseAlgebraic(stri, cmd->getPrecedence() + 1);
		obj = gcnew(CAlgebraic(cmd, obj, obj2));
		uchar = stri.getForward();
		while (isWhiteSpace(uchar))
			uchar = stri.getForward();
	}
	stri.getBackward();
	return obj;
}

gc<const CObject *> CAlgebraic::parseString(String::AbstractIterator &stri)
{
	int uchar = stri.getForward();
	if (uchar != '\'')
		gcthrownew(EInvalidObjectValue);
	gc<const CObject *> obj = parseAlgebraic(stri, 0);
	uchar = stri.getForward();
	if (uchar != '\'')
		gcthrownew(EMissingTerminatorCharacter);
	return obj;
}

void CAlgebraic::serializeData(IO::OutputStream *stream) const
{
	operatr->serialize(stream);
	int length = operands.getLength();
	stream->write(&length, sizeof(length));
	for (size_t i = 0; i < (size_t)length; i++)
		operands[i]->serialize(stream);
}

gc<const CObject *> CAlgebraic::deserializeData(IO::InputStream *stream)
{
	gc<const CObject *> operatr = Command::deserialize(stream);
	if (!operatr->isA<Command>())
		gcthrownew(EInvalidObjectFormat);
	int length;
	stream->read(&length, sizeof(length));
	gc<Array<const CObject *>> operands;
	operands.alloc(length);
	for (size_t i = 0; i < (size_t)length; i++)
		operands[i] = CObject::deserialize(stream);
	return gcnew(CAlgebraic((Command *)(operatr.get()), operands));
}

}