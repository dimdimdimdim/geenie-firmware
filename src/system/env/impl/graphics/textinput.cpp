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
#include <system.h>
#include <graphics/textinput.h>

#define CURSOR_MARGIN		24
#define CURSOR_BLINKDELAY	500
#define MULTITAP_DELAY		2000

namespace Graphics
{

TextInput::TextInput(int width, int height, bool visible, const Font &font, int flags, const String &string, TextContext *cursorContext)
:DisplayItem(width, height, visible)
{
	this->font = &font;
	this->cursorGlyph = font.getGlyph(Font::GLYPH_CURSOR);
	this->flags = flags;
	this->string = gcnew(GapStringBuffer(string));
	this->cursorContext = cursorContext;
	GapStringBuffer::Iterator i = this->string->start();
	int c;
	stringWidth = 0;
	while ((c = i.getForward()) != -1) {
		stringWidth += this->font->getGlyph(Font::getGlyphIndex(c)).getWidth();
	}
	cursorX = stringWidth;
	offsetX = stringWidth - width;
	if (offsetX < 0)
		offsetX = 0;
	cursorBlinkTimer = gcnew(Timer(this, (Timer::CallbackMethod)&TextInput::onCursorBlink));
	cursorBlinkTimer->set(CURSOR_BLINKDELAY, true);
	cursorState = true;
	lastMultiTap = "";
}

void TextInput::initLineScanning(int scanLine, int drawOffsetX, int drawWidth)
{
	int c;
	// start from cursor
	drawCursorX = cursorX - cursorGlyph.getWidth();
	if (drawCursorX < 0)
		drawCursorX = 0;
	drawCursorX -= offsetX + drawOffsetX;
	drawOffsetX += offsetX - cursorX;
	strDrawIndex = string->cursor();
	// adjust
	if (drawOffsetX <= 0) {
		while (drawOffsetX < 0 && (c = strDrawIndex.getBackward()) != -1) {
			drawOffsetX += font->getGlyph(Font::getGlyphIndex(c)).getWidth();
		}
	}
	else {
		while (drawOffsetX > 0 && (c = strDrawIndex.getForward()) != -1) {
			int charWidth = font->getGlyph(Font::getGlyphIndex(c)).getWidth();
			if (drawOffsetX >= charWidth) {
				drawOffsetX -= charWidth;
			}
			else {
				strDrawIndex.getBackward();
				break;
			}
		}
	}
	this->drawWidth = drawWidth;
	this->drawOffsetX = drawOffsetX;
	this->scanLine = scanLine;
	assert(drawOffsetX >= 0);
}

void TextInput::processLine(uint8_t *pixelData, int dataOffset)
{
	if (scanLine >= font->getLineHeight())
		return;
	if (cursorState && drawCursorX > -cursorGlyph.getWidth() && drawCursorX < drawWidth) {
		if (drawCursorX < 0)
			cursorGlyph.drawLine(pixelData, dataOffset, scanLine, -drawCursorX, drawWidth);
		else
			cursorGlyph.drawLine(pixelData, dataOffset + drawCursorX, scanLine, 0, drawWidth - drawCursorX);
	}
	int w = drawWidth;
	GapStringBuffer::Iterator i = strDrawIndex;
	int ofsX = drawOffsetX;
	while (w) {
		int c = i.getForward();
		if (c == -1)
			break;
		int gw = font->getGlyph(Font::getGlyphIndex(c)).drawLine(pixelData, dataOffset, scanLine, ofsX, w);
		w -= gw;
		dataOffset += gw;
		ofsX = 0;
	}
	scanLine++;
}

void TextInput::onCursorBlink()
{
	cursorState = !cursorState;
	if (lastMultiTap) {
		lastMultiTap = "";
		cursorBlinkTimer->set(CURSOR_BLINKDELAY, true);
	}
	redraw();
}

bool TextInput::onKeyPress(const KeyAction *action, int mode)
{
	bool processed = false;
	if (action->isInput()) {
		insertText(action->getString());
		processed = true;
	}
	else if (action->isInputChar()) {
		string->insert(action->getChar());
		if (cursorContext)
			cursorContext->updateForward(action->getChar(), string->cursor());
		int w = font->getGlyph(Font::getGlyphIndex(action->getChar())).getWidth();
		stringWidth += w;
		cursorX += w;
		processed = true;
	}
	else if (action->isBraces()) {
		insertText(action->getString());
		int uc = string->moveCursorLeft();
		if (uc != -1) {
			if (cursorContext)
				cursorContext->updateBackward(uc, string->cursor());
			int w = font->getGlyph(Font::getGlyphIndex(uc)).getWidth();
			cursorX -= w;
		}
		if (action->getString().getLength() > 2) {
			uc = string->moveCursorLeft();
			if (uc != -1) {
				if (cursorContext)
					cursorContext->updateBackward(uc, string->cursor());
				int w = font->getGlyph(Font::getGlyphIndex(uc)).getWidth();
				cursorX -= w;
			}
		}
		processed = true;
	}
	else if (action->isMultiTap()) {
		if (lastMultiTap != action->getString()) {
			lastMultiTap = action->getString();
			multiTapIndex = 0;
		}
		else {
			int uc = string->eraseToLeft();
			if (uc != -1) {
				if (cursorContext)
					cursorContext->updateBackward(uc, string->cursor());
				int w = font->getGlyph(Font::getGlyphIndex(uc)).getWidth();
				stringWidth -= w;
				cursorX -= w;
			}
		}
		UTF8Decoder utf8;
		int uc;
		for (;;) {
			if (multiTapIndex == lastMultiTap.getLength())
				multiTapIndex = 0;
			char c = lastMultiTap[multiTapIndex++];
			if (utf8.decode(c, uc))
				break;
		}
		string->insert(uc);
		if (cursorContext)
			cursorContext->updateForward(uc, string->cursor());
		int w = font->getGlyph(Font::getGlyphIndex(uc)).getWidth();
		stringWidth += w;
		cursorX += w;
		processed = true;
	}
	else if (action->isSpecial()) {
		if (action->getId() == KeyAction::BS) {
			int uc = string->eraseToLeft();
			if (uc != -1) {
				if (cursorContext)
					cursorContext->updateBackward(uc, string->cursor());
				int w = font->getGlyph(Font::getGlyphIndex(uc)).getWidth();
				stringWidth -= w;
				cursorX -= w;
				processed = true;
			}
		}
		else if (action->getId() == KeyAction::DEL) {
			int uc = string->eraseToRight();
			if (uc != -1) {
				int w = font->getGlyph(Font::getGlyphIndex(uc)).getWidth();
				stringWidth -= w;
				processed = true;
			}
		}
		else if (action->getId() == KeyAction::LEFT) {
			if (moveBackward() != -1)
				processed = true;
		}
		else if (action->getId() == KeyAction::RIGHT) {
			if (moveForward() != -1)
				processed = true;
		}
	}
	if (!action->isMultiTap())
		lastMultiTap = "";
	if (processed) {
		update(action->isMultiTap());
		return true;
	}
	return false;
}

void TextInput::update(bool multiTap)
{
	if (offsetX < cursorX - getWidth() + CURSOR_MARGIN)
		offsetX = cursorX - getWidth() + CURSOR_MARGIN;
	if (offsetX > cursorX - CURSOR_MARGIN)
		offsetX = cursorX - CURSOR_MARGIN;
	if (offsetX > stringWidth - getWidth())
		offsetX = stringWidth - getWidth();
	if (offsetX < 0)
		offsetX = 0;
	cursorState = true;
	if (!multiTap)
		cursorBlinkTimer->set(CURSOR_BLINKDELAY, true);
	else
		cursorBlinkTimer->set(MULTITAP_DELAY, false);
	redraw();
}

void TextInput::clearText()
{
	string->clear();
	stringWidth = 0;
	cursorX = 0;
	if (cursorContext)
		cursorContext->reset();
}

void TextInput::insertText(const String &s)
{
	GapStringBuffer::Iterator iter = string->cursor();
	string->insert(s);
	int w = 0;
	const char *pc = s.getChars();
	size_t l = s.getLength();
	int uc;
	UTF8Decoder decoder;
	while (l-- != 0) {
		if (decoder.decode(*(pc++), uc)) {
			if (cursorContext) {
				cursorContext->updateForward(uc, iter);
				iter.getForward();
			}
			w += font->getGlyph(Font::getGlyphIndex(uc)).getWidth();
		}
	}
	stringWidth += w;
	cursorX += w;
}

int TextInput::moveForward()
{
	int uc = string->moveCursorRight();
	if (uc != -1) {
		if (cursorContext)
			cursorContext->updateForward(uc, string->cursor());
		int w = font->getGlyph(Font::getGlyphIndex(uc)).getWidth();
		cursorX += w;
	}
	return uc;
}

int TextInput::moveBackward()
{
	int uc = string->moveCursorLeft();
	if (uc != -1) {
		if (cursorContext)
			cursorContext->updateBackward(uc, string->cursor());
		int w = font->getGlyph(Font::getGlyphIndex(uc)).getWidth();
		cursorX -= w;
	}
	return uc;
}

}
