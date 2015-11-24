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

#include "circarray.h"

// see http://opendatastructures.org/ods-java/3_3_SEList_Space_Efficient_.html

template<class T> class ListImpl : public Object
{
	GC_INHERITS(Object);

protected:
	static const int blockSize = 8;
	typedef T ItemType;

	class Block : public Object
	{
		GC_INHERITS(Object);
		GC_FIELDS(items);
	public:
		CircArray<T, blockSize + 1> items;
		Block *next, *prev;
	};
	Block *firstBlock;
	int length;

public:
	virtual void gcMarkFields()	const
	{
		Block *block = firstBlock;
		if (block != NULL) {
			do {
				gcMarkObject(block);
				block = block->next;
			}
			while (block != firstBlock);
		}
	}

protected:
	void getLocation(int &i, Block *&b) const
	{
		assert(i < length);
		if (i < length / 2) {
			Block *block = firstBlock;
			while (i >= block->items.getLength()) {
				i -= block->items.getLength();
				block = block->next;
			}
			b = block;
		}
		else {
			i = length - 1 - i;
			Block *block = firstBlock->prev;
			while (i >= block->items.getLength()) {
				i -= block->items.getLength();
				block = block->prev;
			}
			i = block->items.getLength() - 1 - i;
			b = block;
		}
	}

	void removeAt(int i, Block *lu)
	{
		Block *u = lu;
		int r = 0;
		while (r < blockSize && u->next != firstBlock && u->items.getLength() == blockSize - 1) {
			u = u->next;
			r++;
		}
		if (r == blockSize && u->next != firstBlock) {
			// gather and remove block
			u = lu;
			for (int j = 0; j < blockSize - 1; j++) {
				while (u->items.getLength() < blockSize) {
					u->items.pushBack(u->next->items.getFront());
					memBarrier();
					u->next->items.removeFront();
					u = u->next;
				}
			}
			u->prev->next = u->next;
			u->next->prev = u->prev;
			lu->items.removeAt(i);
			length--;
		}
		else {
			// work forward to shift blocks contents
			lu->items.removeAt(i);
			length--;
			while (lu->items.getLength() < blockSize - 1 && lu->next != firstBlock) {
				lu->items.pushBack(lu->next->items.getFront());
				memBarrier();
				lu->next->items.removeFront();
				lu = lu->next;
			}
			if (lu->next == firstBlock && lu->items.getLength() == 0) {
				// remove block at the end
				if (lu == firstBlock) {
					assert(length == 0);
					firstBlock = NULL;
				}
				else {
					lu->prev->next = lu->next;
					lu->next->prev = lu->prev;
				}
			}
		}
	}

	void insertAt(int i, Block *lu, const T &object)
	{
		Block *u = lu;
		int r = 0;
		while (r < blockSize && u->next != firstBlock && u->items.getLength() == blockSize + 1) {
			u = u->next;
			r++;
		}
		if (r == blockSize && u->next != firstBlock) {
			// add new block and spread
			gc<Block *> newBlock = gcnew(Block);
			newBlock->next = u;
			newBlock->prev = u->prev;
			memBarrier();
			u->prev->next = newBlock;
			u->prev = newBlock;
			u = newBlock;
			while (u != lu) {
				while (u->items.getLength() < blockSize) {
					u->items.pushFront(u->prev->items.getBack());
					memBarrier();
					u->prev->items.removeBack();
					u = u->prev;
				}
			}
		}
		else {
			if (u->next == firstBlock && u->items.getLength() == blockSize + 1) {
				// add new block at the end
				gc<Block *> newBlock = gcnew(Block);
				newBlock->next = firstBlock;
				newBlock->prev = u;
				memBarrier();
				u->next = newBlock;
				firstBlock->prev = newBlock;
				u = newBlock;
			}
			// work backwards to shift blocks contents
			while (u != lu) {
				u->items.pushFront(u->prev->items.getBack());
				memBarrier();
				u->prev->items.removeBack();
				u = u->prev;
			}
		}
		u->items.insertAt(i, object);
		length++;
	}

	void copyFrom(const ListImpl<T> &src)
	{
		firstBlock = NULL;
		Block *lastBlock;
		Block *srcBlock = src.firstBlock;
		while (srcBlock) {
			Block *destBlock = gcnew(Block);
			for (int i = 0; i < srcBlock->items.getLength(); i++)
				destBlock->items.pushBack(srcBlock->items.getAt(i));
			if (firstBlock == NULL) {
				destBlock->next = destBlock;
				destBlock->prev = destBlock;
				firstBlock = destBlock;
			}
			else {
				destBlock->next = firstBlock;
				destBlock->prev = lastBlock;
				lastBlock->next = destBlock;
				firstBlock->prev = destBlock;
			}
			lastBlock = destBlock;
			srcBlock = srcBlock->next;
		}
		length = src.length;
	}

public:

	ListImpl()
	{
		firstBlock = NULL;
		length = 0;
	}

	ListImpl(const ListImpl<T> &src)
	{
		copyFrom(src);
	}

	int getLength() const
	{
		return length;
	}
	
	void clear()
	{
		firstBlock = NULL;
		length = 0;
	}

	const T &getAt(int i) const
	{
		Block *block;
		getLocation(i, block);
		return block->items.getAt(i);
	}

	T &getAt(int i)
	{
		Block *block;
		getLocation(i, block);
		return block->items.getAt(i);
	}

	void setAt(int i, const T &object)
	{
		Block *block;
		getLocation(i, block);
		block->items.setAt(i, object);
	}

	void append(const T &object)
	{
		if (firstBlock == NULL) {
			firstBlock = gcnew(Block);
			firstBlock->next = firstBlock;
			firstBlock->prev = firstBlock;
		}
		Block *lastBlock = firstBlock->prev;
		if (lastBlock->items.getLength() == blockSize + 1) {
			gc<Block *> newBlock = gcnew(Block);
			newBlock->next = firstBlock;
			newBlock->prev = lastBlock;
			memBarrier();
			lastBlock->next = newBlock;
			firstBlock->prev = newBlock;
			lastBlock = newBlock;
		}
		lastBlock->items.pushBack(object);
		length++;
	}

	void insertAt(int i, const T &object)
	{
		assert(i >= 0 && i <= length);
		if (i == length) {
			append(object);
		}
		else {
			Block *block;
			getLocation(i, block);
			insertAt(i, block, object);
		}
	}

	void removeAt(int i)
	{
		assert(i >= 0 && i < length);
		Block *block;
		getLocation(i, block);
		removeAt(i, block);
	}

	bool remove(const T &object)
	{
		int i;
		Block *block = firstBlock;
		if (block != NULL) {
			do {
				i = block->items.findIndexOf(object);
				if (i != -1) {
					removeAt(i, block);
					return true;
				}
				block = block->next;
			}
			while (block != firstBlock);
		}
		return false;
	}

	int find(const T &object)
	{
		int i, b = 0;
		Block *block = firstBlock;
		if (block != NULL) {
			do {
				i = block->items.findIndexOf(object);
				if (i != -1)
					return b + i;
				block = block->next;
				b += block->items.getLength();
			}
			while (block != firstBlock);
		}
		return -1;
	}
};

