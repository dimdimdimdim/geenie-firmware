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
#include <exception.h>
#include <util/stringbuffer.h>
#include <util/statichashtable.h>
#include <io/streams.h>
#include "../objects/command.h"
#include "../calculator.h"

#define NEEDS_HEADERS
#define REGISTER_CMD(s, x)
#define REGISTER_BINOP(s, x, p)
#define REGISTER_UNOP(s, x, p)
#include "allcommands.i"
#undef REGISTER_CMD
#undef REGISTER_BINOP
#undef REGISTER_UNOP
#undef NEEDS_HEADERS

namespace Calc
{

#define REGISTER_CMD(s, x) void x(Calculator *calculator);
#define REGISTER_BINOP(s, x, p) void x(Calculator *calculator);
#define REGISTER_UNOP(s, x, p) void x(Calculator *calculator);
#include "allcommands.i"
#undef REGISTER_CMD
#undef REGISTER_BINOP
#undef REGISTER_UNOP

enum CmdIdNumber
{
	CMDIDNUM_PRE = IDNUM_FIRST_COMMAND - 1,
#define REGISTER_CMD(s, x) CMDIDNUM_##x,
#define REGISTER_BINOP(s, x, p) CMDIDNUM_##x,
#define REGISTER_UNOP(s, x, p) CMDIDNUM_##x,
#include "allcommands.i"
#undef REGISTER_CMD
#undef REGISTER_BINOP
#undef REGISTER_UNOP
	CMDIDNUM_END
};

typedef void (* CommandFunc)(Calculator *calculator);

class StandardCommand : public Command
{
	GC_INHERITS(Command);
	GC_FIELDS(name);

private:
	String name;
	int id;
	CommandFunc func;

public:
	constexpr StandardCommand(const String &name, int id, CommandFunc func) :name(name), id(id), func(func) { }
	virtual gc<String> getName() const { return name; }
	virtual gc<String> toString(LocalVariables *locals = NULL) const { return name; }
	virtual void execute(Calculator *calculator, LocalVariables *locals) const { func(calculator); }

	virtual int getObjectIdNumber() const { return id; }
	virtual void serializeData(IO::OutputStream *stream) const { };
};

class BinaryOpCommand : public StandardCommand
{
	GC_INHERITS(Command);

private:
	int precedence;

public:
	constexpr BinaryOpCommand(const String &name, int id, CommandFunc func, int precedence) :StandardCommand(name, id, func), precedence(precedence) { }
	virtual int getPrecedence() const				{ return precedence; }
	virtual bool isBinaryOp() const					{ return true; }
	virtual int getAlgebraicArgumentsCount() const	{ return 2; }
};

class UnaryOpCommand : public StandardCommand
{
	GC_INHERITS(Command);

private:
	int precedence;

public:
	constexpr UnaryOpCommand(const String &name, int id, CommandFunc func, int precedence) :StandardCommand(name, id, func), precedence(precedence) { }
	virtual int getPrecedence() const				{ return precedence; }
	virtual bool isUnaryOp() const					{ return true; }
	virtual int getAlgebraicArgumentsCount() const	{ return 1; }
};

#define DECLARE_CMD_HASHENTRY_CLASS(s, x, ret) \
	struct CmdHashEntry_##x { \
		typedef gc<const Command *> ValueType; \
		static constexpr int keyHash = String(s).getHash(); \
		static bool checkKey(const String &key) { return key == s; } \
		static gc<const Command *> getValue() { return ret; } \
	};

#define REGISTER_CMD(s, x) \
	static const StandardCommand cmdObj_##x(s, CMDIDNUM_##x, x); \
	DECLARE_CMD_HASHENTRY_CLASS(s, x, &cmdObj_##x)
#define REGISTER_BINOP(s, x, p) \
	static const BinaryOpCommand cmdObj_##x(s, CMDIDNUM_##x, x, p); \
	DECLARE_CMD_HASHENTRY_CLASS(s, x, &cmdObj_##x)
#define REGISTER_UNOP(s, x, p) \
	static const UnaryOpCommand cmdObj_##x(s, CMDIDNUM_##x, x, p); \
	DECLARE_CMD_HASHENTRY_CLASS(s, x, &cmdObj_##x)
#include "allcommands.i"
#undef REGISTER_CMD
#undef REGISTER_BINOP
#undef REGISTER_UNOP

#define REGISTER_CMD(s, x)			REGISTER(x)
#define REGISTER_BINOP(s, x, p)		REGISTER(x)
#define REGISTER_UNOP(s, x, p)		REGISTER(x)

gc<const Command *> findCommand(int id)
{
	switch (id) {
#define REGISTER(x)					case CMDIDNUM_##x: return CmdHashEntry_##x::getValue();
#include "allcommands.i"
#undef REGISTER
	default:
		return NULL;
	}
}

struct DummyHashEntry {
	typedef gc<const Command *> ValueType;
	static constexpr int keyHash = -1;
	static constexpr bool checkKey(const String &key) { return false; }
	static gc<const Command *> getValue() { return NULL; }
};

typedef StaticHashTable<
#define REGISTER(x)					CmdHashEntry_##x,
#include "allcommands.i"
#undef REGISTER
	DummyHashEntry
> CommandTable;

gc<const Command *> findCommand(const String &name)
{
	return CommandTable::get(name);
}

}
