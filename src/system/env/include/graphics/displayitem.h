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

class ApplicationCanvas;

namespace Graphics
{

class Composite;

class DisplayItem : public Object
{
	GC_INHERITS(Object);
	GC_FIELDS(parent);

private:
	DisplayItem *parent;
	int x, y;
	int flags;
	friend class Composite;
	friend class ::ApplicationCanvas;

	static const int VISIBLE = 1;
	static const int AUTO_WIDTH = 2;
	static const int AUTO_HEIGHT = 4;

	bool updateSize(int width, int height);

protected:
	int width, height;

	bool isWidthAuto()						{ return (flags & AUTO_WIDTH) != 0; }
	bool isHeightAuto()						{ return (flags & AUTO_HEIGHT) != 0; }
	void setSize(int width, int height);
	virtual void sizeChanged()				{ }
	virtual void visibleStateChanged()		{ }
	virtual void relayout()					{ }

public:
	DisplayItem(int width = -1, int height = -1, bool visible = true);

	virtual void initLineScanning(int line, int drawOffset, int drawWidth) = 0;
	virtual void processLine(uint8_t *pixelData, int dataOffset) = 0; // always process with XOR
	
	DisplayItem *getParent()						{ return parent; }
	void setVisible(bool visible);
	virtual bool isVisible();
	int getX()										{ return x; }
	int getY()										{ return y; }
	int getWidth()									{ return width; }
	int getHeight()									{ return height; }
	
	virtual void redraw(int fromLine, int height);
	void redraw()									{ redraw(0, height); }
};

}