template<class Base, bool reversed = false> class ListWithIterators : public Base
{
private:
	typedef typename Base::Block Block;
	typedef typename Base::ItemType ItemType;

	using Base::firstBlock;
	using Base::length;

	/* Iterators are not designed to be tracked by garbage collector.
	   They do not keep a reference to the original list. Therefore, make sure you have a living
	   reference to the list while iterating.
	   Moreover, they are not thread-safe, the list must not be modified during iteration. */
protected:

	template<class R, class I, class L> friend class IteratorBase;

	template<class R, class I, class L> class IteratorBase
	{
	private:
		operator int() { return 0; }

	protected:
		friend class ListWithIterators<Base>;
		L *list;
		Block *block;
		int i;

		IteratorBase(L *list, Block *block, int i)
		{
			this->list = list;
			this->block = block;
			this->i = i;
		}
		I &set(L *list, Block *block, int i)
		{
			this->list = list;
			this->block = block;
			this->i = i;
			return *(I *)this;
		}
		void incrementIndex()
		{
			assert(block != NULL);
			if (++i == block->items.getLength()) {
				if ((block = block->next) == list->firstBlock)
					block = NULL;
				i = 0;
			}
		}
		void decrementIndex()
		{
			if (--i == -1) {
				if (block == list->firstBlock)
					block = NULL;
				else {
					block = block->prev;
					i = block->items.getLength() - 1;
				}
			}
		}
	public:
		IteratorBase()
		{
			list = NULL;
			block = NULL;
			i = 0;
		}
		IteratorBase(const IteratorBase<R, I, L> &it)
		{
			list = it.list;
			block = it.block;
			i = it.i;
		}
		I &operator=(const I &it)
		{
			list = it.list;
			block = it.block;
			i = it.i;
			return *(I *)this;
		}
		R operator*() const
		{
			assert(block != NULL);
			return (R)block->items.getAt(i);
		}
		R operator->() const
		{
			assert(block != NULL);
			return (R)block->items.getAt(i);
		}
		I &operator++()
		{
			if (reversed)
				decrementIndex();
			else
				incrementIndex();
			return *(I *)this;
		}
		I operator++(int)
		{
			I saved(*(I *)this);
			if (reversed)
				decrementIndex();
			else
				incrementIndex();
			return saved;
		}
		I &operator--()
		{
			if (reversed)
				incrementIndex();
			else
				decrementIndex();				
			return *(I *)this;
		}
		I operator--(int)
		{
			I saved(*(I *)this);
			if (reversed)
				incrementIndex();
			else
				decrementIndex();				
			return saved;
		}
		operator bool()
		{
			return block != NULL;
		}
		bool operator!()
		{
			return block == NULL;
		}
		bool operator==(const I &it)
		{
			return list == it.list && block == it.block && (block == NULL || i == it.i);
		}
	};

public:

	class Iterator : public IteratorBase<ItemType &, Iterator, ListWithIterators<Base> >
	{
	protected:
		friend class ListWithIterators<Base>;
		friend class ConstIterator;
		Iterator(ListWithIterators<Base> *list, Block *block, int i) :IteratorBase<ItemType &, Iterator, ListWithIterators<Base> >(list, block, i) { }

		using IteratorBase<ItemType &, Iterator, ListWithIterators<Base> >::list;
		using IteratorBase<ItemType &, Iterator, ListWithIterators<Base> >::block;
		using IteratorBase<ItemType &, Iterator, ListWithIterators<Base> >::i;

	public:
		Iterator()													:IteratorBase<ItemType &, Iterator, ListWithIterators<Base> >() { }
		Iterator(const Iterator &i)									:IteratorBase<ItemType &, Iterator, ListWithIterators<Base> >(i) { }

		void removeItem()
		{
			assert(block != NULL);
			if (block == list->firstBlock->prev && i == block->items.getLength() - 1) {
				list->removeAt(i, block);
				block = NULL;
			}
			else {
				list->removeAt(i, block);
			}
		}

		void insertItem(const ItemType &item)
		{
			if (block == NULL) {
				list->insertAt(i, block, item);
				block = list->firstBlock->prev;
				i = list->firstBlock->prev->items.getLength() - 1;
			}
			else {
				list->insertAt(i, block, item);
			}
		}

	};

	class ConstIterator : public IteratorBase<const ItemType &, ConstIterator, const ListWithIterators<Base> >
	{
	protected:
		friend class ListWithIterators<Base>;
		ConstIterator(const ListWithIterators<Base> *list, Block *block, int i)	 :IteratorBase<const ItemType &, ConstIterator, const ListWithIterators<Base> >(list, block, i) { }

		using IteratorBase<const ItemType &, ConstIterator, const ListWithIterators<Base> >::list;
		using IteratorBase<const ItemType &, ConstIterator, const ListWithIterators<Base> >::block;
		using IteratorBase<const ItemType &, ConstIterator, const ListWithIterators<Base> >::i;

	public:
		ConstIterator()												:IteratorBase<const ItemType &, ConstIterator, const ListWithIterators<Base> >() { }
		ConstIterator(const ConstIterator &i)						:IteratorBase<const ItemType &, ConstIterator, const ListWithIterators<Base> >(i) { }
		ConstIterator(const Iterator &i)							:IteratorBase<const ItemType &, ConstIterator, const ListWithIterators<Base> >(i.list, i.block, i.i) { }
		ConstIterator &operator=(const Iterator &i)					{ return set(i.list, i.block, i.i); }
		bool operator==(const Iterator &i)							{ return list == i.list && block == i.block && (block == NULL || i == i.i); }
	};

	Iterator start()
	{
		if (reversed) {
			if (firstBlock == NULL)
				return Iterator();
			return Iterator(this, firstBlock->prev, firstBlock->prev->items.getLength() - 1);
		}
		else {
			return Iterator(this, firstBlock, 0);
		}
	}

	Iterator end()
	{
		if (reversed) {
			return Iterator(this, firstBlock, 0);
		}
		else {
			if (firstBlock == NULL)
				return Iterator();
			return Iterator(this, firstBlock->prev, firstBlock->prev->items.getLength() - 1);
		}
	}

	ConstIterator start() const
	{
		if (reversed) {
			if (firstBlock == NULL)
				return Iterator();
			return Iterator(this, firstBlock->prev, firstBlock->prev->items.getLength() - 1);
		}
		else {
			return Iterator(this, firstBlock, 0);
		}
	}

	ConstIterator end() const
	{
		if (reversed) {
			return Iterator(this, firstBlock, 0);
		}
		else {
			if (firstBlock == NULL)
				return Iterator();
			return Iterator(this, firstBlock->prev, firstBlock->prev->items.getLength() - 1);
		}
	}

	/*
	void removeAt(const Iterator &pos)
	{
		assert(pos.block != NULL);
		Base::removeAt(pos.i, pos.block);
	}

	void removeAt(int i)
	{
		Base::removeAt(i);
	}

	void insertAt(const Iterator &pos, const ItemType &object)
	{
		if (pos.block == NULL)
			Base::append(object);
		else
			Base::insertAt(pos.i, pos.block, object);
	}
	
	void insertAt(int i, const ItemType &object)
	{
		Base::insertAt(i, object);
	}
	*/
};

