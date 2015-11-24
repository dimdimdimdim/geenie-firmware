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

EXTERN_C void abort(void)
{
	for (;;) { }
}

EXTERN_C void *malloc(size_t size)
{
	return SysCalls::malloc(size);
}

EXTERN_C void free(void *ptr)
{
	SysCalls::free(ptr);
}

EXTERN_C void *realloc(void *ptr, size_t size)
{
	return SysCalls::realloc(ptr, size);
}

EXTERN_C void *memcpy(void *dest, const void *src, size_t n)
{
	char *pcD = (char *)dest, *pcS = (char *)src;
	while (n--)
		*pcD++ = *pcS++;
	return dest;
}

EXTERN_C void *memmove(void *dest, const void *src, size_t n)
{
	char *pcD = (char *)dest;
	const char *pcS = (const char *)src;
	if ((size_t)pcD < (size_t)pcS) {
		while (n--)
			*pcD++ = *pcS++;
	}
	else if ((size_t)pcD > (size_t)pcS) {
		pcD += n;
		pcS += n;
		while (n--)
			*--pcD = *--pcS;
	}
	return dest;
}

EXTERN_C void *memset(void *ptr, int value, size_t n)
{
	char *pc = (char *)ptr;
	while (n--)
		*pc++ = (char)value;
	return ptr;
}

EXTERN_C size_t strlen(const char *str)
{
	const char *orig_str = str;
	while (*str != 0)
		str++;
	return str - orig_str;
}

EXTERN_C int strcmp(const char *str1, const char *str2)
{
	while (*str1 != '\0' && *str1 == *str2)
		str1++, str2++;
	return (*(unsigned char *)str1) - (*(unsigned char *)str2);
}

EXTERN_C int *__errno(void)
{
	return &Kernel::getCurrentThread()->context.errnoValue;
}

