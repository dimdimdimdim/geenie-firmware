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

#include <util/list.h>
#include <exception.h>
#include <application.h>
#include <display.h>
#include <keyboard.h>
#include <graphics/font.h>
#include <graphics/textinput.h>

#include "objects/cobject.h"
#include "stacklayout.h"
#include "objecttextcontext.h"

#define checkType(x, t)		(x->isA<t>())

namespace Calc
{
	class Command;
}

class Calculator : public Application
{
	GC_INHERITS(Application);
	GC_FIELDS(stack, stackLayout, inputField, inputContext);

private:
	Stack<const Calc::CObject *> *stack;
	Calc::StackLayout *stackLayout;
	Graphics::TextInput *inputField;
	Calc::ObjectTextContext *inputContext;

public:
	Calculator() :Application("Calculator") { }

	virtual void onInit();
	virtual void onKeyPress(const KeyAction *action, int mode);
	
	bool enterInput();

	void checkStack(int nitems);
	const Calc::CObject *getX();
	const Calc::CObject *getY();
	const Calc::CObject *getZ();
	gc<const Calc::CObject *> pop();
	void push(const Calc::CObject *object);
	void swap();
	void drop();
	void drop2();
	void dropn();
	void dup();
	void dupdup();
	void dup2();
	void dupn();
	void over();
	void pick();
	void pick3();
	void roll();
	void rolld();
	void rot();
};

namespace Calc
{

gc<const Calc::Command *> parseCommand(int initialChar, String::AbstractIterator &nextChars, bool algebraicMode = false, int minPrecedence = 0);
gc<const Calc::CObject *> parseSingleObject(int initialChar, String::AbstractIterator &nextChars, bool algebraicMode = false, int minPrecedence = 0);
gc<const Calc::Command *> findCommand(const String &name);
gc<const Calc::Command *> findCommand(int id);
gc<const Menu *> findMenu(const String &name);

DECLARE_EXCEPTION_NOMSG(CalcException, Exception);
DECLARE_EXCEPTION(ECommandNotFound, CalcException, CommandNotFound);
DECLARE_EXCEPTION(EInvalidObjectFormat, CalcException, InvalidObjectFormat);
DECLARE_EXCEPTION(EIntegerTooBig, CalcException, IntegerTooBig);
DECLARE_EXCEPTION(ETooFewArguments, CalcException, TooFewArguments);
DECLARE_EXCEPTION(EBadArgumentType, CalcException, BadArgumentType);
DECLARE_EXCEPTION(ESyntaxError, CalcException, SyntaxError);
DECLARE_EXCEPTION(EInvalidObjectValue, ESyntaxError, InvalidObjectValue);
DECLARE_EXCEPTION(EMissingTerminatorCharacter, ESyntaxError, MissingTerminatorCharacter);
DECLARE_EXCEPTION(EMissingOperand, ESyntaxError, MissingOperand);
DECLARE_EXCEPTION(EAlgebraicNotSupported, ESyntaxError, AlgebraicNotSupported);
DECLARE_EXCEPTION(EWrongNumberOfAlgebraicOperands, ESyntaxError, WrongNumberOfAlgebraicOperands);

}
