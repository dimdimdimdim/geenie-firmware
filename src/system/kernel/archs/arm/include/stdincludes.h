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
#include <string.h>
#include <errno.h>
#include <math.h>
#include <float.h>
#include <limits>

typedef struct { } dummy_t;

#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C
#endif

EXTERN_C void abort();

#ifdef DEBUG
#ifdef __cplusplus
#define assert(x)		do { if (!(x)) ::abort(); } while(0)
#else
#define assert(x)		do { if (!(x)) abort(); } while(0)
#endif
#else
#define assert(x)		(void)0
#endif

#define memBarrier()	asm volatile("" : : : "memory")

#define alloca(size)   __builtin_alloca(size)

#ifdef __cplusplus

template<class T> const T &max(const T &a, const T &b)
{
	return a > b ? a : b;
}

template<class T> const T &min(const T &a, const T &b)
{
	return a < b ? a : b;
}

#define RODATA __attribute__((section(".rodata")))

#endif
