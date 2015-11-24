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
#include <util/stringbuffer.h>
#include <chars.h>
#include "math.h"

namespace Math
{

double strtod(const char *s00, char **se);
char *dtoa(double d, int mode, int ndigits, int *decpt, int *sign, char **rve);
void freedtoa(char *s);

}

gc<String> doubleToString(double d, int mode)
{
	gc<StringBuffer> strBuf;
	doubleToString(strBuf, d, mode);
	return strBuf.getString();
}

static bool appendExponent(StringBuffer &strBuf, int exp)
{
	if (!strBuf.append('E'))
		return false;
	if (exp < 0) {
		if (!strBuf.append('-'))
			return false;
		exp = -exp;
	}
	char decExp[12], *p = decExp + sizeof(decExp);
	*(--p) = '\0';
	while (exp) {
		*(--p) = exp % 10 + '0';
		exp /= 10;
	}
	if (!strBuf.append(p))
		return false;
	return true;
}

bool doubleToString(StringBuffer &strBuf, double d, int mode)
{
	if (mode == 0) {
		int decpt, sign;
		char *rve;
		char *s = Math::dtoa(d, 1, 12, &decpt, &sign, &rve);
		if (sign)
			if (!strBuf.append('-'))
				return false;
		int n = rve - s;
		if (decpt > 0 && decpt <= 16) {
			if (n <= decpt) {
				strBuf.append(s, n);
				for (int i = 0; i < decpt - n; i++)
					if (!strBuf.append('0'))
						return false;
			}
			else {
				if (!strBuf.append(s, decpt)
				 || !strBuf.append('.')
				 || !strBuf.append(s + decpt, n - decpt))
					return false;
			}
		}
		else if (decpt <= 0 && n - decpt <= 15) {
			if (!strBuf.append("0."))
				return false;
			for (int i = 0; i < -decpt; i++)
				if (!strBuf.append('0'))
					return false;
			if (!strBuf.append(s, n))
				return false;
		}
		else {
			if (!strBuf.append(s[0]))
				return false;
			if (n > 1) {
				if (!strBuf.append('.'))
					return false;
				if (!strBuf.append(s + 1, n - 1))
					return false;
			}
			if (decpt != 1)
				return appendExponent(strBuf, decpt - 1);
		}
		Math::freedtoa(s);
	}
	return true;
}

double stringToDouble(String::AbstractIterator &stri)
{
	gc<StringBuffer> strBuf;
	int uchar = stri.getForward();
	if (uchar == '-') {
		strBuf.append('-');
		uchar = stri.getForward();
	}
	else if (uchar == '+') {
		uchar = stri.getForward();
	}
	while (uchar != -1 && !isSeparator(uchar) && !isOperator(uchar)) {
		if (uchar == 'Y')
			{ strBuf.append("E24"); uchar = stri.getForward(); break; }
		else if (uchar == 'Z')
			{ strBuf.append("E21"); uchar = stri.getForward(); break; }
		else if (uchar == 'E')
			{ strBuf.append("E18"); uchar = stri.getForward(); break; }
		else if (uchar == 'P')
			{ strBuf.append("E15"); uchar = stri.getForward(); break; }
		else if (uchar == 'T')
			{ strBuf.append("E12"); uchar = stri.getForward(); break; }
		else if (uchar == 'G')
			{ strBuf.append("E9"); uchar = stri.getForward(); break; }
		else if (uchar == 'M')
			{ strBuf.append("E6"); uchar = stri.getForward(); break; }
		else if (uchar == 'k')
			{ strBuf.append("E3"); uchar = stri.getForward(); break; }
		else if (uchar == 'm')
			{ strBuf.append("E-3"); uchar = stri.getForward(); break; }
		else if (uchar == 0xB5) //µ
			{ strBuf.append("E-6"); uchar = stri.getForward(); break; }
		else if (uchar == 'n')
			{ strBuf.append("E-9"); uchar = stri.getForward(); break; }
		else if (uchar == 'p')
			{ strBuf.append("E-12"); uchar = stri.getForward(); break; }
		else if (uchar == 'f')
			{ strBuf.append("E-15"); uchar = stri.getForward(); break; }
		else if (uchar == 'a')
			{ strBuf.append("E-18"); uchar = stri.getForward(); break; }
		else if (uchar == 'z')
			{ strBuf.append("E-21"); uchar = stri.getForward(); break; }
		else if (uchar == 'y')
			{ strBuf.append("E-24"); uchar = stri.getForward(); break; }
		else if (uchar >= 128)
			return NAN;
		strBuf.append((char)uchar);
		uchar = stri.getForward();
	}
	if (uchar != -1) {
		if (!isSeparator(uchar) && !isOperator(uchar))
			return NAN;
		stri.getBackward();
	}
	strBuf.append('\0');

	gc<String> s = strBuf.getString();
	char *se;
	double ret = Math::strtod(s.getChars(), &se);
	if (se != s.getChars() + s.getLength() - 1)
		return NAN;
	return ret;
}