template<class T, bool isObject> class ListSpecialization : public ListImpl<T>
{
};

template<class T> class ListSpecialization<T, true> : public ListImpl<Object *>
{
protected:
	typedef T ItemType;
	void removeAt(int i, Block *lu)							{ ListImpl<Object *>::removeAt(i, lu); }
	void insertAt(int i, Block *lu, const T &object)		{ ListImpl<Object *>::insertAt(i, lu, (Object *)object); }

public:
	const T &getAt(int i) const								{ return (const T &)ListImpl<Object *>::getAt(i); }
	T &getAt(int i)											{ return (T &)ListImpl<Object *>::getAt(i); }
	void setAt(int i, const T &object)						{ ListImpl<Object *>::setAt(i, (Object *)object); }
	void append(const T &object)							{ ListImpl<Object *>::append((Object *)object); }
	void removeAt(int i)									{ ListImpl<Object *>::removeAt(i); }
	void insertAt(int i, const T &object)					{ ListImpl<Object *>::insertAt(i, (Object *)object); }
};

template<class T> class ListSpecialized : public ListSpecialization<T, false>
{
};

template<class T> class ListSpecialized<T *> : public ListSpecialization<T *, IsTypeDerivedFrom<T, const Object>::value>
{
};

template<class T> class List : public ListWithIterators<ListSpecialized<T> >
{
public:
	List()										{ }
	List(const List<T> &src)					{ copyFrom(src); }
	List<T> &operator=(const List<T> &src)		{ copyFrom(src); }
};

