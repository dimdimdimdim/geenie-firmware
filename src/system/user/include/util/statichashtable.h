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
/*
template<class K, class V> class StaticHashTable
{
public:
	struct Entry
	{
		K key;
		V value;
		const Entry *next;
		constexpr Entry(const K &key, const V &value, const Entry *next) :key(key), value(value), next(next) { }
		constexpr Entry(const K &key, const V &value) :key(key), value(value), next(NULL) { }
		constexpr Entry(const Entry &entry) :key(entry.key), value(entry.value), next(entry.next) { }
	};

private:

	static constexpr size_t rotHash(size_t n)							{ return (n << 1) & 31 | (n >> 4) & 1; }
	static constexpr size_t getStringHash(const char *s, size_t l)		{ return l == 0 ? 0 : (rotHash(getStringHash(s + 1, l - 1)) ^ (*s & 31)); }
	static constexpr size_t getHash(const String &str)					{ return getStringHash(str.getChars(), str.getLength()); }

	static constexpr Entry makeEntry(const K &key, const V &value, const Entry *next)
	{
		return Entry(key, value, next);
	}

	struct Entries
	{
		const Entry e[32];
		constexpr Entries(size_t index, const K &key, const V &value, const Entries &entries) :e{
			Entry(key, value, &(entries.e[ 0]))
		//	(index ==  0 ? &Entry(key, value, entries.e[ 0]) : entries.e[ 0]), 
		//	(index ==  1 ? &Entry(key, value, entries.e[ 1]) : entries.e[ 1]), 
		//	(index ==  2 ? &Entry(key, value, entries.e[ 2]) : entries.e[ 2]), 
		//	(index ==  3 ? &Entry(key, value, entries.e[ 3]) : entries.e[ 3]), 
		//	(index ==  4 ? &Entry(key, value, entries.e[ 4]) : entries.e[ 4]), 
		//	(index ==  5 ? &Entry(key, value, entries.e[ 5]) : entries.e[ 5]), 
		//	(index ==  6 ? &Entry(key, value, entries.e[ 6]) : entries.e[ 6]), 
		//	(index ==  7 ? &Entry(key, value, entries.e[ 7]) : entries.e[ 7]), 
		//	(index ==  8 ? &Entry(key, value, entries.e[ 8]) : entries.e[ 8]), 
		//	(index ==  9 ? &Entry(key, value, entries.e[ 9]) : entries.e[ 9]), 
		//	(index == 10 ? &Entry(key, value, entries.e[10]) : entries.e[10]), 
		//	(index == 11 ? &Entry(key, value, entries.e[11]) : entries.e[11]), 
		//	(index == 12 ? &Entry(key, value, entries.e[12]) : entries.e[12]), 
		//	(index == 13 ? &Entry(key, value, entries.e[13]) : entries.e[13]), 
		//	(index == 14 ? &Entry(key, value, entries.e[14]) : entries.e[14]), 
		//	(index == 15 ? &Entry(key, value, entries.e[15]) : entries.e[15]), 
		//	(index == 16 ? &Entry(key, value, entries.e[16]) : entries.e[16]), 
		//	(index == 17 ? &Entry(key, value, entries.e[17]) : entries.e[17]), 
		//	(index == 18 ? &Entry(key, value, entries.e[18]) : entries.e[18]), 
		//	(index == 19 ? &Entry(key, value, entries.e[19]) : entries.e[19]), 
		//	(index == 20 ? &Entry(key, value, entries.e[20]) : entries.e[20]), 
		//	(index == 21 ? &Entry(key, value, entries.e[21]) : entries.e[21]), 
		//	(index == 22 ? &Entry(key, value, entries.e[22]) : entries.e[22]), 
		//	(index == 23 ? &Entry(key, value, entries.e[23]) : entries.e[23]), 
		//	(index == 24 ? &Entry(key, value, entries.e[24]) : entries.e[24]), 
		//	(index == 25 ? &Entry(key, value, entries.e[25]) : entries.e[25]), 
		//	(index == 26 ? &Entry(key, value, entries.e[26]) : entries.e[26]), 
		//	(index == 27 ? &Entry(key, value, entries.e[27]) : entries.e[27]), 
		//	(index == 28 ? &Entry(key, value, entries.e[28]) : entries.e[28]), 
		//	(index == 29 ? &Entry(key, value, entries.e[29]) : entries.e[29]), 
		//	(index == 30 ? &Entry(key, value, entries.e[30]) : entries.e[30]), 
		//	(index == 31 ? &Entry(key, value, entries.e[31]) : entries.e[31]), 
		} { }
//		constexpr Entries(const Entries &entries) :e(entries.e) { }
		constexpr Entries() :e{} { }
	} entries;

	static constexpr Entries initEntries(const Entry *items, size_t size)
	{
		return size == 0 ? Entries() :
			Entries(getHash(items[0].key), items[0].key, items[0].value, initEntries(items + 1, size - 1));
	}

public:
	constexpr StaticHashTable(std::initializer_list<const Entry> items) :entries(initEntries(items.begin(), items.size())) { }
	
	V operator[](const K &key) const
	{
		const Entry *e = entries.e[getHash(key)];
		while (e && e->key != key)
			e = e->next;
		if (!e)
			return V();
		return e->value;
	}
};
*/

