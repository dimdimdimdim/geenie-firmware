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

#include "../../kernel/memory/allocator.h"
#include <util/macros.h>

namespace Kernel
{
class GCConstrItem;
}

typedef const void *TypeId;

template<class T> TypeId getStaticTypeId(const T &object)
{
#ifdef _MSC_VER
	return getStaticTypeId<T>;
#else
	return (TypeId)getStaticTypeId<T>;
#endif
}

#define typeId(x) (getStaticTypeId(*(x *)NULL))

/* You can eventually use multiple inheritance on subclasses, but there must be only one
   base class that derive from Object, it must be publicly derived, and it must be
   the leftmost class in the inheritance declaration.
   This relies on compiler behavior that is not standardized, but this works
   with MSVC and GCC. */
class Object
{
protected:
	virtual void *getClassPointer(TypeId t)				{ if (t == typeId(Object)) return this; return NULL; }
	virtual const void *getClassPointer(TypeId t) const	{ if (t == typeId(Object)) return this; return NULL; }

public:
	// do not call that yourself
	virtual void gcMarkFields() const					{ }
	virtual void gcClearWeakFields() const				{ }

public:
	/* Objects stored on the stack are added to garbage collector roots only after all constructors of the
	   hierarchy have been completed. This means that if a garbage collection is triggered during object 
	   construction, all allocations that have been made will be released.
       Therefore, for objects that may lie on the stack, it is FORBIDDEN to allocate anything
       within the constructors. Use an init() method for this.
	   On the other hand, it is MANDATORY to set all gced pointers to NULL.
	   These restrictions, however, do not apply for classes whose instances _only_ reside on the heap
	   (when all instances are allocated through gcnew). These classes may allocate other objects from
	   their constructors. */
	constexpr Object()									{ }
	void *operator new(size_t sz, const Kernel::GCConstrItem &citem);
	void operator delete(void *p, const Kernel::GCConstrItem &citem) { }
	void *operator new(size_t sz, void *ptr) { return ptr; }
	void operator delete(void *p, void *ptr) { }
	void operator delete(void *p) { }

	/* Destructor is called by the garbage collector when the object is destroyed.
       However, unlike Java, you can't re-reference the object from the destructor, that
       would lead to an invalid reference because the object is destroyed anyway and the
       memory is released.
	   Also don't call any memory management function (alloc, free) within the destructor.
	   ... Actually, just don't override it. It's best. */
	virtual ~Object()								{ }

	virtual TypeId getTypeId() const				{ return typeId(Object); }
	template<class TT> TT *dynCast()				{ return (TT *)getClassPointer(typeId(TT)); }
	template<class TT> const TT *dynCast() const	{ return (const TT *)getClassPointer(typeId(TT)); }
	template<class TT> bool isA() const				{ return getClassPointer(typeId(TT)) != NULL; }
};

/* Interface work like java.
   They keep a reference to the main object, so it can be tracked by the garbage collector.
   Objects implementing interface can only be created using gcnew. Thay can't be allocated on the stack or be in rom/flash. */
class Interface
{
private:
	Object *base;

protected:
	virtual void *getClassPointer(TypeId t)				{ if (t == typeId(Interface)) return this; return NULL; }
	virtual const void *getClassPointer(TypeId t) const	{ if (t == typeId(Interface)) return this; return NULL; }

public:
	// do not call that yourself
	virtual void gcMarkFields() const					{ }
	virtual void gcClearWeakFields() const				{ }

public:
	Interface();
	virtual ~Interface()							{ }

	Object *getBaseObject()							{ return base; }
	const Object *getBaseObject() const				{ return base; }
	virtual TypeId getTypeId() const				{ return typeId(Interface); }
	template<class TT> TT *dynCast()				{ return (TT *)base->getClassPointer(typeId(TT)); }
	template<class TT> const TT *dynCast() const	{ return (const TT *)base->getClassPointer(typeId(TT)); }
	template<class TT> bool isA() const				{ return getClassPointer(typeId(TT)) != NULL; }
};

