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
#include "../calculator.h"
#include "../objects/cobject.h"
#include "../objects/creal.h"
#include "../objects/cinteger.h"
#include "../objects/cstring.h"

namespace Calc
{

void cmdAdd(Calculator *calculator)
{
	gc<CObject *> result;
	gc<const CObject *> x = calculator->getX();
	gc<const CObject *> y = calculator->getY();
	const CReal *rx = x->dynCast<CReal>();
	const CReal *ry = y->dynCast<CReal>();
	const CString *sx, *sy;
	const CInteger *ix, *iy;
	if (rx && ry) {
		result = gcnew(CReal(ry->getValue() + rx->getValue()));
	}
	else if (sx = x->dynCast<CString>(), sy = y->dynCast<CString>(), sx || sy) {
		gc<String> ssy = sy ? sy->getValue() : y->toString();
		gc<String> ssx = sx ? sx->getValue() : x->toString();
		result = gcnew(CString(ssy + ssx));
	}
	else if (ix = x->dynCast<CInteger>(), iy = y->dynCast<CInteger>(), ix || iy) {
		if ((!ix && !rx) || (!iy && !ry))
			gcthrownew(EBadArgumentType);
		uint64_t iix = ix ? ix->getValue() : (uint64_t)(rx->getValue());
		uint64_t iiy = iy ? iy->getValue() : (uint64_t)(ry->getValue());
		result = gcnew(CInteger(iiy + iix));
	}
	else
		gcthrownew(EBadArgumentType);
	calculator->drop2();
	calculator->push(result);
}

void cmdSub(Calculator *calculator)
{
	gc<CObject *> result;
	gc<const CObject *> x = calculator->getX();
	gc<const CObject *> y = calculator->getY();
	const CReal *rx = x->dynCast<CReal>();
	const CReal *ry = y->dynCast<CReal>();
	const CInteger *ix, *iy;
	if (rx && ry) {
		result = gcnew(CReal(ry->getValue() - rx->getValue()));
	}
	else if (ix = x->dynCast<CInteger>(), iy = y->dynCast<CInteger>(), ix || iy) {
		if ((!ix && !rx) || (!iy && !ry))
			gcthrownew(EBadArgumentType);
		uint64_t iix = ix ? ix->getValue() : (uint64_t)(rx->getValue());
		uint64_t iiy = iy ? iy->getValue() : (uint64_t)(ry->getValue());
		result = gcnew(CInteger(iiy - iix));
	}
	else
		gcthrownew(EBadArgumentType);
	calculator->drop2();
	calculator->push(result);
}

void cmdMul(Calculator *calculator)
{
	gc<CObject *> result;
	gc<const CObject *> x = calculator->getX();
	gc<const CObject *> y = calculator->getY();
	const CReal *rx = x->dynCast<CReal>();
	const CReal *ry = y->dynCast<CReal>();
	const CInteger *ix, *iy;
	if (rx && ry) {
		result = gcnew(CReal(ry->getValue() * rx->getValue()));
	}
	else if (ix = x->dynCast<CInteger>(), iy = y->dynCast<CInteger>(), ix || iy) {
		if ((!ix && !rx) || (!iy && !ry))
			gcthrownew(EBadArgumentType);
		uint64_t iix = ix ? ix->getValue() : (uint64_t)(rx->getValue());
		uint64_t iiy = iy ? iy->getValue() : (uint64_t)(ry->getValue());
		result = gcnew(CInteger(iiy * iix));
	}
	else
		gcthrownew(EBadArgumentType);
	calculator->drop2();
	calculator->push(result);
}

void cmdDiv(Calculator *calculator)
{
	gc<CObject *> result;
	gc<const CObject *> x = calculator->getX();
	gc<const CObject *> y = calculator->getY();
	const CReal *rx = x->dynCast<CReal>();
	const CReal *ry = y->dynCast<CReal>();
	const CInteger *ix, *iy;
	if (rx && ry) {
		result = gcnew(CReal(ry->getValue() / rx->getValue()));
	}
	else if (ix = x->dynCast<CInteger>(), iy = y->dynCast<CInteger>(), ix || iy) {
		if ((!ix && !rx) || (!iy && !ry))
			gcthrownew(EBadArgumentType);
		uint64_t iix = ix ? ix->getValue() : (uint64_t)(rx->getValue());
		uint64_t iiy = iy ? iy->getValue() : (uint64_t)(ry->getValue());
		result = gcnew(CInteger(iiy / iix));
	}
	else
		gcthrownew(EBadArgumentType);
	calculator->drop2();
	calculator->push(result);
}

void cmdMod(Calculator *calculator)
{
	gc<CObject *> result;
	gc<const CObject *> x = calculator->getX();
	gc<const CObject *> y = calculator->getY();
	const CReal *rx = x->dynCast<CReal>();
	const CReal *ry = y->dynCast<CReal>();
	const CInteger *ix, *iy;
	if (rx && ry) {
		result = gcnew(CReal(fmod(ry->getValue(), rx->getValue())));
	}
	else if (ix = x->dynCast<CInteger>(), iy = y->dynCast<CInteger>(), ix || iy) {
		if ((!ix && !rx) || (!iy && !ry))
			gcthrownew(EBadArgumentType);
		uint64_t iix = ix ? ix->getValue() : (uint64_t)(rx->getValue());
		uint64_t iiy = iy ? iy->getValue() : (uint64_t)(ry->getValue());
		result = gcnew(CInteger(iiy % iix));
	}
	else
		gcthrownew(EBadArgumentType);
	calculator->drop2();
	calculator->push(result);
}

void cmdNeg(Calculator *calculator)
{
	gc<CObject *> result;
	gc<const CObject *> x = calculator->getX();
	const CReal *rx = x->dynCast<CReal>();
	const CInteger *ix;
	if (rx) {
		result = gcnew(CReal(-rx->getValue()));
	}
	else if ((ix = x->dynCast<CInteger>())) {
		result = gcnew(CInteger((uint64_t)-(int64_t)ix->getValue()));
	}
	else
		gcthrownew(EBadArgumentType);
	calculator->drop();
	calculator->push(result);
}

void cmdNot(Calculator *calculator)
{
	gc<CObject *> result;
	gc<const CObject *> x = calculator->getX();
	const CReal *rx = x->dynCast<CReal>();
	const CInteger *ix;
	if (rx) {
		result = gcnew(CReal(rx->getValue() == 0 ? 1 : 0));
	}
	else if ((ix = x->dynCast<CInteger>())) {
		result = gcnew(CInteger(~ix->getValue()));
	}
	else
		gcthrownew(EBadArgumentType);
	calculator->drop();
	calculator->push(result);
}

void cmdAnd(Calculator *calculator)
{
	gc<CObject *> result;
	gc<const CObject *> x = calculator->getX();
	gc<const CObject *> y = calculator->getY();
	const CReal *rx = x->dynCast<CReal>();
	const CReal *ry = y->dynCast<CReal>();
	const CInteger *ix, *iy;
	if (rx && ry) {
		result = gcnew(CReal(ry->getValue() && rx->getValue()));
	}
	else if ((ix = x->dynCast<CInteger>()) && (iy = y->dynCast<CInteger>())) {
		result = gcnew(CInteger(iy->getValue() & ix->getValue()));
	}
	else
		gcthrownew(EBadArgumentType);
	calculator->drop2();
	calculator->push(result);
}

void cmdOr(Calculator *calculator)
{
	gc<CObject *> result;
	gc<const CObject *> x = calculator->getX();
	gc<const CObject *> y = calculator->getY();
	const CReal *rx = x->dynCast<CReal>();
	const CReal *ry = y->dynCast<CReal>();
	const CInteger *ix, *iy;
	if (rx && ry) {
		result = gcnew(CReal(ry->getValue() || rx->getValue()));
	}
	else if ((ix = x->dynCast<CInteger>()) && (iy = y->dynCast<CInteger>())) {
		result = gcnew(CInteger(iy->getValue() | ix->getValue()));
	}
	else
		gcthrownew(EBadArgumentType);
	calculator->drop2();
	calculator->push(result);
}

void cmdXor(Calculator *calculator)
{
	gc<CObject *> result;
	gc<const CObject *> x = calculator->getX();
	gc<const CObject *> y = calculator->getY();
	const CReal *rx = x->dynCast<CReal>();
	const CReal *ry = y->dynCast<CReal>();
	const CInteger *ix, *iy;
	if (rx && ry) {
		result = gcnew(CReal((ry->getValue() != 0) != (rx->getValue() != 0)));
	}
	else if ((ix = x->dynCast<CInteger>()) && (iy = y->dynCast<CInteger>())) {
		result = gcnew(CInteger(iy->getValue() ^ ix->getValue()));
	}
	else
		gcthrownew(EBadArgumentType);
	calculator->drop2();
	calculator->push(result);
}

void cmdIsEqual(Calculator *calculator)
{
	gc<CObject *> result;
	gc<const CObject *> x = calculator->getX();
	gc<const CObject *> y = calculator->getY();
	const CReal *rx = x->dynCast<CReal>();
	const CReal *ry = y->dynCast<CReal>();
	const CInteger *ix, *iy;
	const CString *sx, *sy;
	if (rx && ry) {
		result = gcnew(CReal(ry->getValue() == rx->getValue()));
	}
	else if ((ix = x->dynCast<CInteger>()) && (iy = y->dynCast<CInteger>())) {
		result = gcnew(CReal(iy->getValue() == ix->getValue()));
	}
	else if ((sx = x->dynCast<CString>()) && (sy = y->dynCast<CString>())) {
		result = gcnew(CReal(sy->getValue() == sx->getValue()));
	}
	else
		result = gcnew(CReal(0));
	calculator->drop2();
	calculator->push(result);
}

void cmdIsNotEqual(Calculator *calculator)
{
	gc<CObject *> result;
	gc<const CObject *> x = calculator->getX();
	gc<const CObject *> y = calculator->getY();
	const CReal *rx = x->dynCast<CReal>();
	const CReal *ry = y->dynCast<CReal>();
	const CInteger *ix, *iy;
	const CString *sx, *sy;
	if (rx && ry) {
		result = gcnew(CReal(ry->getValue() != rx->getValue()));
	}
	else if ((ix = x->dynCast<CInteger>()) && (iy = y->dynCast<CInteger>())) {
		result = gcnew(CReal(iy->getValue() != ix->getValue()));
	}
	else if ((sx = x->dynCast<CString>()) && (sy = y->dynCast<CString>())) {
		result = gcnew(CReal(sy->getValue() != sx->getValue()));
	}
	else
		result = gcnew(CReal(1));
	calculator->drop2();
	calculator->push(result);
}

void cmdIsLower(Calculator *calculator)
{
	gc<CObject *> result;
	gc<const CObject *> x = calculator->getX();
	gc<const CObject *> y = calculator->getY();
	const CReal *rx = x->dynCast<CReal>();
	const CReal *ry = y->dynCast<CReal>();
	const CInteger *ix, *iy;
	const CString *sx, *sy;
	if (rx && ry) {
		result = gcnew(CReal(ry->getValue() < rx->getValue()));
	}
	else if ((ix = x->dynCast<CInteger>()) && (iy = y->dynCast<CInteger>())) {
		result = gcnew(CReal(iy->getValue() < ix->getValue()));
	}
	else if ((sx = x->dynCast<CString>()) && (sy = y->dynCast<CString>())) {
		result = gcnew(CReal(sy->getValue() < sx->getValue()));
	}
	else
		gcthrownew(EBadArgumentType);
	calculator->drop2();
	calculator->push(result);
}

void cmdIsGreater(Calculator *calculator)
{
	gc<CObject *> result;
	gc<const CObject *> x = calculator->getX();
	gc<const CObject *> y = calculator->getY();
	const CReal *rx = x->dynCast<CReal>();
	const CReal *ry = y->dynCast<CReal>();
	const CInteger *ix, *iy;
	const CString *sx, *sy;
	if (rx && ry) {
		result = gcnew(CReal(ry->getValue() > rx->getValue()));
	}
	else if ((ix = x->dynCast<CInteger>()) && (iy = y->dynCast<CInteger>())) {
		result = gcnew(CReal(iy->getValue() > ix->getValue()));
	}
	else if ((sx = x->dynCast<CString>()) && (sy = y->dynCast<CString>())) {
		result = gcnew(CReal(sy->getValue() > sx->getValue()));
	}
	else
		gcthrownew(EBadArgumentType);
	calculator->drop2();
	calculator->push(result);
}

void cmdIsLowerEqual(Calculator *calculator)
{
	gc<CObject *> result;
	gc<const CObject *> x = calculator->getX();
	gc<const CObject *> y = calculator->getY();
	const CReal *rx = x->dynCast<CReal>();
	const CReal *ry = y->dynCast<CReal>();
	const CInteger *ix, *iy;
	const CString *sx, *sy;
	if (rx && ry) {
		result = gcnew(CReal(ry->getValue() <= rx->getValue()));
	}
	else if ((ix = x->dynCast<CInteger>()) && (iy = y->dynCast<CInteger>())) {
		result = gcnew(CReal(iy->getValue() <= ix->getValue()));
	}
	else if ((sx = x->dynCast<CString>()) && (sy = y->dynCast<CString>())) {
		result = gcnew(CReal(sy->getValue() <= sx->getValue()));
	}
	else
		gcthrownew(EBadArgumentType);
	calculator->drop2();
	calculator->push(result);
}

void cmdIsGreaterEqual(Calculator *calculator)
{
	gc<CObject *> result;
	gc<const CObject *> x = calculator->getX();
	gc<const CObject *> y = calculator->getY();
	const CReal *rx = x->dynCast<CReal>();
	const CReal *ry = y->dynCast<CReal>();
	const CInteger *ix, *iy;
	const CString *sx, *sy;
	if (rx && ry) {
		result = gcnew(CReal(ry->getValue() >= rx->getValue()));
	}
	else if ((ix = x->dynCast<CInteger>()) && (iy = y->dynCast<CInteger>())) {
		result = gcnew(CReal(iy->getValue() >= ix->getValue()));
	}
	else if ((sx = x->dynCast<CString>()) && (sy = y->dynCast<CString>())) {
		result = gcnew(CReal(sy->getValue() >= sx->getValue()));
	}
	else
		gcthrownew(EBadArgumentType);
	calculator->drop2();
	calculator->push(result);
}

}
