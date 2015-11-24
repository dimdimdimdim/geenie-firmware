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
#include <io/streams.h>

using namespace IO;

size_t BufferedInputStream::read(void *data, size_t len)
{
	if (index == filled) {
		index = 0;
		filled = stream->read(buf, bufLen);
		if (filled == 0)
			return 0;
	}
	if (len > filled - index)
		len = filled - index;
	memcpy(data, buf + index, len);
	index += len;
	return len;
}
