#ifdef NEEDS_HEADERS

#include "cinteger.h"
#include "creal.h"
#include "cstring.h"
#include "cprogram.h"
#include "calgebraic.h"

#endif

// DO NOT CHANGE THE ORDER OF THE DECLARATIONS BELOW
// OTHERWISE, SERIALIZATION IDENTIFIERS WILL CHANGE AND
// PREVIOUS EEPROM/FLASH CONTENTS WILL BECOME UNREADABLE
// Moreover, the return value of the TYPE command will also change
// You can however add new object types safely

REGISTER(CReal)			// id = 1
REGISTER(CInteger)		// id = 2
REGISTER(CString)		// ...
REGISTER(CProgram)
REGISTER(CAlgebraic)
