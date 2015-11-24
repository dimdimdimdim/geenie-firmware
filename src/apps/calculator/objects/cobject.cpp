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
#include <graphics/text.h>
#include "../calculator.h"
#include "cobject.h"

namespace Calc
{

gc<String> CObject::toString(LocalVariables *locals) const
{
	gc<StringBuffer> strBuf;
	buildString(strBuf, locals);
	return strBuf.getString();
}

bool CObject::buildString(StringBuffer &str, LocalVariables *locals) const
{
	return str.append(toString(locals));
}

void CObject::enter(Calculator *calculator, LocalVariables *locals) const
{
	calculator->push(this);
}

void CObject::execute(Calculator *calculator, LocalVariables *locals) const
{
	calculator->push(this);
}

gc<List<const CObject *> *> CObject::parseSequence(String::AbstractIterator &stri, int endingChar)
{
	int uchar = stri.getForward();
	while (isWhiteSpace(uchar))
		uchar = stri.getForward();
	if (uchar == -1)
		return NULL;
	gc<List<const CObject *> *> list = gcnew(List<const CObject *>);
	while (uchar != -1 && uchar != endingChar) {
		list->append(parseSingleObject(uchar, stri));
		uchar = stri.getForward();
		while (isWhiteSpace(uchar))
			uchar = stri.getForward();
	}
	if (endingChar != -1 && uchar != endingChar)
		gcthrownew(EMissingTerminatorCharacter);
	return list;
}

using namespace Graphics;

DisplayItem *CObject::getDisplayItem(const Graphics::Font &font, LocalVariables *locals) const
{
	return gcnew(Text(0, 0, font, Text::ALIGN_RIGHT | Text::ELLIPSIS, toString(locals)));
}

}