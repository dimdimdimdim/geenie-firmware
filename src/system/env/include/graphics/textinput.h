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

#include <util/gapstringbuffer.h>
#include <graphics/displayitem.h>
#include <graphics/font.h>
#include <graphics/text.h>
#include <keyboard.h>

namespace Graphics
{

class Font;

class TextInput : public DisplayItem
{
	GC_INHERITS(DisplayItem);
	GC_FIELDS(string, cursorBlinkTimer, lastMultiTap, cursorContext);

protected:
	GapStringBuffer *string;
	int stringWidth;
	int offsetX;
	int cursorX;
	const Font *font;
	Glyph cursorGlyph;
	int flags;
	GapStringBuffer::Iterator strDrawIndex;
	int drawWidth;
	int drawOffsetX;
	int drawCursorX;
	int scanLine;
	class Timer *cursorBlinkTimer;
	bool cursorState;
	String lastMultiTap;
	size_t multiTapIndex;
	TextContext *cursorContext;

	void onCursorBlink();

public:
	TextInput(int width, int height, bool visible, const Font &font, int flags = 0, const String &string = "", TextContext *cursorContext = NULL);
	const Font &getFont() { return *font; }

	virtual void initLineScanning(int scanLine, int drawOffsetX, int drawWidth);
	virtual void processLine(uint8_t *pixelData, int dataOffset);

	virtual bool onKeyPress(const KeyAction *action, int mode);

	void clearText();
	void insertText(const String &string);
	int moveForward();
	int moveBackward();
	void update(bool multiTap = false);

	GapStringBuffer::Iterator getTextIterator()		{ return string->start(); }
	GapStringBuffer::Iterator getCursorIterator()	{ return string->cursor(); }
};

}
