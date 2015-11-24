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
#include <sysutils.h>

#define CPU_MHZ 96

void itm_trace_enable()
{
	__REG(0xE000EDFC) |= 1 << 24;				// Debug Exception and Monitor Register
	__REG(0xE00400F0) = 2;					// TPIU Selected Pin Protocol Register
	__REG(0xE0040304) = 0x100;				// TPIU Formatter and Flush Control Register
	__REG(0xE0040010) = (CPU_MHZ / 6) - 1;	// TPIU Async Clock Prescaler Register
	__REG(0xE0000FB0) = 0xC5ACCE55;			// ITM Lock Access Register
	__REG(0xE0000E80) = 0x00010015;			// ITM Trace Control Register
	__REG(0xE0000E00) = 0x00000001;			// ITM Trace Enable Register
	__REG(0xE0000E40) = 0x00000001;			// ITM Trace Privilege Register
}

static void wait_cycles(int n)
{
	volatile int i;
	for (i = 0; i < n; i++) { }
}

void itm_trace(const char *str)
{
	while (*str) {
		wait_cycles(50);
		while (__REG(0xE0000000) == 0) { }
		__REG(0xE0000000) = *str;
		str++;
	}	
}