template<class TT, int n> class GCObjectMarker				{ public: static void mark(const TT &x)				{ } };
template<class TT> class GCObjectMarker<TT, 1>				{ public: static void mark(const TT &x)				{ x.gcMarkFields(); } };
template<class TT> class GCObjectMarker<TT, 2>				{ public: static void mark(const TT &x)				{ x.getBaseObject()->gcMarkFields(); } };
template<class TT> class GCMarker							{ public: static void mark(const TT &x)				{ GCObjectMarker<TT, IsTypeDerivedFrom<TT, const Object>::value ? 1 : IsTypeDerivedFrom<TT, const Interface>::value ? 2 : 0>::mark(x); } };
template<class TT, size_t n> class GCMarker<TT [n]>			{ public: static void mark(TT (&x)[n])				{ for (size_t i = 0; i < n; i++) GCMarker<TT>::mark(x[i]); } };
template<class TT, size_t n> class GCMarker<const TT [n]>	{ public: static void mark(const TT (&x)[n])		{ for (size_t i = 0; i < n; i++) GCMarker<TT>::mark(x[i]); } };
template<class TT> class GCMarker<TT *>						{ public: static void mark(TT *x)					{ if (x && Kernel::isInRam(x) && Kernel::Allocator::mark(x)) GCMarker<TT>::mark(*x); } };
template<class TT> class GCMarker<const TT *>				{ public: static void mark(const TT *x)				{ if (x && Kernel::isInRam(x) && Kernel::Allocator::mark(x)) GCMarker<TT>::mark(*x); } };
template<class TT> class GCMarker<TT * const>				{ public: static void mark(TT *x)					{ if (x && Kernel::isInRam(x) && Kernel::Allocator::mark(x)) GCMarker<TT>::mark(*x); } };
template<class TT> class GCMarker<const TT * const>			{ public: static void mark(const TT *x)				{ if (x && Kernel::isInRam(x) && Kernel::Allocator::mark(x)) GCMarker<TT>::mark(*x); } };
template<> class GCMarker<void *>							{ public: static void mark(void *x)					{ if (x && Kernel::isInRam(x)) Kernel::Allocator::mark(x); } };
template<> class GCMarker<const void *>						{ public: static void mark(const void *x)			{ if (x && Kernel::isInRam(x)) Kernel::Allocator::mark(x); } };
template<> class GCMarker<void * const>						{ public: static void mark(void *x)					{ if (x && Kernel::isInRam(x)) Kernel::Allocator::mark(x); } };
template<> class GCMarker<const void * const>				{ public: static void mark(const void *x)			{ if (x && Kernel::isInRam(x)) Kernel::Allocator::mark(x); } };

template<class TT> void gcMarkObject(TT &x)					{ GCMarker<TT>::mark(x); }

class GCRootItem
{
protected:
	GCRootItem *next;

public:
	GCRootItem *getNext()		{ return next; }
	virtual void markObject() = 0;
};

class GCStackItem;
class GCGlobalItem;

namespace Kernel
{
void addStackRef(GCStackItem *ref);
void removeStackRef(GCStackItem *ref);
namespace SysCallsImpl
{
void addGlobalRef(GCGlobalItem *ref);
void removeGlobalRef(GCGlobalItem *ref);
}
}

class GCStackItem : public GCRootItem
{
private:
	friend void Kernel::addStackRef(GCStackItem *ref);
	friend void Kernel::removeStackRef(GCStackItem *ref);

protected:
	void addRef()				{ Kernel::addStackRef(this); }
	void removeRef()			{ Kernel::removeStackRef(this); }
};

class GCGlobalItem : public GCRootItem
{
private:
	GCGlobalItem **prevRef;
	friend void Kernel::SysCallsImpl::addGlobalRef(GCGlobalItem *ref);
	friend void Kernel::SysCallsImpl::removeGlobalRef(GCGlobalItem *ref);

protected:
    void addRef();
	void removeRef();
};

/* gc<T> objects must lie on the _stack_ only and must only point to objects that have been dynamically allocated in RAM. 
   Each Ref object must only be used from the thread that owns the stack they reside on.
   They are used when returning objects from a function/method, or for local variables.
   There is no need to use them within function arguments, because we assume the reference
   already comes from a referenced object so the object cannot be destroyed within the call.
   gc<T, GCGlobalItem> objects must lie on the _heap_ only.*/
