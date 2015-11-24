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
#include <criticalsection.h>
#include <sysutils.h>

void CriticalSection::lock()
{
	if (status != 0 && thread == Kernel::getCurrentThread()) {
		// just increment status
		asm volatile ("\
		1:                                                                                \n\
			ADD		r12, %[obj], %[status]		/* address of status field */	          \n\
			LDREX	r1, [r12]    				/* load the lock status */                \n\
			ADD		r1, r1, #1					/* increment */                           \n\
			STREX	r1, r1, [r12]				/* update status */                       \n\
			CMP		r1, #0              		/* check if update succeeded */           \n\
			BNE		1b                                                                    \n\
		" : : [obj]"r"(this), [status]"X"(__builtin_offsetof(CriticalSection, status)) : "r1", "r12", "cc", "memory");
	}
	else {
		uint32_t failure;
		asm volatile ("\
			ADD		r12, %[obj], %[status]		/* address of status field */	          \n\
			MOV		r1, #1 		   				/* 'lock taken' value */                  \n\
			LDREX	%[failure], [r12]    		/* load the lock status */                \n\
			CMP		%[failure], #0            	/* is the lock free? */                   \n\
			ITE		EQ                                                            		  \n\
			STREXEQ	%[failure], r1, [r12]		/* try and claim the lock */              \n\
			MOVNE	%[failure], #1				/* otherwise: set failure status */       \n\
		" : [failure]"=r"(failure) : [obj]"r"(this), [status]"X"(__builtin_offsetof(CriticalSection, status)) : "r1", "r12", "cc", "memory");
		if (failure) {
			SysCalls::lockCriticalSection(this);
		}
		registerThreadLock();
	}
}
	
void CriticalSection::unlock()
{
	assert(status != 0 && thread == Kernel::getCurrentThread());
	if ((status & ~WAITERS_FLAG) > 1) {
		// just decrement status
		asm volatile ("\
		1:                                                                                \n\
			ADD		r12, %[obj], %[status]		/* address of status field */	          \n\
			LDREX	r1, [r12]    				/* load the lock status */                \n\
			SUB		r1, r1, #1					/* decrement */                           \n\
			STREX	r1, r1, [r12]				/* update status */                       \n\
			CMP		r1, #0              		/* check if update succeeded */           \n\
			BNE		1b                                                          		  \n\
		" : : [obj]"r"(this), [status]"X"(__builtin_offsetof(CriticalSection, status)) : "r1", "r12", "cc", "memory");
	}
	else {
		unregisterThreadLock();
		uint32_t failure;
		asm volatile ("\
			ADD		r12, %[obj], %[status]		/* address of status field */	          \n\
			MOV		r1, #0       				/* 'lock free' value */                   \n\
			LDREX	%[failure], [r12]    		/* load the lock status */                \n\
			CMP		%[failure], #1            	/* is the lock taken with no waiter ? */  \n\
			ITE		EQ                                                            		  \n\
			STREXEQ	%[failure], r1, [r12]		/* release the lock */                    \n\
			MOVNE	%[failure], #1				/* otherwise: set failure status */       \n\
		" : [failure]"=r"(failure) : [obj]"r"(this), [status]"X"(__builtin_offsetof(CriticalSection, status)) : "r1", "r12", "cc", "memory");
		if (failure) {
			SysCalls::unlockCriticalSection(this);
		}
	}
}