/*
template<class V, const char *_key, V _value> struct StaticHashEntry
{
	static constexpr int keyHash = getHash(_key);
	static constexpr String key = _key;
	static constexpr V value = _value;

	typedef V ValueType;
};
*/

/*
template<class V, typename Entry, typename NextChecker> struct StaticHashTableEntryChecker
{
	static V get(const String &key)		{ if (Entry::checkKey(key)) return Entry::getValue(); return NextChecker::get(key); }
};

template<class V> struct StaticHashTableFinalChecker
{
	static V get(const String &key)		{ return V(); }
};

template<class V, int slot, typename... Entries> struct StaticHashTableSlot
{
	typedef StaticHashTableFinalChecker<V> Checker;
};

template<class V, int slot, typename First, typename... Others> struct StaticHashTableSlot<V, slot, First, Others...>
{
	typedef typename StaticHashTableSlot<V, slot, Others...>::Checker NextChecker;
	typedef TemplateIf<slot == First::keyHash, StaticHashTableEntryChecker<V, First, NextChecker>, NextChecker> Checker;
};

template<typename First, typename... Others> struct StaticHashTable
{
	typedef typename First::ValueType V;

	static V get(const String &key)
	{
		switch (key.getHash()) {
			case  0: return StaticHashTableSlot<V,  0, First, Others...>::Checker::get(key);
			case  1: return StaticHashTableSlot<V,  1, First, Others...>::Checker::get(key);
			case  2: return StaticHashTableSlot<V,  2, First, Others...>::Checker::get(key);
			case  3: return StaticHashTableSlot<V,  3, First, Others...>::Checker::get(key);
			case  4: return StaticHashTableSlot<V,  4, First, Others...>::Checker::get(key);
			case  5: return StaticHashTableSlot<V,  5, First, Others...>::Checker::get(key);
			case  6: return StaticHashTableSlot<V,  6, First, Others...>::Checker::get(key);
			case  7: return StaticHashTableSlot<V,  7, First, Others...>::Checker::get(key);
			case  8: return StaticHashTableSlot<V,  8, First, Others...>::Checker::get(key);
			case  9: return StaticHashTableSlot<V,  9, First, Others...>::Checker::get(key);
			case 10: return StaticHashTableSlot<V, 10, First, Others...>::Checker::get(key);
			case 11: return StaticHashTableSlot<V, 11, First, Others...>::Checker::get(key);
			case 12: return StaticHashTableSlot<V, 12, First, Others...>::Checker::get(key);
			case 13: return StaticHashTableSlot<V, 13, First, Others...>::Checker::get(key);
			case 14: return StaticHashTableSlot<V, 14, First, Others...>::Checker::get(key);
			case 15: return StaticHashTableSlot<V, 15, First, Others...>::Checker::get(key);
			case 16: return StaticHashTableSlot<V, 16, First, Others...>::Checker::get(key);
			case 17: return StaticHashTableSlot<V, 17, First, Others...>::Checker::get(key);
			case 18: return StaticHashTableSlot<V, 18, First, Others...>::Checker::get(key);
			case 19: return StaticHashTableSlot<V, 19, First, Others...>::Checker::get(key);
			case 20: return StaticHashTableSlot<V, 20, First, Others...>::Checker::get(key);
			case 21: return StaticHashTableSlot<V, 21, First, Others...>::Checker::get(key);
			case 22: return StaticHashTableSlot<V, 22, First, Others...>::Checker::get(key);
			case 23: return StaticHashTableSlot<V, 23, First, Others...>::Checker::get(key);
			case 24: return StaticHashTableSlot<V, 24, First, Others...>::Checker::get(key);
			case 25: return StaticHashTableSlot<V, 25, First, Others...>::Checker::get(key);
			case 26: return StaticHashTableSlot<V, 26, First, Others...>::Checker::get(key);
			case 27: return StaticHashTableSlot<V, 27, First, Others...>::Checker::get(key);
			case 28: return StaticHashTableSlot<V, 28, First, Others...>::Checker::get(key);
			case 29: return StaticHashTableSlot<V, 29, First, Others...>::Checker::get(key);
			case 30: return StaticHashTableSlot<V, 30, First, Others...>::Checker::get(key);
			case 31: return StaticHashTableSlot<V, 31, First, Others...>::Checker::get(key);
		}
		return V();
	}
};
*/


