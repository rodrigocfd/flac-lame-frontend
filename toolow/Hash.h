//
// Associative array automation.
// Sunday night of September 8, 2013.
// With moving semantics at Sunday, December 15, 2013.
// Part of TOOLOW - Thin Object Oriented Layer Over Win32.
//

#pragma once
#include "String.h"

template<typename T> class Hash {
public:
	struct Elem {
		Elem()                             { }
		Elem(const Elem& other)            : key(other.key), val(other.val) { }
		Elem(Elem&& other)                 : key((String&&)other.key), val((T&&)val) { }
		explicit Elem(const wchar_t *k)    : key(k) { }
		Elem& operator=(const Elem& other) { key = other.key; val = other.val; return *this; }
		Elem& operator=(Elem&& other)      { key = (String&&)other.key; val = (T&&)other.val; return *this; }
		String key;
		T      val;
	};

public:
	Hash()                  : _szUsed(0) { }
	Hash(const Hash& other) : _szUsed(0) { operator=(other); }
	Hash(Hash&& other)      : _szUsed(0) { operator=((Hash&&)other); }

	int         size() const                     { return _szUsed; }
	bool        exists(const wchar_t *key) const { return this->_byKey(key) > -1; }
	const Elem* at(int index) const              { return &_elems[index]; }
	Elem*       at(int index)                    { return &_elems[index]; }
	
	const T& operator[](const String& key) const  { return operator[](key.str()); }
	T&       operator[](const String& key)        { return operator[](key.str()); }
	const T& operator[](const wchar_t *key) const { return operator[](key); }
	T&       operator[](const wchar_t *key) {
		int idx = this->_byKey(key);
		if(idx == -1) { // key not found
			this->reserve(++_szUsed); // so let's insert it
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
		this->reserve(other._szUsed);
		for(int i = 0; i < other._szUsed; ++i) {
			_elems[i].key = other._elems[i].key;
			_elems[i].val = other._elems[i].val;
		}
		_szUsed = other._szUsed;
		return *this;
	}
	Hash& operator=(Hash&& other) {
		_elems = (Array<Elem>&&)other._elems;
		_szUsed = other._szUsed;
		return *this;
	}

	Hash& removeAll()                { _elems.free(); _szUsed = 0; return *this; }
	Hash& remove(const wchar_t *key) { return this->remove(this->_byKey(key)); }
	Hash& remove(int index) {
		if(index >= 0 && index <= _szUsed - 1) {
			_elems.remove(index);
			--_szUsed;
		}
		return *this;
	}

private:
	Array<Elem> _elems;
	int         _szUsed;

	int _byKey(const wchar_t *keyName) const {
		for(int i = 0; i < _szUsed; ++i) // linear search
			if(_elems[i].key.equals(keyName, String::Case::SENS)) // an empty string is also a valid key
				return i;
		return -1; // not found
	}
};