
REGISTER_MENU(main, "MENU",
	MENU(math),
	MENU(prog)
)
REGISTER_MENU(math, "MATH",
	MENU(base)
)
REGISTER_MENU(base, "BASE",
	COMMAND("HEX"),
	COMMAND("DEC"),
	COMMAND("OCT"),
	COMMAND("BIN"),
	MENU(logic)
)
REGISTER_MENU(logic, "LOGIC",
	COMMAND("AND"),
	COMMAND("OR"),
	COMMAND("XOR"),
	COMMAND("NOT")
)
REGISTER_MENU(prog, "PROG",
	MENU(brch)
)
REGISTER_MENU(brch, "BRCH",
	COMMAND("IF"),
	COMMAND("CASE")
)
