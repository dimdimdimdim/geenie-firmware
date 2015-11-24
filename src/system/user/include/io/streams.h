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

namespace IO
{

class InputStream : public virtual Interface
{
	GC_INHERITS(Interface);

public:
	InputStream()				{ }
	
	/* Just like POSIX. Returns 0 if end of file is reached.
       May return less bytes than requested. */
	virtual size_t read(void *data, size_t len) = 0;

	virtual int getChar()
	{
		char c;
		if (read(&c, 1) == 0)
			return -1;
		return c;
	}

	void readFully(void *data, size_t len)
	{
		while (len) {
			size_t n = read(data, len);
			if (n == 0)
				gcthrownew(EEndOfFile);
			data = (char *)data + n;
			len -= n;
		}
	}

};

class OutputStream : public virtual Interface
{
	GC_INHERITS(Interface);

public:
	OutputStream()				{ }

	/* Just like POSIX.
       Write may not be complete (may return less bytes than requested). */
	virtual size_t write(const void *data, size_t len) = 0;

	virtual void putChar(int c)
	{
		char cc = (char)c;
		write(&cc, 1);
	}

	void writeFully(const void *data, size_t len)
	{
		while (len) {
			size_t n = write(data, len);
			assert(n != 0);
			data = (char *)data + n;
			len -= n;
		}
	}
	
	virtual void flush() { }
};

class BufferedInputStream : public Object, public virtual InputStream
{
	GC_INHERITS(Object, InputStream);
	GC_FIELDS(stream, buf);

private:
	InputStream *stream;
	char *buf;
	size_t index, filled, bufLen;
public:

	BufferedInputStream(InputStream *stream, size_t bufLen = 512)
	{
		this->stream = stream;
		this->bufLen = bufLen;
		this->index = 0;
		this->filled = 0;
		buf = gcnew(char[bufLen]);
	}

	virtual size_t read(void *data, size_t len);

};

} // namespace IO