template<class V, typename... Entries> struct StaticHashTableEntries
{
	template<int i> static V getFromSlot(const String &key)		{ return V(); }
};

template<class V, typename First, typename... Others> struct StaticHashTableEntries<V, First, Others...>
{
	template<int i> static V getFromSlot(const String &key)		{ if (i == First::keyHash && First::checkKey(key)) return First::getValue(); return StaticHashTableEntries<V, Others...>::template getFromSlot<i>(key); }
};

template<typename First, typename... Others> struct StaticHashTable : private StaticHashTableEntries<typename First::ValueType, First, Others...>
{
	typedef typename First::ValueType V;

	static V get(const String &key)
	{
		switch (key.getHash()) {
			case  0: return StaticHashTableEntries<V, First, Others...>::template getFromSlot<0>(key);
			case  1: return StaticHashTableEntries<V, First, Others...>::template getFromSlot<1>(key);
			case  2: return StaticHashTableEntries<V, First, Others...>::template getFromSlot<2>(key);
			case  3: return StaticHashTableEntries<V, First, Others...>::template getFromSlot<3>(key);
			case  4: return StaticHashTableEntries<V, First, Others...>::template getFromSlot<4>(key);
			case  5: return StaticHashTableEntries<V, First, Others...>::template getFromSlot<5>(key);
			case  6: return StaticHashTableEntries<V, First, Others...>::template getFromSlot<6>(key);
			case  7: return StaticHashTableEntries<V, First, Others...>::template getFromSlot<7>(key);
			case  8: return StaticHashTableEntries<V, First, Others...>::template getFromSlot<8>(key);
			case  9: return StaticHashTableEntries<V, First, Others...>::template getFromSlot<9>(key);
			case 10: return StaticHashTableEntries<V, First, Others...>::template getFromSlot<10>(key);
			case 11: return StaticHashTableEntries<V, First, Others...>::template getFromSlot<11>(key);
			case 12: return StaticHashTableEntries<V, First, Others...>::template getFromSlot<12>(key);
			case 13: return StaticHashTableEntries<V, First, Others...>::template getFromSlot<13>(key);
			case 14: return StaticHashTableEntries<V, First, Others...>::template getFromSlot<14>(key);
			case 15: return StaticHashTableEntries<V, First, Others...>::template getFromSlot<15>(key);
			case 16: return StaticHashTableEntries<V, First, Others...>::template getFromSlot<16>(key);
			case 17: return StaticHashTableEntries<V, First, Others...>::template getFromSlot<17>(key);
			case 18: return StaticHashTableEntries<V, First, Others...>::template getFromSlot<18>(key);
			case 19: return StaticHashTableEntries<V, First, Others...>::template getFromSlot<19>(key);
			case 20: return StaticHashTableEntries<V, First, Others...>::template getFromSlot<20>(key);
			case 21: return StaticHashTableEntries<V, First, Others...>::template getFromSlot<21>(key);
			case 22: return StaticHashTableEntries<V, First, Others...>::template getFromSlot<22>(key);
			case 23: return StaticHashTableEntries<V, First, Others...>::template getFromSlot<23>(key);
			case 24: return StaticHashTableEntries<V, First, Others...>::template getFromSlot<24>(key);
			case 25: return StaticHashTableEntries<V, First, Others...>::template getFromSlot<25>(key);
			case 26: return StaticHashTableEntries<V, First, Others...>::template getFromSlot<26>(key);
			case 27: return StaticHashTableEntries<V, First, Others...>::template getFromSlot<27>(key);
			case 28: return StaticHashTableEntries<V, First, Others...>::template getFromSlot<28>(key);
			case 29: return StaticHashTableEntries<V, First, Others...>::template getFromSlot<29>(key);
			case 30: return StaticHashTableEntries<V, First, Others...>::template getFromSlot<30>(key);
			case 31: return StaticHashTableEntries<V, First, Others...>::template getFromSlot<31>(key);
		}
		return V();
	}
};
