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
#include <debugging.h>
#include <graphics/textinput.h>
#include "calculator.h"
#include "objects/command.h"
#include "objects/creal.h"

using namespace Calc;

void Calculator::onInit()
{
	stack = gcnew(Stack<const CObject *>);
	inputContext = gcnew(ObjectTextContext);
	inputField = gcnew(Graphics::TextInput(getEnv()->getDisplayWidth(), getEnv()->getDisplayHeight(), false, Graphics::Font::BIG, 0, "", inputContext)); 
	stackLayout = gcnew(StackLayout(stack, inputField));
	setDisplayContents(stackLayout);
}

bool Calculator::enterInput()
{
	if (inputField->isVisible()) {
		try {
			GapStringBuffer::Iterator strit = inputField->getTextIterator();
			List<const CObject *> *objects = CObject::parseSequence(strit);
			inputField->clearText();
			if (objects != NULL) {
				for (List<const CObject *>::Iterator i = objects->start(); i; i++)
					(*i)->enter(this, NULL);
			}
		}
		catch (Exception *ex) {
			LOG(APP, INFO, "Exception: %s\n", ex->getMessage());
			inputField->setVisible(false);
			return false;
		}
		inputField->setVisible(false);
	}
	return true;
}

void Calculator::onKeyPress(const KeyAction *action, int mode)
{
	if (action->isVoid())
		return;
	else if (action->isCommand()) {
		if (inputField->isVisible() && inputContext->isWithinAlgebraic()) {
			inputField->insertText(action->getString());
			inputField->update();
		}
		else if (inputField->isVisible() && inputContext->isWithinProgList()) {
			GapStringBuffer::Iterator it = inputField->getCursorIterator();
			int uc = it.getBackward();
			if (uc != -1 && !isWhiteSpace(uc))
				inputField->insertText(" ");
			inputField->insertText(action->getString());
			it = inputField->getCursorIterator();
			uc = it.getForward();
			if (uc != -1 && !isWhiteSpace(uc))
				inputField->insertText(" ");
			else if (uc != -1)
				inputField->moveForward();
			inputField->update();
		}
		else {
			stackLayout->clearCachedItems();
			if (enterInput()) {
				try {
					gc<const Calc::Command *> cmd = Calc::findCommand(action->getString());
					if (cmd == NULL)
						gcthrownew(ECommandNotFound);
					stackLayout->clearCachedItems();
					cmd->enter(this, NULL);
				}
				catch (Exception *ex) {
					LOG(APP, INFO, "Exception: %s\n", ex->getMessage());
				}
				stackLayout->relayout();
				stackLayout->redraw();
			}
		}
	}
	else if (action->isMenu()) {
	}
	else if (action->isSpecial()) {
		if (action->getId() == KeyAction::ABORT) {
			if (inputField->isVisible()) {
				inputField->setVisible(false);
				inputField->clearText();
			}
		}
		else if (action->getId() == KeyAction::ENTER) {
			stackLayout->clearCachedItems();
			if (inputField->isVisible())
				enterInput();
			else {
				if (stack->getLength() > 0) {
					dup();
					stackLayout->relayout();
					stackLayout->redraw();
				}
			}
		}
		else if (action->getId() == KeyAction::BS && !inputField->isVisible()) {
			if (stack->getLength() > 0) {
				stackLayout->clearCachedItems();
				drop();
				stackLayout->relayout();
				stackLayout->redraw();
			}
		}
		else {
			if (inputField->isVisible())
				inputField->onKeyPress(action, mode);
		}
	}
	else {
		if (!inputField->isVisible())
			inputField->setVisible(true);
		inputField->onKeyPress(action, mode);
	}
}

void Calculator::checkStack(int nitems)
{
	if (stack->getLength() < nitems)
		gcthrownew(ETooFewArguments);
}

const Calc::CObject *Calculator::getX()
{
	if (stack->getLength() < 1)
		gcthrownew(ETooFewArguments);
	return stack->getAt(0);
}

