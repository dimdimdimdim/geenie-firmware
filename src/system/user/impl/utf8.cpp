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
#include <util/utf8.h>

bool UTF8Decoder::decode(char chr, int &uchar)
{
	unsigned char c = chr;
	if (c < 0x80) {
		uchar = c;
		return true;
	}
	if (c >= 0x80 && c < 0xBF) {
		uchar = (uchar << 6) | (c & 0x3F);
		if (--n == 0)
			return true;
	}
	if (c >= 0xC0 && c < 0xE0) {
		uchar = c & 0x1F;
		n = 1;
	}
	if (c >= 0xE0 && c < 0xF0) {
		uchar = c & 0x0F;
		n = 2;
	}
	if (c >= 0xF0 && c < 0xF8) {
		uchar = c & 0x07;
		n = 3;
	}
	return false;
}

bool UTF8Decoder::decodeBackward(char chr, int &uchar)
{
	unsigned char c = chr;
	if (c < 0x80) {
		uchar = c;
		n = 0;
		return true;
	}
	if (c >= 0x80 && c < 0xBF) {
		uchar = ((uchar >> 6) & 0x3FFFFFF) | ((int)(c & 0x3F) << 26);
		n++;
		return false;
	}
	if (c >= 0xC0 && c < 0xE0) {
		uchar = ((uchar >> 26) & 0x3F) | ((int)(c & 0x1F) << 6);
		if (n >= 1) {
			n = 0;
			return true;
		}
	}
	if (c >= 0xE0 && c < 0xF0) {
		uchar = ((uchar >> 20) & 0xFFF) | ((int)(c & 0x0F) << 12);
		if (n >= 2) {
			n = 0;
			return true;
		}
	}
	if (c >= 0xF0 && c < 0xF8) {
		uchar = ((uchar >> 14) & 0x3FFFF) | ((int)(c & 0x07) << 18);
		if (n >= 3) {
			n = 0;
			return true;
		}
	}
	n = 0;
	return false;
}

UTF8Encoder::UTF8Encoder(int c)
{
	if (c < 0x80) {
		chars[0] = c;
		n = 1;
	}
	else if (c < 0x800) {
		chars[0] = (char)((c >> 6) | 0xC0);
		chars[1] = (char)((c & 0xBF) | 0x80);
		n = 2;
	}
	else if (c < 0x10000) {
		chars[0] = (char)((c >> 12) | 0xE0);
		chars[1] = (char)(((c >> 6) & 0xBF) | 0x80);
		chars[2] = (char)((c & 0xBF) | 0x80);
		n = 3;
	}
	else if (c < 0x200000) {
		chars[0] = (char)((c >> 18) | 0xF0);
		chars[0] = (char)(((c >> 12) & 0xBF) | 0x80);
		chars[0] = (char)(((c >> 6) & 0xBF) | 0x80);
		chars[0] = (char)((c & 0xBF) | 0x80);
		n = 4;
	}
	else
		n = 0;
}
