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
#include <debugging.h>
#include <util/list.h>
#include "../threading/scheduler.h"

extern bool debugOutAvailable;

namespace Kernel
{

GCGlobalItem *globalRefs = NULL;

void Allocator::doGC()
{
	GCRootItem *r = globalRefs;
	while (r) {
		r->markObject();
		r = r->getNext();
	}
	int threadCount = Scheduler::getThreadCount();
	SysThread **threads = (SysThread **)alloca(sizeof(SysThread *) * threadCount);
	int n = Scheduler::getThreadList(threads, threadCount);
	assert(n == threadCount);
	(void)n;
	for (int i = 0; i < threadCount; i++) {
		gcMarkObject(threads[i]);
		r = threads[i]->lastGCStackItem;
		while (r) {
			r->markObject();
			r = r->getNext();
		}
		r = threads[i]->lastGCConstrItem;
		while (r) {
			r->markObject();
			r = r->getNext();
		}
		gcMarkObject(threads[i]->lastThrownException);
		if (threads[i]->tmpStackObjRef) {
			void *object = *threads[i]->tmpStackObjRef;
			if (object && mark(object) && isObject(object))
				((Object *)object)->gcMarkFields();
		}
	}
	ram.releaseUnmarked();
}

void *Allocator::tryAlloc(size_t size, uint32_t flags)
{
	int i = getTLSFSlot(size);
	assert(i < tableSize);
	while (table[i] == NULL && i < tableSize)
		i++;
	if (i == tableSize)
		return NULL; // no more space available for this block size
	HEADER *block = table[i];
	assert((block->info & MASK_SIZE) >= size);
	
	//if (debugOutAvailable)
	//	LOG(KERNEL, INFO, "Allocating %d bytes\n", size);
	// check block size
	size_t blockSize = block->info & MASK_SIZE;
	if (blockSize < size + SIZEOF_HEADER_USED + MIN_ALLOC) { // we can't split it
		lastFreeMem -= size + SIZEOF_HEADER_USED;
		// remove block from heap
		table[i] = block->nextFree;
		// adjust returned block properties
		block->info = blockSize | FLAG_USED | flags;
		return block->data;
	}
	else { // we must split it
		lastFreeMem -= size + SIZEOF_HEADER_USED * 2;
		// remove block from heap
		table[i] = block->nextFree;
		// new block from remaining space
		HEADER *newBlock = (HEADER *)((size_t)block + SIZEOF_HEADER_USED + size);
		newBlock->info = blockSize - size - SIZEOF_HEADER_USED;
		int j = getTLSFSlot(newBlock->info);
		assert(j < tableSize);
		newBlock->nextFree = table[j];
		table[j] = newBlock;
		// adjust returned block
		block->info = size | FLAG_USED | flags;
		return block->data;
	}
}

bool Allocator::tryExpand(HEADER *block, HEADER *nextBlock, size_t size)
{
	size_t blockSize = block->info & MASK_SIZE;
	uint32_t origFlags = block->info & MASK_COPY_FLAGS;
	size_t nextBlockSize = nextBlock->info & MASK_SIZE;

	if ((nextBlock->info & FLAG_USED) == 0 && size <= blockSize + nextBlockSize + SIZEOF_HEADER_USED) {
		// remove next block from table
		int i = getTLSFSlot(nextBlockSize);
		assert(i < tableSize);
		HEADER **header = &table[i];
		while (*header != nextBlock) {
			assert(*header != NULL);
			header = &(*header)->nextFree;
		}
		*header = (*header)->nextFree;
		// rearrange blocks
		size_t nextBlockNewSize = blockSize + nextBlockSize - size;
		if (nextBlockNewSize < MIN_ALLOC) {
			// take over whole next block
			lastFreeMem -= nextBlockSize + SIZEOF_HEADER_USED;
			block->info = (blockSize + nextBlockSize + SIZEOF_HEADER_USED) | FLAG_USED | origFlags;
		}
		else {
			// split next block
			lastFreeMem -= nextBlockSize - nextBlockNewSize;
			HEADER *newBlock = (HEADER *)((uintptr_t)block + SIZEOF_HEADER_USED + size);
			i = getTLSFSlot(nextBlockNewSize);
			assert(i < tableSize);
			newBlock->info = nextBlockNewSize;
			newBlock->nextFree = table[i];
			table[i] = newBlock;
			block->info = size | FLAG_USED | origFlags;
		}
		return true;
	}
	return false;
}

Allocator::Allocator(MEMPOOL *pools)
{
	lastFreeMem = 0;
	for (int i = 0; i < tableSize; i++) {
		table[i] = NULL;
	}
	this->pools = pools;
	while (pools->length != 0) {
		assert((pools->address & 3) == 0);
		assert((pools->length & 3) == 0);
		lastFreeMem += pools->length - SIZEOF_HEADER_USED;
		// write the initial header and add pool to the table
		int i = getTLSFSlot(pools->length - SIZEOF_HEADER_USED);
		assert(i < tableSize);
		HEADER *initial = (HEADER *)pools->address;
		initial->info = pools->length - SIZEOF_HEADER_USED;
		initial->nextFree = table[i];
		table[i] = initial;
		pools++;
	}
}

void *Allocator::alloc(size_t size, uint32_t flags)
{
	if (size == 0)
		return NULL;
	size = (size + 3) & ~3; // blocks must be 32-bit aligned 
	if (size < MIN_ALLOC)
		size = MIN_ALLOC;
	if (size > MAX_ALLOC)
		return NULL; // too big
	
	void *newAlloc = tryAlloc(size, flags);
	if (newAlloc != NULL)
		return newAlloc;
	doGC();
	return tryAlloc(size,  flags);
}

void *Allocator::malloc(size_t size)
{
	return alloc(size, FLAG_STATIC);
}

void Allocator::free(void *ptr)
{
	// We don't coerce with next block, that will be done later by releaseUnmarked.
	HEADER *block = (HEADER *)((uintptr_t)ptr - SIZEOF_HEADER_USED);
	block->info = block->info & MASK_SIZE;
	lastFreeMem += block->info;
	int i = getTLSFSlot(block->info);
	assert(i < tableSize);
	block->nextFree = table[i];
	table[i] = block;
}

void *Allocator::realloc(void *ptr, size_t size)
{
	if (ptr == NULL)
		return alloc(size, FLAG_STATIC);
	if (size == 0) {
		free(ptr);
		return NULL;
	}
	size = (size + 3) & ~3; // blocks must be 32-bit aligned 
	if (size < MIN_ALLOC)
		size = MIN_ALLOC;
	if (size > MAX_ALLOC)
		return NULL; // too big
	
	HEADER *block = (HEADER *)((uintptr_t)ptr - SIZEOF_HEADER_USED);
	size_t blockSize = block->info & MASK_SIZE;

	if (size > blockSize) {
		// check if next block is free and large enough
		HEADER *nextBlock = (HEADER *)((uintptr_t)ptr + blockSize);
		bool isLastBlock;
		bool gcDone = false;
		for (MEMPOOL *pool = pools; pool->length != 0; pool++)
			if ((isLastBlock = ((uintptr_t)nextBlock == pool->address + pool->length))) 
				break;
		if (!isLastBlock) {
			if (tryExpand(block, nextBlock, size))
				return ptr;
			doGC();
			gcDone = true;
			if (tryExpand(block, nextBlock, size))
				return ptr;
		}
		uint32_t origFlags = block->info & MASK_COPY_FLAGS;
		void *newAlloc = tryAlloc(size, FLAG_STATIC);
		if (newAlloc == NULL) {
			if (gcDone)
				return NULL;
			doGC();
			newAlloc = tryAlloc(size, FLAG_STATIC);
			if (newAlloc == NULL)
				return NULL;
		}
		memcpy(newAlloc, ptr, blockSize);
		free(ptr);
		block = (HEADER *)((uintptr_t)newAlloc - SIZEOF_HEADER_USED);
		block->info |= origFlags;
		return newAlloc;
	}
	else if (size + SIZEOF_HEADER_USED + MIN_ALLOC < blockSize) {
		// we can release the remaining space. We don't coerce with next block, that will be done later by releaseUnmarked.
		HEADER *newBlock = (HEADER *)((uintptr_t)ptr + size);
		newBlock->info = blockSize - size - SIZEOF_HEADER_USED;
		lastFreeMem += newBlock->info;
		int i = getTLSFSlot(newBlock->info);
		assert(i < tableSize);
		newBlock->nextFree = table[i];
		table[i] = newBlock;
		block->info = size | FLAG_USED | (block->info & MASK_COPY_FLAGS);
	}
	return ptr;
}

void *Allocator::gcAlloc(size_t size)
{
	return alloc(size, 0);
}

void Allocator::releaseUnmarked()
{
	lastFreeMem = 0;
	for (int i = 0; i < tableSize; i++) {
		table[i] = NULL;
	}
	// first pass: clear weak references to unmarked objects
	for (MEMPOOL *pool = pools; pool->length != 0; pool++) {
		// enumerate blocks
		HEADER *block = (HEADER *)pool->address;
		while ((uintptr_t)block < pool->address + pool->length) {
			if (((block->info & (FLAG_USED | FLAG_OBJECT)) == (FLAG_USED | FLAG_OBJECT)) && ((block->info & (FLAG_STATIC | FLAG_GCMARK))) != 0) {
				Object *obj = (Object *)block->data;
				obj->gcClearWeakFields(); // clear weak references
			}
			block = (HEADER *)((uintptr_t)block + (block->info & MASK_SIZE) + SIZEOF_HEADER_USED);
		}
		assert((uintptr_t)block == pool->address + pool->length);
	}
	// second pass: release
	for (MEMPOOL *pool = pools; pool->length != 0; pool++) {
		// enumerate blocks
		HEADER *block = (HEADER *)pool->address;
		size_t size;
		while ((uintptr_t)block < pool->address + pool->length) {
			if ((block->info & (FLAG_USED | FLAG_GCMARK)) == (FLAG_USED | FLAG_GCMARK) || (block->info & (FLAG_USED | FLAG_STATIC)) == (FLAG_USED | FLAG_STATIC)) {
				// block is marked or static, keep it
				block->info &= ~FLAG_GCMARK;
				size = block->info & MASK_SIZE;
				assert(size > 0);
				block = (HEADER *)((uintptr_t)block + size + SIZEOF_HEADER_USED);
			}
			else {
				// block can be freed, releasing memory and coercing next blocks if applicable
				HEADER *nextBlock = block;
				do {
					assert((nextBlock->info & FLAG_GCMARK) == 0);
					if ((nextBlock->info & (FLAG_USED | FLAG_OBJECT)) == (FLAG_USED | FLAG_OBJECT)) {
						Object *obj = (Object *)nextBlock->data;
						obj->~Object(); // calling destructor
					}
					size = nextBlock->info & MASK_SIZE;
					assert(size > 0);
					nextBlock = (HEADER *)((uintptr_t)nextBlock + size + SIZEOF_HEADER_USED);
				} while ((uintptr_t)nextBlock < pool->address + pool->length && (nextBlock->info & (FLAG_USED | FLAG_GCMARK)) != (FLAG_USED | FLAG_GCMARK) && (nextBlock->info & (FLAG_USED | FLAG_STATIC)) != (FLAG_USED | FLAG_STATIC));
				block->info = (uintptr_t)nextBlock - (uintptr_t)block - SIZEOF_HEADER_USED;
				lastFreeMem += block->info;
				int j = getTLSFSlot(block->info);
				assert(j < tableSize);
				block->nextFree = table[j];
				table[j] = block;
				block = nextBlock;
			}
		}
		assert((uintptr_t)block == pool->address + pool->length);
	}
}

namespace SysCallsImpl
{

void *malloc(size_t size)
{
	ContextSwitchLock __csl;
	return ram.malloc(size);
}


void *realloc(void *ptr, size_t size)
{
	ContextSwitchLock __csl;
	return ram.realloc(ptr, size);
}

void free(void *ptr)
{
	ContextSwitchLock __csl;
	ram.free(ptr);
}

void *gcAlloc(void *&ptr, size_t size)
{
	ContextSwitchLock __csl;
	ptr = ram.gcAlloc(size);
	memset(ptr, 0, size);
	memBarrier();
	return ptr;
}

void addGlobalRef(GCGlobalItem *ref)
{
	ContextSwitchLock __csl;
	GCGlobalItem **root = &globalRefs;
	if (*root)
		(*root)->prevRef = (GCGlobalItem **)&ref->next;
	ref->next = *root;
	ref->prevRef = root;
	*root = ref;
}

void removeGlobalRef(GCGlobalItem *ref)
{
	ContextSwitchLock __csl;
	*(ref->prevRef) = (GCGlobalItem *)ref->next;
}

}

}