const Calc::CObject *Calculator::getY()
{
	if (stack->getLength() < 2)
		gcthrownew(ETooFewArguments);
	return stack->getAt(1);
}

const Calc::CObject *Calculator::getZ()
{
	if (stack->getLength() < 3)
		gcthrownew(ETooFewArguments);
	return stack->getAt(2);
}

gc<const CObject *> Calculator::pop()
{
	if (stack->getLength() == 0)
		gcthrownew(ETooFewArguments);
	return stack->pop();
}

void Calculator::push(const CObject *object)
{
	stack->push(object);
}

void Calculator::swap()
{
	checkStack(2);
	gc<const CObject *> x = stack->getAt(0);
	stack->setAt(0, stack->getAt(1));
	stack->setAt(1, x);
}

void Calculator::drop()
{
	checkStack(1);
	stack->removeAt(0);
}

void Calculator::drop2()
{
	checkStack(2);
	stack->removeAt(0);
	stack->removeAt(0);
}

void Calculator::dropn()
{
	const CObject *x = getX();
	const CReal *rx = x->dynCast<CReal>();
	if (!rx)
		gcthrownew(EBadArgumentType);
	int n = (int)(rx->getValue());
	if (n <= 0) {
		stack->removeAt(0);
		return;
	}
	checkStack(n + 1);
	for (int i = 0; i < n + 1; i++)
		stack->removeAt(0);
}

void Calculator::dup()
{
	const CObject *x = getX();
	stack->push(x);
}

void Calculator::dupdup()
{
	const CObject *x = getX();
	stack->push(x);
	stack->push(x);
}

void Calculator::dup2()
{
	const CObject *x = getX();
	const CObject *y = getY();
	stack->push(y);
	stack->push(x);
}

void Calculator::dupn()
{
	const CObject *x = getX();
	const CReal *rx = x->dynCast<CReal>();
	if (!rx)
		gcthrownew(EBadArgumentType);
	int n = (int)(rx->getValue());
	if (n <= 0) {
		stack->removeAt(0);
		return;
	}
	checkStack(n + 1);
	stack->removeAt(0);
	for (int i = 0; i < n; i++)
		stack->push(stack->getAt(n));
}

void Calculator::over()
{
	const CObject *y = getY();
	stack->push(y);
}

void Calculator::pick()
{
	const CObject *x = getX();
	const CReal *rx = x->dynCast<CReal>();
	if (!rx)
		gcthrownew(EBadArgumentType);
	int n = (int)(rx->getValue());
	if (n <= 0) {
		stack->removeAt(0);
		return;
	}
	checkStack(n + 1);
	stack->removeAt(0);
	const CObject *o = stack->getAt(n);
	stack->push(o);
}

void Calculator::pick3()
{
	const CObject *z = getZ();
	stack->push(z);
}

void Calculator::roll()
{
	const CObject *x = getX();
	const CReal *rx = x->dynCast<CReal>();
	if (!rx)
		gcthrownew(EBadArgumentType);
	int n = (int)(rx->getValue());
	if (n <= 0) {
		stack->removeAt(0);
		return;
	}
	checkStack(n + 1);
	stack->removeAt(0);
	gc<const CObject *> o = stack->getAt(n);
	stack->removeAt(n);
	stack->push(o);
}

void Calculator::rolld()
{
	const CObject *x = getX();
	const CReal *rx = x->dynCast<CReal>();
	if (!rx)
		gcthrownew(EBadArgumentType);
	int n = (int)(rx->getValue());
	if (n <= 0) {
		stack->removeAt(0);
		return;
	}
	checkStack(n + 1);
	stack->removeAt(0);
	gc<const CObject *> o = stack->pop();
	stack->insertAt(n - 1, o);
}

void Calculator::rot()
{
	checkStack(3);
	gc<const CObject *> o = stack->getAt(2);
	stack->setAt(2, stack->getAt(1));
	stack->setAt(1, stack->getAt(0));
	stack->setAt(0, o);
}

