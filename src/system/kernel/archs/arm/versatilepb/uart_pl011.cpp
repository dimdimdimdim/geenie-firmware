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
#include <io/streams.h>
#include "../../peripherals/interrupts.h"

#define UART0_BASE_ADDR 0x101f1000
#define UART0_DR (*((volatile uint32_t *)(UART0_BASE_ADDR + 0x000)))
#define UART0_IMSC (*((volatile uint32_t *)(UART0_BASE_ADDR + 0x038)))

static void uart_handler()
{
	UART0_DR = UART0_DR;
}

EXTERN_C void uart_enable()
{
	configureInterrupt(12, 14, uart_handler);
	enableInterrupt(12);
	/* enable RXIM interrupt */
	UART0_IMSC = 1 << 4;
}

class PL011UART0OutputStream : public IO::OutputStream
{
public:
	virtual size_t write(const void *data, size_t len)
	{
		char *p = (char *)data;
		while (len--)
			UART0_DR = *p++;
		return p - (char *)data;
	}
};

static PL011UART0OutputStream uart0;
IO::OutputStream *debugRawOut = &uart0;
