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

namespace Kernel
{

struct MEMPOOL
{
	uintptr_t address;
	size_t length;
};

/*	Based on Two-Level Segregated fit allocator, adapted for garbage collection. See Masmano white papers for detailed info.
 *  Manages an unlimited number of pools (that don't need to be contiguous), but each pool must be not more than 128k bytes.
 */
class Allocator
{
protected:
	
	static const size_t MIN_ALLOC = 4;
	static const size_t MAX_ALLOC = 128 * 1024 - 1;
	
	static const uint32_t FLAG_USED =		0x10000000;
	static const uint32_t FLAG_GCMARK =		0x20000000;
	static const uint32_t FLAG_STATIC =		0x40000000;
	static const uint32_t FLAG_OBJECT =		0x80000000;
	static const uint32_t MASK_SIZE =		0x0001FFFC;
	static const uint32_t MASK_COPY_FLAGS = (FLAG_STATIC | FLAG_OBJECT);
	
	struct HEADER {
		uint32_t info;
		union {
			HEADER *nextFree;
			char data[1];
		};
	};
	static const size_t SIZEOF_HEADER_USED = 4;
	
	size_t getTLSFSlot(size_t size)
	{
		if (size < 32)
			return size >> 2;
		int i = 31 - __builtin_clz(size);
		return i * (1 << 3) + ((size >> (i - 3)) & ((1 << 3) - 1)) - 32;
	}
	
	static const int tableSize = 104; // getTLSFSlot(MAX_ALLOC) + 1;
	HEADER *table[tableSize];
	MEMPOOL *pools;
	size_t lastFreeMem;
	
	void *tryAlloc(size_t size, uint32_t flags);
	bool tryExpand(HEADER *block, HEADER *nextBlock, size_t size);
	void *alloc(size_t size, uint32_t flags);

	void releaseUnmarked();

public:
	
	Allocator(MEMPOOL *pools);
	
	static void doGC();

	void *malloc(size_t size);
	void *realloc(void *ptr, size_t size);
	void free(void *ptr);

	void *gcAlloc(size_t size);
	
	bool contains(const void *ptr)
	{
		for (int i = 0; pools[i].length != 0; i++)
			if ((uintptr_t)ptr >= pools[i].address && (uintptr_t)ptr < pools[i].address + pools[i].length)
				return true;
		return false;
	}

	static bool mark(const void *object)
	{
		HEADER *header = (HEADER *)((uintptr_t)object - SIZEOF_HEADER_USED);
		if (header->info & FLAG_GCMARK)
			return false;
		else {
			header->info |= FLAG_GCMARK;
			return true;
		}
	}

	static bool isMarked(void *object)
	{
		HEADER *header = (HEADER *)((uintptr_t)object - SIZEOF_HEADER_USED);
		return (header->info & FLAG_GCMARK) != 0;
	}

	static void setAsObject(void *object)
	{
		HEADER *header = (HEADER *)((uintptr_t)object - SIZEOF_HEADER_USED);
		header->info |= FLAG_OBJECT;
	}

	static bool isObject(void *object)
	{
		HEADER *header = (HEADER *)((uintptr_t)object - SIZEOF_HEADER_USED);
		return (header->info & FLAG_OBJECT) != 0;
	}

	size_t getFreeMem() { return lastFreeMem; }
};

extern Allocator ram;

inline bool isInRam(const void *ptr) { return ram.contains(ptr); }

}
