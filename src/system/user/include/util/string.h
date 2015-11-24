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

/* strings are immutable */
class String
{
private:
	size_t length;
	const char *chars;

	String(const char *chars, size_t length)
	{
		this->chars = chars;
		this->length = length;
	}

	static constexpr int rotHash(int n)
	{
		return ((n << 1) & 31) | ((n >> 4) & 1);
	}
	
	static constexpr int getCharsHash(const char *s, size_t l)
	{
		return l == 0 ? 0 : (rotHash(getCharsHash(s + 1, l - 1)) ^ (*s & 31));
	}

public:
	void gcMarkFields() const
	{
		if (Kernel::isInRam(chars))
			Kernel::Allocator::mark(chars);
	}

public:

	constexpr String()
		:length(0), chars(NULL)
	{
	}

	// Constructs a string with the given string literal
	template<size_t n> constexpr String(const char (&chars)[n])
		: length(n - 1), chars(n == 1 ? NULL : chars)
	{
	}

	// Make a new string using the given chars buffer.
	// The string takes over the buffer.
	// To guarantee immutability, the buffer must not be modified after this call.
	static gc<String> make(const char *chars, size_t length)
	{
		return String(chars, length);
	}

	// Builds a string with the given chars and length.
	// A new copy of the character array is made.
	static gc<String> build(const char *chars, size_t length);

	constexpr size_t getLength() const
	{
		return length;
	}

	constexpr const char *getChars() const
	{
		return chars;
	}

	constexpr char operator[](size_t i) const
	{
		return chars[i];
	}

	constexpr int getHash() const
	{
		// hash result is between 0 and 31
		return getCharsHash(getChars(), getLength());
	}

	const String &operator=(const String &s)
	{
		chars = s.chars;
		length = s.length;
		return *this;
	}

	gc<String> operator+(const String &s) const;
	template<size_t n> gc<String> operator+(const char(&s)[n]) const { return operator+(String(s)); }

	gc<String> part(size_t from, size_t length) const;
	gc<String> part(size_t from) const								{ return part(from, getLength() - from); }

	int compare(const String &s) const;
	bool operator==(const String &s) const							{ return compare(s) == 0; }
	bool operator!=(const String &s) const							{ return compare(s) != 0; }
	bool operator<=(const String &s) const							{ return compare(s) <= 0; }
	bool operator>=(const String &s) const							{ return compare(s) >= 0; }
	bool operator<(const String &s) const							{ return compare(s) < 0; }
	bool operator>(const String &s) const							{ return compare(s) > 0; }
/*	template<size_t n> bool operator==(const char (&s)[n]) const	{ return compare(s) == 0; }
	template<size_t n> bool operator!=(const char (&s)[n]) const	{ return compare(s) != 0; }
	template<size_t n> bool operator<=(const char (&s)[n]) const	{ return compare(s) <= 0; }
	template<size_t n> bool operator>=(const char (&s)[n]) const	{ return compare(s) >= 0; }
	template<size_t n> bool operator<(const char (&s)[n]) const		{ return compare(s) < 0; }
	template<size_t n> bool operator>(const char (&s)[n]) const		{ return compare(s) > 0; }*/

	static const size_t notFound = (size_t)-1;

	size_t findFirst(char c) const
	{
		for (size_t i = 0; i < getLength(); i++)
			if (chars[i] == c)
				return i;
		return notFound;
	}

	size_t findLast(char c) const
	{
		for (size_t i = getLength() - 1; i != (size_t)-1; i++)
			if (chars[i] == c)
				return i;
		return notFound;
	}

	operator bool() const
	{
		return chars != NULL;
	}

	bool operator!() const
	{
		return chars == NULL;
	}

	class AbstractIterator
	{
	public:
		virtual int getForward() = 0;
		virtual int getBackward() = 0;
		virtual int getPosition() const = 0;
		virtual size_t getObjectSize() const = 0;
		virtual AbstractIterator *copyTo(void *ptr) const { memcpy(ptr, this, getObjectSize()); return (AbstractIterator *)this; }
	};

	class Iterator : public AbstractIterator
	{
	private:
		const String *string;
		size_t pos;
	public:
		Iterator()									{ string = NULL;  pos = 0; }
		Iterator(const Iterator &i)					{ string = i.string; pos = i.pos; }
		Iterator(const String &s)					{ string = &s; pos = 0; }
		Iterator(const String &s, size_t pos)		{ string = &s; this->pos = pos; }
		Iterator &operator=(const Iterator &i)		{ string = i.string; pos = i.pos; return *this; }

		bool operator==(const Iterator &i) const	{ return string == i.string && pos == i.pos; }
		bool operator!=(const Iterator &i) const	{ return string != i.string || pos != i.pos; }

		virtual int getForward();
		virtual int getBackward();
		virtual int getPosition() const				{ return pos; }
		virtual size_t getObjectSize() const		{ return sizeof(Iterator); };

	};

	Iterator start() const
	{
		return Iterator(*this);
	}

	Iterator end() const
	{
		return Iterator(*this, getLength());
	}
};

template<> class GCMarker<String>				{ public: static void mark(const String &x)			{ x.gcMarkFields(); } };
template<> class GCMarker<const String>			{ public: static void mark(const String &x)			{ x.gcMarkFields(); } };
