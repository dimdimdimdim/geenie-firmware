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

#define __REG(x)		(*((volatile uint32_t *)(x)))
#define __REG8(x)		(*((volatile uint8_t *)(x)))
#define __REG16(x)		(*((volatile uint16_t *)(x)))

#ifdef MCU_CMSIS_HEADER

#include EXPAND_AND_STRINGIFY(../../../../../../MCU_CMSIS_HEADER)

__attribute__((always_inline)) inline void __switch_to_USR()
{
	__set_CONTROL(1);
	__ISB();
}

#else

__attribute__(( always_inline )) inline void __enable_irq(void)
{
  __ASM volatile ("cpsie i" : : : "memory");
}

__attribute__(( always_inline )) inline void __disable_irq(void)
{
  __ASM volatile ("cpsid i" : : : "memory");
}

__attribute__((always_inline)) inline void __switch_to_USR()
{
	asm volatile("cps #0x10");
}

#endif
