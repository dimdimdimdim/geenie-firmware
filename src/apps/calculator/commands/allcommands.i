#ifdef NEEDS_HEADERS

#endif

// DO NOT CHANGE THE ORDER OF THE DECLARATIONS BELOW
// OTHERWISE, SERIALIZATION IDENTIFIERS WILL CHANGE AND
// PREVIOUS EEPROM/FLASH CONTENTS WILL BECOME UNREADABLE
// You can however add new commands safely, at the end

REGISTER_UNOP("NEG", cmdNeg, 22)
REGISTER_BINOP("*", cmdMul, 21)
REGISTER_BINOP("/", cmdDiv, 21)
REGISTER_BINOP("MOD", cmdMod, 21)
REGISTER_BINOP("+", cmdAdd, 20)
REGISTER_BINOP("-", cmdSub, 20)
REGISTER_BINOP("<", cmdIsLower, 15)
REGISTER_BINOP(">", cmdIsGreater, 15)
REGISTER_BINOP(STR_LESSOREQUAL, cmdIsLowerEqual, 15)
REGISTER_BINOP(STR_GREATEROREQUAL, cmdIsGreaterEqual, 15)
REGISTER_BINOP("==", cmdIsEqual, 14)
REGISTER_BINOP(STR_NOTEQUAL, cmdIsNotEqual, 14)
REGISTER_UNOP("NOT", cmdNot, 10)
REGISTER_BINOP("AND", cmdAnd, 9)
REGISTER_BINOP("XOR", cmdXor, 8)
REGISTER_BINOP("OR", cmdOr, 7)

REGISTER_CMD("SWAP", cmdSwap)
REGISTER_CMD("DROP", cmdDrop)
REGISTER_CMD("DROP2", cmdDrop2)
REGISTER_CMD("DROPN", cmdDropN)
REGISTER_CMD("DUP", cmdDup)
REGISTER_CMD("DUPDUP", cmdDupDup)
REGISTER_CMD("DUP2", cmdDup2)
REGISTER_CMD("DUPN", cmdDupN)
REGISTER_CMD("OVER", cmdOver)
REGISTER_CMD("PICK", cmdPick)
REGISTER_CMD("PICK3", cmdPick3)
REGISTER_CMD("ROLL", cmdRoll)
REGISTER_CMD("ROLLD", cmdRollD)
REGISTER_CMD("ROT", cmdRot)