// stack reverses the order of items so push/pop operations are optimized
template<class T> class Stack : public ListWithIterators<ListSpecialized<T>, true>
{
public:
	Stack()													{ }
	Stack(const Stack<T> &src)								{ copyFrom(src); }
	Stack<T> &operator=(const Stack<T> &src)				{ copyFrom(src); }

	using ListSpecialized<T>::getLength;

	const T &getAt(int i) const								{ return ListSpecialized<T>::getAt(getLength() - 1 - i); }
	T &getAt(int i)											{ return ListSpecialized<T>::getAt(getLength() - 1 - i); }
	void setAt(int i, const T &object)						{ ListSpecialized<T>::setAt(getLength() - 1 - i, object); }
	void append(const T &object)							{ ListSpecialized<T>::insertAt(0, object); }
	void insertAt(int i, const T &object)					{ ListSpecialized<T>::insertAt(getLength() - 1 - i, object); }
	void removeAt(int i)									{ ListSpecialized<T>::removeAt(getLength() - 1 - i); }
	int find(const T &object)								{ int i = ListSpecialized<T>::find(object); if (i == -1) return -1; return getLength() - 1 - i; }

	void push(const T &object)								{ ListSpecialized<T>::append(object); }
	typename gc<T>::Type pop()								{ typename gc<T>::Type ret(getAt(0)); removeAt(0); return ret; }
};