template <class T, class GCType = GCStackItem> class gc : public T, private GCType
{
public:
	typedef gc<T, GCType> Type;

	gc()														{ memBarrier(); GCType::addRef(); }
	gc(const gc<T, GCType> &object)								:T(object) { memBarrier(); GCType::addRef(); }
	gc(const T &object)											:T(object) { memBarrier(); GCType::addRef(); }
	gc<T, GCType> &operator=(const T &object)					{ T::operator=(object); return *this; }
	gc<T, GCType> &operator=(const gc<T, GCType> &object)		{ T::operator=(static_cast<T>(object)); return *this; }
	~gc()														{ GCType::removeRef(); }
private:
	virtual void markObject()									{ gcMarkObject(*this); }
};
template <class T, class GCType> class gc<T *, GCType> : private GCType
{
private:
	T *object;
public:
	typedef gc<T *, GCType> Type;

	gc()										{ object = NULL; memBarrier(); GCType::addRef(); }
	template<class U, class GCT> gc(const gc<U *, GCT> &object)	 { static_assert(IsTypeDerivedFrom<U, T>::value, "implicit upcast forbidden"); this->object = object.get(); memBarrier(); GCType::addRef(); }
	gc(const gc<T *, GCType> &object)			{ this->object = object.object; memBarrier(); GCType::addRef(); }
	gc(T *object)								{ this->object = object; memBarrier(); GCType::addRef(); }
	~gc()										{ GCType::removeRef(); }

	template<class U, class GCT> gc<T *, GCType> &operator=(const gc<U *, GCT> &object)		{ static_assert(IsTypeDerivedFrom<U, T>::value, "implicit upcast forbidden"); this->object = object.get(); return *this; }
	gc<T *, GCType> &operator=(const gc<T *, GCType> &object)								{ this->object = object.object; return *this; }
	T *operator=(T *object)						{ this->object = object; return object; }
	T &operator*() const						{ return *object; }
	T *operator->() const						{ return object; }
	operator T*() const							{ return object; }
	T *get() const								{ return object; }
	template<class U> operator gc<U *>() const	{ static_assert(IsTypeDerivedFrom<T, U>::value, "implicit upcast forbidden"); return gc<U *>(static_cast<U *>(object)); }
	template<class U> U staticCast() const		{ return static_cast<U>(object); }
	bool operator==(T *object) const			{ return this->object == object; }
	bool operator!=(T *object) const			{ return this->object != object; }
private:
	virtual void markObject()				{ gcMarkObject(object); }
};
template <class GCType> class gc<void *, GCType> : private GCType
{
private:
	void *object;
public:
	typedef gc<void *, GCType> Type;

	gc()									{ object = NULL; memBarrier(); GCType::addRef(); }
	gc(const gc<void *, GCType> &object)	{ this->object = object.object; memBarrier(); GCType::addRef(); }
	gc(void *object)						{ this->object = object; memBarrier(); GCType::addRef(); }
	~gc()									{ GCType::removeRef(); }

	gc<void *, GCType> &operator=(const gc<void *, GCType> &object) { this->object = object.object; return *this; }
	void *operator=(void *object)			{ this->object = object; memBarrier(); return object; }
	operator void*()						{ return object; }
	void *get()								{ return object; }
private:
	virtual void markObject()				{ gcMarkObject(object); }
};
template<class GCType> class gc<bool, GCType> : private GCType { public: typedef bool Type; };
template<class GCType> class gc<signed char, GCType> : private GCType { public: typedef signed char Type; };
template<class GCType> class gc<unsigned char, GCType> : private GCType { public: typedef unsigned char Type; };
template<class GCType> class gc<signed short, GCType> : private GCType { public: typedef signed short Type; };
template<class GCType> class gc<unsigned short, GCType> : private GCType { public: typedef unsigned short Type; };
template<class GCType> class gc<signed int, GCType> : private GCType { public: typedef signed int Type; };
template<class GCType> class gc<unsigned int, GCType> : private GCType { public: typedef unsigned int Type; };
template<class GCType> class gc<signed long, GCType> : private GCType { public: typedef signed long Type; };
template<class GCType> class gc<unsigned long, GCType> : private GCType { public: typedef unsigned long Type; };
template<class GCType> class gc<signed long long, GCType> : private GCType { public: typedef signed long long Type; };
template<class GCType> class gc<unsigned long long, GCType> : private GCType { public: typedef unsigned long long Type; };
template<class GCType> class gc<float, GCType> : private GCType { public: typedef float Type; };
template<class GCType> class gc<double, GCType> : private GCType { public: typedef double Type; };

template<class TT, bool iface> class GCMarkChecker { public: static bool check(TT *object) { return object && !Kernel::Allocator::isMarked(object); } };
template<class TT> class GCMarkChecker<TT, true> { public: static bool check(TT *object) { return object && !Kernel::Allocator::isMarked(object->getBaseObject()); } };
/* Weak refs can only lie in structures or objects, and must be included in GC_FIELDS declaration
   They do not prevent garbage collection of the objects, and are automatically set to NULL by
   the garbage collector when the object gets destroyed. */
