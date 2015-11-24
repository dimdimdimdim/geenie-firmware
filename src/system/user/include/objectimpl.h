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

class GCConstrItem : public GCRootItem
{
private:
	mutable void *object;
public:

	GCConstrItem()
	{
		GCConstrItem **root = &Kernel::getCurrentThread()->lastGCConstrItem;
		object = NULL;
		next = *root;
		memBarrier();
		*root = this;
	}

	~GCConstrItem()
	{
		GCConstrItem **root = &Kernel::getCurrentThread()->lastGCConstrItem;
		while (*root != this) {
			assert(*root != NULL);
			root = (GCConstrItem **)&((*root)->next);
		}
		*root = (GCConstrItem *)(*root)->next;
		memBarrier();
	}
	
	void *&getRef() const
	{
		return object;
	}

	template<class T> static gc<T *> extract(T *x)
	{
		assert(Kernel::getCurrentThread()->lastGCConstrItem != NULL && Kernel::getCurrentThread()->lastGCConstrItem->object == x);
		return gc<T *>(x);
	}

	virtual void markObject()
	{
		// compiler doesn't know the type at this point. we have to rely on the FLAG_OBJECT flag within the memory block header
		if (object && Allocator::mark(object) && Allocator::isObject(object))
			((Object *)object)->gcMarkFields();
	}

};

}

inline Interface::Interface()
{
	assert(Kernel::getCurrentThread()->lastGCConstrItem != NULL);
	base = (Object *)Kernel::getCurrentThread()->lastGCConstrItem->getRef(); // set the object base pointer
}

inline void *Object::operator new(size_t size, const Kernel::GCConstrItem &citem)
{
	void *ptr = SysCalls::gcAlloc(citem.getRef(), size);
	new(ptr) Object(); // set up vtable before setting FLAG_OBJECT - Object constructor is then called twice but it is empty anyway...
	memBarrier();
	Kernel::Allocator::setAsObject(ptr);
	return ptr;
}

inline void *operator new(size_t size, const Kernel::GCConstrItem &citem)
{
	return SysCalls::gcAlloc(citem.getRef(), size);
}

inline void operator delete(void *ptr, const Kernel::GCConstrItem &citem)
{
}

inline void *operator new[](size_t size, const Kernel::GCConstrItem &citem)
{
	return SysCalls::gcAlloc(citem.getRef(), size);
}

inline void operator delete[](void *ptr, const Kernel::GCConstrItem &citem)
{
}

// gcnew cannot be used to allocate arrays of Objects. Use Array<T> or List<T> instead.
// You can however allocate arrays of primitive types or arrays of
// struct/classes whose contents don't need to be tracked by the garbage collector
#define gcnew(x) (Kernel::GCConstrItem::extract(new((Kernel::GCConstrItem())) x))

template<class T> inline gc<T *> WeakRef<T>::get()
{
	Kernel::getCurrentThread()->tmpStackObjRef = (void **)&object;
	memBarrier();
	gc<T *> ref = object;
	memBarrier();
	Kernel::getCurrentThread()->tmpStackObjRef = NULL;
	return ref;
}

namespace Kernel
{

inline void addStackRef(GCStackItem *ref)
{
	GCStackItem **root = &Kernel::getCurrentThread()->lastGCStackItem;
	ref->next = *root;
	memBarrier();
	*root = ref;
}

inline void removeStackRef(GCStackItem *ref)
{
	GCStackItem **root = &Kernel::getCurrentThread()->lastGCStackItem;
	while (*root != ref) {
		assert(*root != NULL);
		root = (GCStackItem **)&((*root)->next);
	}
	*root = (GCStackItem *)(*root)->next;
	memBarrier();
}

}

inline void GCGlobalItem::addRef()
{
	SysCalls::addGlobalRef(this);
}

inline void GCGlobalItem::removeRef()
{
	SysCalls::removeGlobalRef(this);
}
