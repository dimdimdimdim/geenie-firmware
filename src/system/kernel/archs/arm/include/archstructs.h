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
#include <exception>

/* from unwind-cxx.h */

struct _Unwind_Exception
{
	// don't know what's inside... don't even know where the definition of this is, but we don't care.
};

struct __cxa_exception
{
  std::type_info *exceptionType;
  void (*exceptionDestructor)(void *); 
  std::unexpected_handler unexpectedHandler;
  std::terminate_handler terminateHandler;
  __cxa_exception *nextException;
  int handlerCount;
  __cxa_exception* nextPropagatingException;
  int propagationCount;
  _Unwind_Exception unwindHeader;
};

struct __cxa_eh_globals
{
  __cxa_exception *caughtExceptions;
  unsigned int uncaughtExceptions;
  __cxa_exception *propagatingExceptions;
};

struct ThreadArchContext
{
	uint32_t *registers;
	int errnoValue;
	__cxa_eh_globals exceptionState;
};
