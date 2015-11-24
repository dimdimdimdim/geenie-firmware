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
#include <application.h>
#include <keyboard.h>

struct KeyDef
{
	const KeyAction normalAction;
	const KeyAction shiftAction;
	const KeyAction alphaAction;
	const KeyAction alphaShiftAction;
	const KeyAction metaAction;
};

static const KeyDef keyDefs[KBD_KEY_COUNT] = {
	// first row below screen
	{
		KeyAction::Special(KeyAction::F1),
		KeyAction::Special(KeyAction::F1),
		KeyAction::Input("G"),
		KeyAction::Input("g"),
		KeyAction::Special(KeyAction::ACTIVATE_ENV1)
	},
	{
		KeyAction::Special(KeyAction::F2),
		KeyAction::Special(KeyAction::F2),
		KeyAction::Input("H"),
		KeyAction::Input("h"),
		KeyAction::Special(KeyAction::ACTIVATE_ENV2)
	},
	{
		KeyAction::Special(KeyAction::F3),
		KeyAction::Special(KeyAction::F3),
		KeyAction::Input("I"),
		KeyAction::Input("i"),
		KeyAction::Special(KeyAction::ACTIVATE_ENV3)
	},
	{
		KeyAction::Special(KeyAction::F4),
		KeyAction::Special(KeyAction::F4),
		KeyAction::Input("J"),
		KeyAction::Input("j"),
		KeyAction::Special(KeyAction::ACTIVATE_ENV4)
	},
	{
		KeyAction::Special(KeyAction::F5),
		KeyAction::Special(KeyAction::F5),
		KeyAction::Input("K"),
		KeyAction::Input("k"),
		KeyAction::Special(KeyAction::ACTIVATE_ENV5)
	},
	{
		KeyAction::Special(KeyAction::F6),
		KeyAction::Special(KeyAction::F6),
		KeyAction::Input("L"),
		KeyAction::Input("l"),
		KeyAction::Special(KeyAction::ACTIVATE_ENV6)
	},
	// second row below screen
	{
		KeyAction::Menu("MENU"),
		KeyAction::Menu("CHARS"),
		KeyAction::Input("M"),
		KeyAction::Input("m"),
		KeyAction()
	},
	{
		KeyAction::Menu("NEXT"),
		KeyAction::Menu("PREV"),
		KeyAction::Input("N"),
		KeyAction::Input("n"),
		KeyAction()
	},
	{
		KeyAction::Menu("LOGIC"),
		KeyAction::Menu("MODES"),
		KeyAction::Input("O"),
		KeyAction::Input("o"),
		KeyAction()
	},
	{
		KeyAction::Menu("VARS"),
		KeyAction::Menu("UNITS"),
		KeyAction::Input("P"),
		KeyAction::Input("p"),
		KeyAction()
	},
	{
		KeyAction::Command("SWAP"),
		KeyAction::Menu("STACK"),
		KeyAction::Input("Q"),
		KeyAction::Input("q"),
		KeyAction()
	},
	{
		KeyAction::Command("DROP"),
		KeyAction::Command("CLEAR"),
		KeyAction::Input("R"),
		KeyAction::Input("r"),
		KeyAction()
	},
	// third row below screen
	{
		KeyAction::Menu("MATH"),
		KeyAction::Menu("PLOT"),
		KeyAction::Input("S"),
		KeyAction::Input("s"),
		KeyAction()
	},
	{
		KeyAction::Menu("PROG"),
		KeyAction::Command("PURGE"),
		KeyAction::Input("T"),
		KeyAction::Input("t"),
		KeyAction()
	},
	{
		KeyAction::Command("STO"),
		KeyAction::Command("RCL"),
		KeyAction::Input("U"),
		KeyAction::Input("u"),
		KeyAction()
	},
	{
		KeyAction::Menu("HOME"),
		KeyAction::Menu("UP"),
		KeyAction::Input("V"),
		KeyAction::Input("v"),
		KeyAction()
	},
	{
		KeyAction::Command("EDIT"),
		KeyAction::Command("VIEW"),
		KeyAction::Input("W"),
		KeyAction::Input("w"),
		KeyAction()
	},
	{
		KeyAction::Command("EVAL"),
		KeyAction::Command("UNDO"),
		KeyAction::Input("X"),
		KeyAction::Input("x"),
		KeyAction()
	},
	// fourth row below screen
	{
		KeyAction::Special(KeyAction::ON),
		KeyAction::Special(KeyAction::OFF),
		KeyAction::Special(KeyAction::ON),
		KeyAction::Special(KeyAction::OFF),
		KeyAction()
	},
	{
		KeyAction::Special(KeyAction::SHIFT),
		KeyAction::Special(KeyAction::SHIFT),
		KeyAction::Special(KeyAction::SHIFT),
		KeyAction::Special(KeyAction::SHIFT),
		KeyAction()
	},
	{
		KeyAction::Special(KeyAction::ALPHA),
		KeyAction::Special(KeyAction::ALPHA),
		KeyAction::Special(KeyAction::ALPHA),
		KeyAction::Special(KeyAction::ALPHA),
		KeyAction()
	},
	{
		KeyAction::Braces("''"),
		KeyAction::MultiTap(STR_PI STR_SUM STR_INTEGRAL STR_DERIVATIVE),
		KeyAction::Input("Y"),
		KeyAction::Input("y"),
		KeyAction()
	},
	{
		KeyAction::Input("#"),
		KeyAction::Input("="),
		KeyAction::Input("Z"),
		KeyAction::Input("z"),
		KeyAction()
	},
	{
		KeyAction::Special(KeyAction::CHANGE_SIGN),
		KeyAction::MultiTap(STR_NOTEQUAL "<>" STR_LESSOREQUAL STR_GREATEROREQUAL),
		KeyAction::Input(STR_MICRO),
		KeyAction::Input(STR_MICRO),
		KeyAction()
	},
	// first row on the left of the screen
	{
		KeyAction::Command("SIN"),
		KeyAction::Command("ASIN"),
		KeyAction::Input("D"),
		KeyAction::Input("d"),
		KeyAction()
	},
	{
		KeyAction::Command("COS"),
		KeyAction::Command("ACOS"),
		KeyAction::Input("E"),
		KeyAction::Input("e"),
		KeyAction()
	},
	{
		KeyAction::Command("TAN"),
		KeyAction::Command("ATAN"),
		KeyAction::Input("F"),
		KeyAction::Input("f"),
		KeyAction()
	},
	{
		KeyAction::Special(KeyAction::BS),
		KeyAction::Special(KeyAction::DEL),
		KeyAction::Special(KeyAction::BS),
		KeyAction::Special(KeyAction::DEL),
		KeyAction()
	},
	// second row on the left of the screen
	{
		KeyAction::Command("SQRT"),
		KeyAction::Command("SQ"),
		KeyAction::Input("A"),
		KeyAction::Input("a"),
		KeyAction()
	},
	{
		KeyAction::Command("^"),
		KeyAction::Command("NROOT"),
		KeyAction::Input("B"),
		KeyAction::Input("b"),
		KeyAction()
	},
	{
		KeyAction::Command("INV"),
		KeyAction::Command("MOD"),
		KeyAction::Input("C"),
		KeyAction::Input("c"),
		KeyAction()
	},
	{
		KeyAction::Command("/"),
		KeyAction::Command("LOG"),
		KeyAction::Input("/"),
		KeyAction::Command("LOG"),
		KeyAction()
	},
	// third row on the left of the screen
	{
		KeyAction::Input("7"),
		KeyAction::Braces("(  )"),
		KeyAction::Input("7"),
		KeyAction::Braces("(  )"),
		KeyAction()
	},
	{
		KeyAction::Input("8"),
		KeyAction::Braces("[  ]"),
		KeyAction::Input("8"),
		KeyAction::Braces("[  ]"),
		KeyAction()
	},
	{
		KeyAction::Input("9"),
		KeyAction::Braces("{  }"),
		KeyAction::Input("9"),
		KeyAction::Braces("{  }"),
		KeyAction()
	},
	{
		KeyAction::Command("*"),
		KeyAction::Command("ALOG"),
		KeyAction::Input("*"),
		KeyAction::Command("ALOG"),
		KeyAction()
	},
	// fourth row on the left of the screen
	{
		KeyAction::Input("4"),
		KeyAction::Braces(STR_OPENAQUOTE "  " STR_CLOSEAQUOTE),
		KeyAction::Input("4"),
		KeyAction::Braces(STR_OPENAQUOTE "  " STR_CLOSEAQUOTE),
		KeyAction()
	},
	{
		KeyAction::Input("5"),
		KeyAction::Braces("\"\""),
		KeyAction::Input("5"),
		KeyAction::Braces("\"\""),
		KeyAction()
	},
	{
		KeyAction::Input("6"),
		KeyAction::MultiTap(":;!?"),
		KeyAction::Input("6"),
		KeyAction::MultiTap(":;!?"),
		KeyAction()
	},
	{
		KeyAction::Command("-"),
		KeyAction::Command("LN"),
		KeyAction::Input("-"),
		KeyAction::Command("LN"),
		KeyAction()
	},
	// fifth row on the left of the screen
	{
		KeyAction::Input("1"),
		KeyAction::MultiTap("%$"),
		KeyAction::Input("1"),
		KeyAction::MultiTap("%$"),
		KeyAction()
	},
	{
		KeyAction::Input("2"),
		KeyAction::MultiTap(STR_DEGREE "~"),
		KeyAction::Input("2"),
		KeyAction::MultiTap(STR_DEGREE "~"),
		KeyAction()
	},
	{
		KeyAction::Input("3"),
		KeyAction::MultiTap(STR_ARROWRIGHT "\\"),
		KeyAction::Input("3"),
		KeyAction::MultiTap(STR_ARROWRIGHT "\\"),
		KeyAction()
	},
	{
		KeyAction::Command("+"),
		KeyAction::Command("EXP"),
		KeyAction::Input("+"),
		KeyAction::Command("EXP"),
		KeyAction()
	},
	// sixth row on the left of the screen
	{
		KeyAction::Input("0"),
		KeyAction::MultiTap("_&@"),
		KeyAction::Input("0"),
		KeyAction::MultiTap("_&@"),
		KeyAction()
	},
	{
		KeyAction::Input("."),
		KeyAction::MultiTap("," STR_ANGLE),
		KeyAction::Input("."),
		KeyAction::MultiTap("," STR_ANGLE),
		KeyAction()
	},
	{
		KeyAction::Input(" "),
		KeyAction::Input(" "),
		KeyAction::Input(" "),
		KeyAction::Input(" "),
		KeyAction()
	},
	{
		KeyAction::Special(KeyAction::ENTER),
		KeyAction::Command("DUP"),
		KeyAction::Special(KeyAction::ENTER),
		KeyAction::Command("DUP"),
		KeyAction::Input("\n")
	},
	// arrows
	{
		KeyAction::Special(KeyAction::UP),
		KeyAction::Special(KeyAction::UP),
		KeyAction::Special(KeyAction::UP),
		KeyAction::Special(KeyAction::UP),
		KeyAction()
	},
	{
		KeyAction::Special(KeyAction::DOWN),
		KeyAction::Special(KeyAction::DOWN),
		KeyAction::Special(KeyAction::DOWN),
		KeyAction::Special(KeyAction::DOWN),
		KeyAction()
	},
	{
		KeyAction::Special(KeyAction::LEFT),
		KeyAction::Special(KeyAction::LEFT),
		KeyAction::Special(KeyAction::LEFT),
		KeyAction::Special(KeyAction::LEFT),
		KeyAction()
	},
	{
		KeyAction::Special(KeyAction::RIGHT),
		KeyAction::Special(KeyAction::RIGHT),
		KeyAction::Special(KeyAction::RIGHT),
		KeyAction::Special(KeyAction::RIGHT),
		KeyAction()
	},
};

