//
// Associative array automation.
// Sunday night of September 8, 2013.
// Part of TOOLOW - Thin Object Oriented Layer Over Win32.
//

#pragma once
#include "String.h"

template<typename T> class Hash {
public:
	struct Elem {
		Elem() { }
		explicit Elem(const wchar_t *k) : key(k) { }
		String key;
		T val;
	};

	Hash()                  : _szUsed(0) { }
	Hash(const Hash& other) : _szUsed(0) { operator=(other); }

	int         size() const                     { return _szUsed; }
	bool        exists(const wchar_t *key) const { return _byKey(key) > -1; }
	const Elem* at(int index) const              { return &_elems[index]; }
	Elem*       at(int index)                    { return &_elems[index]; }
	
	const T& operator[](const String& key) const  { return operator[](key.str()); }
	T& operator[](const String& key)              { return operator[](key.str()); }
	const T& operator[](const wchar_t *key) const { return operator[](key); }
	T& operator[](const wchar_t *key) {
		int idx = _byKey(key);
		if(idx == -1) { // key not found
			reserve(++_szUsed); // so let's insert it
			_elems[_szUsed - 1] = Elem(key); // create entry with default constructor
			idx = _szUsed - 1;
		}
		return _elems[idx].val;
	}

	Hash& reserve(int howMany) {
		if(howMany > _elems.size()) _elems.realloc(howMany); // always grows
		return *this;
	}

	Hash& operator=(const Hash& other) {
		reserve(other._szUsed);
		for(int i = 0; i < other._szUsed; ++i) {
			_elems[i].key = other._elems[i].key;
			_elems[i].val = other._elems[i].val;
		}
		return *this;
	}

	Hash& removeAll()                { _elems.free(); _szUsed = 0; return *this; }
	Hash& remove(const wchar_t *key) { return remove(_byKey(key)); }
	Hash& remove(int index) {
		if(index >= 0 && index <= _szUsed - 1) {
			_elems.remove(index);
			--_szUsed;
		}
		return *this;
	}

private:
	Array<Elem> _elems;
	int _szUsed;

	int _byKey(const wchar_t *keyName) const {
		for(int i = 0; i < _szUsed; ++i) // linear search
			if(_elems[i].key.equalSens(keyName)) // an empty string is also a valid key
				return i;
		return -1; // not found
	}
};