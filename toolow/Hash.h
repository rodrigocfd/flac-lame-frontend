//
// Associative array automation.
// Part of TOOLOW - Thin Object Oriented Layer Over Win32.
// @author Rodrigo Cesar de Freitas Dias
// @see https://github.com/rodrigocfd/toolow
//

#pragma once
#include "String.h"

template<typename T> class Hash final {
public:
	struct Elem final {
		Elem()                             { }
		Elem(const Elem& other)            : key(other.key), val(other.val) { }
		Elem(Elem&& other)                 : key(MOVE(other.key)), val(MOVE(val)) { }
		explicit Elem(const wchar_t *k)    : key(k) { }
		Elem& operator=(const Elem& other) { key = other.key; val = other.val; return *this; }
		Elem& operator=(Elem&& other)      { key = MOVE(other.key); val = MOVE(other.val); return *this; }
		String key;
		T      val;
	};

private:
	Array<Elem> _elems;
public:
	Hash()                  { }
	Hash(const Hash& other) { operator=(other); }
	Hash(Hash&& other)      { operator=(MOVE(other)); }

	int         size() const                     { return _elems.size(); }
	bool        exists(const wchar_t *key) const { return this->_byKey(key) > -1; }
	const Elem* at(int index) const              { return &_elems[index]; }
	Elem*       at(int index)                    { return &_elems[index]; }

	Hash& operator=(const Hash& other) { _elems.resize(0).append(other._elems); return *this; }
	Hash& operator=(Hash&& other)      { _elems = MOVE(other._elems); return *this; }
	Hash& reserve(int howMany)         { _elems.reserve(howMany); return *this; }
	Hash& removeAll()                  { _elems.resize(0); return *this; }
	Hash& remove(const wchar_t *key)   { return this->remove(this->_byKey(key)); }
	Hash& remove(int index)            { _elems.remove(index); return *this; }

	const T& operator[](const String& key) const  { return operator[](key.str()); }
	const T& operator[](const wchar_t *key) const {
		int idx = this->_byKey(key);
		if(idx == -1) // key not found
			return _elems[0].val; // not right... lame C++ won't allow null references!
		return _elems[idx].val;
	}
	T& operator[](const String& key)  { return operator[](key.str()); }
	T& operator[](const wchar_t *key) {
		int idx = this->_byKey(key);
		if(idx == -1) { // key not found, let's insert it
			_elems.append(Elem(key)); // create entry with default constructor
			idx = _elems.size() - 1;
		}
		return _elems[idx].val;
	}
	
	void each(function<void(Elem& elem)> callback) {
		// Example usage:
		// Hash<int> nums;
		// nums.each([](Hash<int>::Elem& elem) { elem.val += 10; });
		for(int i = 0; i < _elems.size(); ++i)
			callback(_elems[i]);
	}
	void each(function<void(const Elem& elem)> callback) const {
		// Example usage:
		// Hash<int> nums;
		// nums.each([](const Hash<int>::Elem& elem) { int x = elem.val; });
		for(int i = 0; i < _elems.size(); ++i)
			callback(_elems[i]);
	}
private:
	int _byKey(const wchar_t *keyName) const {
		for(int i = 0; i < _elems.size(); ++i) // linear search
			if(_elems[i].key.equalsCS(keyName)) // an empty string is also a valid key
				return i;
		return -1; // not found
	}
};