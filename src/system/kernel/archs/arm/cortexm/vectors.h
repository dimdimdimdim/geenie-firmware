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

template<int n> void __irqHandler();
#define irqHandler(n) template<> void __irqHandler<n>()

/*
EXTERN_C void handler_default();
EXTERN_C void __reset__();

class SystemVectors
{
private:
	void *stackTopPtr;
	IrqHandler resetPtr;
	IrqHandler nmiPtr;
	IrqHandler hardFaultPtr;
	IrqHandler memManagePtr;
	IrqHandler busFaultPtr;
	IrqHandler usageFaultPtr;
	void *rsvd1Ptr;
	void *rsvd2Ptr;
	void *rsvd3Ptr;
	void *rsvd4Ptr;
	IrqHandler svcPtr;
	IrqHandler debugMonPtr;
	void *rsvd5Ptr;
	IrqHandler pendSVPtr;
	IrqHandler sysTickPtr;

public:
	static void __attribute__((weak, alias("handler_default"))) nmi();
	static void __attribute__((weak, alias("handler_default"))) hardFault();
	static void __attribute__((weak, alias("handler_default"))) memManage();
	static void __attribute__((weak, alias("handler_default"))) busFault();
	static void __attribute__((weak, alias("handler_default"))) usageFault();
	static void __attribute__((weak, alias("handler_default"))) svc();
	static void __attribute__((weak, alias("handler_default"))) debugMon();
	static void __attribute__((weak, alias("handler_default"))) pendSV();
	static void __attribute__((weak, alias("handler_default"))) sysTick();
	
	constexpr SystemVectors()
		:stackTopPtr(&__system_stack_top)
		,resetPtr(__reset__)
		,nmiPtr(nmi)
		,hardFaultPtr(hardFault)
		,memManagePtr(memManage)
		,busFaultPtr(busFault)
		,usageFaultPtr(usageFault)
		,rsvd1Ptr(NULL)
		,rsvd2Ptr(NULL)
		,rsvd3Ptr(NULL)
		,rsvd4Ptr(NULL)
		,svcPtr(svc)
		,debugMonPtr(debugMon)
		,rsvd5Ptr(NULL)
		,pendSVPtr(pendSV)
		,sysTickPtr(sysTick)
	{ }
};

template<int i> class IRQVector
{
private:
	IRQVector<i - 1> previousItems;
	IrqHandler handlerPtr;
	
public:
	/ pb with weak alias and templates: see GCC ARM Embedded bug #1403649. Removing alias works but wastes ~1k flash... too bad. 
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wattributes"
	static void __attribute__((weak)) handler() { abort(); }
	#pragma GCC diagnostic pop
	
	constexpr IRQVector() :handlerPtr(IRQVector<i>::handler) { }
};

template<> class IRQVector<-1> : private SystemVectors
{
};
*/