template<class T> class WeakRef
{
private:
	mutable T *object;

public:
	// do not call that yourself
	void checkClear() const					{ if (GCMarkChecker<T, IsTypeDerivedFrom<T, const Interface>::value>::check(object)) object = NULL; }

public:
	WeakRef()								{ }
	~WeakRef()								{ }

	T *operator=(T *object)					{ this->object = object; return object; }
	gc<T *> get();
	T *unsafeGet()							{ return object; }
};

template<class TT, int n> class GCObjectWeakFieldClearer				{ public: static void clear(const TT &x)				{ } };
template<class TT> class GCObjectWeakFieldClearer<TT, 1>				{ public: static void clear(const TT &x)				{ x.gcClearWeakFields(); } };
template<class TT> class GCWeakFieldClearer								{ public: static void clear(const TT &x)				{ GCObjectWeakFieldClearer<TT, IsTypeDerivedFrom<TT, const Object>::value ? 1 : 0>::clear(x); } };
template<class TT> class GCWeakFieldClearer<WeakRef<TT>>				{ public: static void clear(WeakRef<TT> &x)				{ x.checkClear(); } };
template<class TT> class GCWeakFieldClearer<WeakRef<const TT>>			{ public: static void clear(WeakRef<const TT> &x)		{ x.checkClear(); } };
template<class TT> class GCWeakFieldClearer<const WeakRef<TT>>			{ public: static void clear(const WeakRef<TT> &x)		{ x.checkClear(); } };
template<class TT> class GCWeakFieldClearer<const WeakRef<const TT>>	{ public: static void clear(const WeakRef<const TT> &x)	{ x.checkClear(); } };
template<class TT, size_t n> class GCWeakFieldClearer<TT [n]>			{ public: static void clear(TT (&x)[n])					{ for (size_t i = 0; i < n; i++) GCWeakFieldClearer<TT>::clear(x[i]); } };
template<class TT, size_t n> class GCWeakFieldClearer<const TT [n]>		{ public: static void clear(const TT (&x)[n])			{ for (size_t i = 0; i < n; i++) GCWeakFieldClearer<TT>::clear(x[i]); } };

template<class TT> inline void gcClearWeakField(TT &x)					{ GCWeakFieldClearer<TT>::clear(x); }

#define _GET_BASE_CLASS_POINTER(x)										ptr = x::getClassPointer(t); if (ptr) return ptr;
#define _GC_MARK_BASE_FIELDS(x)											x::gcMarkFields();
#define _GC_CLEAR_BASE_WEAK_FIELDS(x)									x::gcClearWeakFields();
#define _GC_MARK_FIELD(x)												gcMarkObject(x);
#define _GC_CLEAR_WEAK_FIELD(x)											gcClearWeakField(x);

#define GC_INHERITS(...) \
	protected: \
	virtual void *getClassPointer(TypeId t)				{ if (t == getStaticTypeId(*this)) return this; return (void *)getBaseClassPointer(t); } \
	virtual const void *getClassPointer(TypeId t) const	{ if (t == getStaticTypeId(*this)) return this; return getBaseClassPointer(t); } \
	public: \
	virtual TypeId getTypeId() const					{ return getStaticTypeId(*this); } \
	private: \
	const void *getBaseClassPointer(TypeId t) const		{ const void *ptr; ITERATE(_GET_BASE_CLASS_POINTER, __VA_ARGS__) return NULL; } \
	void gcMarkBaseFields() const						{ ITERATE(_GC_MARK_BASE_FIELDS, __VA_ARGS__) } \
	void gcClearBaseWeakFields() const					{ ITERATE(_GC_CLEAR_BASE_WEAK_FIELDS, __VA_ARGS__) }
#define GC_FIELDS(...) \
	public: \
	virtual void gcMarkFields() const				{ gcMarkBaseFields(); ITERATE(_GC_MARK_FIELD, __VA_ARGS__); } \
	virtual void gcClearWeakFields() const			{ gcClearBaseWeakFields(); ITERATE(_GC_CLEAR_WEAK_FIELD, __VA_ARGS__); }
#define GC_NO_FIELDS \
	public: \
	virtual void gcMarkFields() const				{ gcMarkBaseFields(); } \
	virtual void gcClearWeakFields() const			{ gcClearBaseWeakFields(); }

#define localCopy(x)	(*(x.copyTo(alloca(x.getObjectSize()))))