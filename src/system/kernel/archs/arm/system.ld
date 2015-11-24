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
#include <util/macros.h>

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(__vectors_start)

/* Memory Spaces Definitions */
MEMORY
{
	#define BLOCK(origin, length)	ORIGIN = origin, LENGTH = length
	#define FLASH(block, ...)		flash (rx) : block
	#define RAM(block, ...)			ram (rwx) : block
	#include EXPAND_AND_STRINGIFY(MCU_MEMORY_MAP)
}

SECTIONS
{
    .fixed :
    {
		__vectors_start = .;
        KEEP(*(.vectors))
		__vectors_end = .;
		__system_code_start = .;
        *(.text*)
		__system_code_end = .;
        *(.rodata*)
        *(.data.rel.ro.*)
        *(.glue_7)
        *(.glue_7t)
        . = ALIGN(4);
	} >flash

	.fixed_cxx :
	{
		. = ALIGN(4);
		__system_preinit_array_start = .;
		KEEP (*(.preinit_array))
		__system_preinit_array_end = .;
		. = ALIGN(4);
		__system_init_array_start = .;
		KEEP (*(SORT(.init_array.*)))
		KEEP (*(.init_array))
		__system_init_array_end = .;
        . = ALIGN(4);
	} >flash

	.ARM.extab :
	{
		*(.ARM.extab*)
	} >flash
	
 	.ARM.exidx :
	{
		__exidx_start = .;
		*(.ARM.exidx* .gnu.linkonce.armexidx.*)
		__exidx_end = .;
	} >flash

    . = ALIGN(4);
	__system_relocate_source = .;

    .relocate :
    {
        . = ALIGN(4);
        __system_relocate_dest_start = .;
		__system_ramcode_start = .;
        *(.ramcode*);
		__system_ramcode_end = .;
        . = ALIGN(4);
        *(.data*);
        . = ALIGN(4);
        __system_relocate_dest_end = .;
    } >ram AT>flash

    .bss (NOLOAD) :
	{
        . = ALIGN(4);
        __system_bss_start = .;
        *(.bss*)
        . = ALIGN(4);
        __system_bss_end = .;
    } >ram

	. = ALIGN(8);
	
	. = . + 0x400;
	__system_stack_top = .;
	
	__heap_start = .;
}
