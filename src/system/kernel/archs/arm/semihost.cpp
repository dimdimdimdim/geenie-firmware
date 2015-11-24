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
//#include "semihost.h"

#ifdef __thumb2__
#define AngelSWI            "bkpt 0xAB"
#else
#ifdef __thumb__
#define AngelSWI            "swi 0xAB"
#else
#define AngelSWI            "swi 0x123456"
#endif
#endif


static inline int do_AngelSWI(int reason, void *arg)
{
	int value;
	asm volatile ("mov r0, %1; mov r1, %2; " AngelSWI "; mov %0, r0"
		: "=r" (value) /* Outputs */
		: "r" (reason), "r" (arg) /* Inputs */
		: "r0", "r1", "r2", "r3", "ip", "lr", "memory", "cc"
	);
	return value;
}

namespace Semihost
{

int sysOpen(const char *file, int mode)
{
	int args[3];
	args[0] = (int)file;
	args[1] = mode;
	args[2] = (int)strlen(file);
	return do_AngelSWI(0x01, args);
}

int sysClose(int handle)
{
	int args[1];
	args[0] = handle;
	return do_AngelSWI(0x02, args);
}

int sysWrite(int handle, const void *data, size_t length)
{
	int args[3];
	args[0] = handle;
	args[1] = (int)data;
	args[2] = (int)length;
	return do_AngelSWI(0x05, args);
}

void sysWrite0(const char *str)
{
	do_AngelSWI(0x04, (void *)str);
}

void reportException(int reason)
{
	do_AngelSWI(0x18, (void *)reason);
}

}