const KeyAction *getKeyAction(int keyCode, int mode)
{
	assert(keyCode < KBD_KEY_COUNT);
	const KeyDef &def = keyDefs[keyCode];
	if ((mode & Kbd::META) != 0)
		return &def.metaAction;
	else {
		if ((mode & Kbd::HEX) != 0 && (mode & Kbd::ALPHA_LOCK) == 0) {
			const KeyAction &alpha = def.alphaAction;
			if (alpha.isInput() && alpha.getString()[0] >= 'A' && alpha.getString()[0] <= 'F')
				mode ^= Kbd::ALPHA;
		}
		if (((mode & Kbd::ALPHA) ^ (mode & Kbd::ALPHA_LOCK)) == 0) {
			if (((mode & Kbd::SHIFT) ^ (mode & Kbd::SHIFT_LOCK))== 0)
				return &def.normalAction;
			else
				return &def.shiftAction;
		}
		else {
			if (((mode & Kbd::SHIFT) ^ (mode & Kbd::SHIFT_LOCK))== 0)
				return &def.alphaAction;
			else
				return &def.alphaShiftAction;
		}
	}
}

void KeyUpDownEvent::handle()
{
	if ((keyCode & 0x80) == 0)
		getCurrentApplication()->onKeyDown(keyCode);
	else
		getCurrentApplication()->onKeyUp(keyCode & 0x7F);
}

void KeyPressEvent::handle()
{
	getCurrentApplication()->onKeyPress(action, mode);
}
