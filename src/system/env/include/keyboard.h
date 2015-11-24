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

#include <events.h>
#include <chars.h>

#define KBD_KEY_COUNT				52

class KeyAction
{
public:

	typedef enum {
		TYPE_VOID,
		TYPE_COMMAND,
		TYPE_MENU,
		TYPE_INPUT,
		TYPE_INPUT_MULTITAP,
		TYPE_INPUT_BRACES,
		TYPE_INPUT_CHR,
		TYPE_SPECIAL,
	} Type;

	typedef enum {
		ON,
		OFF,
		ABORT,
		SHIFT,
		ALPHA,
		UP,
		DOWN,
		LEFT,
		RIGHT,
		ENTER,
		BS,
		DEL,
		F1,
		F2,
		F3,
		F4,
		F5,
		F6,
		CHANGE_SIGN,
		ACTIVATE_ENV1,
		ACTIVATE_ENV2,
		ACTIVATE_ENV3,
		ACTIVATE_ENV4,
		ACTIVATE_ENV5,
		ACTIVATE_ENV6,
	} SpecialId;

private:
	Type type;
	union {
		String string;
		int chr;
		SpecialId id;
	};

	constexpr bool isStringType() const { return type == TYPE_COMMAND || type == TYPE_MENU || type == TYPE_INPUT || type == TYPE_INPUT_MULTITAP || type == TYPE_INPUT_BRACES; }

public:

	void gcMarkFields() const
	{
		if (isStringType())
			gcMarkObject(string);
	}

public:
	constexpr KeyAction() :type(TYPE_VOID), chr(0) { }
	constexpr KeyAction(Type type, const String &string) :type(type), string(string) { }
	constexpr KeyAction(Type type, int chr) : type(type), chr(chr) { }
	constexpr KeyAction(Type type, SpecialId id) : type(type), id(id) { }
	constexpr static KeyAction Command(const String &string) { return KeyAction(TYPE_COMMAND, string); }
	constexpr static KeyAction Menu(const String &string) { return KeyAction(TYPE_MENU, string); }
	constexpr static KeyAction Input(const String &string) { return KeyAction(TYPE_INPUT, string); }
	constexpr static KeyAction MultiTap(const String &string) { return KeyAction(TYPE_INPUT_MULTITAP, string); }
	constexpr static KeyAction Braces(const String &string) { return KeyAction(TYPE_INPUT_BRACES, string); }
	constexpr static KeyAction InputChar(int chr) { return KeyAction(TYPE_INPUT_CHR, chr); }
	constexpr static KeyAction Special(SpecialId id) { return KeyAction(TYPE_SPECIAL, id); }

	bool isVoid() const				{ return type == TYPE_VOID; }
	bool isCommand() const			{ return type == TYPE_COMMAND; }
	bool isMenu() const				{ return type == TYPE_MENU; }
	bool isInput() const			{ return type == TYPE_INPUT; }
	bool isMultiTap() const			{ return type == TYPE_INPUT_MULTITAP; }
	bool isBraces() const			{ return type == TYPE_INPUT_BRACES; }
	bool isInputChar() const		{ return type == TYPE_INPUT_CHR; }
	bool isSpecial() const			{ return type == TYPE_SPECIAL; }

	Type getType() const			{ return type; }
	int getChar() const				{ return chr; }
	int getId() const				{ return id; }
	const String &getString() const	{ return string; }
};

template<> class GCMarker<KeyAction>				{ public: static void mark(const KeyAction &x)			{ x.gcMarkFields(); } };
template<> class GCMarker<const KeyAction>			{ public: static void mark(const KeyAction &x)			{ x.gcMarkFields(); } };

namespace Kbd
{
	static const int SHIFT =		 1;
	static const int SHIFT_LOCK =	 2;
	static const int ALPHA =		 4;
	static const int ALPHA_LOCK =	 8;
	static const int HEX =			16;
	static const int META =			32;
	static const int FROM_MENU =	64;
}

const KeyAction *getKeyAction(int keyCode, int mode);

class KeyUpDownEvent : public Event
{
	GC_INHERITS(Object);
	GC_NO_FIELDS;
private:
	uint8_t keyCode;
public:
	KeyUpDownEvent(uint8_t keyCode) { this->keyCode = keyCode; }
	virtual void handle();
};

class KeyPressEvent : public Event
{
	GC_INHERITS(Object);
	GC_FIELDS(action);
private:
	const KeyAction *action;
	int mode;
public:
	KeyPressEvent(const KeyAction *action, int mode) { this->action = action; this->mode = mode; }
	virtual void handle();
};

class AuxKeyboardData
{
private:
	KeyAction::Type type;
	char string[20];
	int num;
	int mode;

public:
	AuxKeyboardData()					{ type = KeyAction::TYPE_VOID; }
	
	void setVoid()						{ type = KeyAction::TYPE_VOID; }
	bool isVoid()						{ return type == KeyAction::TYPE_VOID; }

	void setMode(int mode)
	{
		this->mode = mode;
	}

	void setSpecial(KeyAction::SpecialId id)
	{
		type = KeyAction::TYPE_SPECIAL;
		num = (int)id;
	}

	void setInputChar(int chr)
	{
		type = KeyAction::TYPE_INPUT_CHR;
		num = chr;
	}

	void setInput(const String &text)
	{
		type = KeyAction::TYPE_INPUT;
		size_t len = min(text.getLength(), sizeof(string));
		memcpy(string, text.getChars(), len);
		num = len;
	}

	void setCommand(const String &name)
	{
		type = KeyAction::TYPE_COMMAND;
		size_t len = min(name.getLength(), sizeof(string));
		memcpy(string, name.getChars(), len);
		num = len;
	}

	void setMenu(const String &name)
	{
		type = KeyAction::TYPE_MENU;
		size_t len = min(name.getLength(), sizeof(string));
		memcpy(string, name.getChars(), len);
		num = len;
	}

	int getMode() const
	{
		return mode;
	}

	gc<KeyAction *> getKeyAction() const
	{
		if (type == KeyAction::TYPE_COMMAND || type == KeyAction::TYPE_MENU || type == KeyAction::TYPE_INPUT || type == KeyAction::TYPE_INPUT_MULTITAP || type == KeyAction::TYPE_INPUT_BRACES)
			return gcnew(KeyAction(type, String::build(string, num > (int)sizeof(string) ? sizeof(string) : (size_t)num)));
		else if (type == KeyAction::TYPE_INPUT_CHR)
			return gcnew(KeyAction(type, (int)num));
		else if (type == KeyAction::TYPE_SPECIAL)
			return gcnew(KeyAction(type, (KeyAction::SpecialId)num));
		else
			return gcnew(KeyAction);
	}

};
