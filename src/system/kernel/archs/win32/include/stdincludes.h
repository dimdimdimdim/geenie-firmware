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
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <intrin.h>
#include <math.h>
#include <float.h>
#include <limits>

#ifdef _DEBUG
#define ASSERT assert
#else
#define ASSERT(x)
#endif

inline int __builtin_clz(uint32_t n)
{
	unsigned long msbset = 0;
	if (_BitScanReverse(&msbset, n))
		return 31 - msbset;
	else
		return 32;
}

#define isnan(x)	_isnan(x)
#ifndef NAN
#define NAN (std::numeric_limits<double>::quiet_NaN())
#endif

#define memBarrier()	_ReadWriteBarrier()
#pragma warning(disable:4351)
#pragma warning(disable:4250)

#ifndef __PLACEMENT_NEW_INLINE
inline void *operator new(size_t, void *p)     throw() { return p; }
inline void *operator new[](size_t, void *p)   throw() { return p; }
inline void  operator delete  (void *, void *) throw() { };
inline void  operator delete[](void *, void *) throw() { };
#endif

typedef void *HANDLE;

template<class T> const T &max(const T &a, const T &b)
{
	return a > b ? a : b;
}

template<class T> const T &min(const T &a, const T &b)
{
	return a < b ? a : b;
}

#define RODATA
