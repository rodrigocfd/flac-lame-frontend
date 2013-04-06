//
// Array automation.
// Part of TOOLOW - Thin Object Oriented Layer Over Win32.
//

#pragma once
#include <stdlib.h>
#include <string.h>
#include <new>

template<typename T> class Array {
public:
	Array()                  : _ptr(0), _sz(0) { }
	explicit Array(int size) : _ptr(0), _sz(0) { this->realloc(size); }
	~Array()                 { this->free(); }

	int      size() const                { return _sz; }
	const T& operator[](int index) const { return _ptr[index]; }
	T&       operator[](int index)       { return _ptr[index]; }

	Array& realloc(int size) {
		if(!size) return this->free();
		for(int i = size; i < _sz; ++i) _ptr[i].~T(); // when size < _sz, call destructors
		_ptr = (T*)::realloc(_ptr, sizeof(T) * size);
		for(int i = _sz; i < size; ++i) new(_ptr + i) T; // when size > _sz, call constructors
		_sz = size;
		return *this;
	}

	Array& free() {
		if(!_sz) return *this;
		for(int i = 0; i < _sz; ++i) _ptr[i].~T();
		::free(_ptr);
		_ptr = 0; _sz = 0;
		return *this;
	}

	Array& remove(int index) {
		if(index > _sz - 1) return *this; // index out of bounds
		if(_sz == 1) return this->free();
		_ptr[index].~T();
		if(index < _sz - 1)
			::memmove(_ptr + index, _ptr + index + 1, (_sz - index - 1) * sizeof(T));
		_ptr = (T*)::realloc(_ptr, sizeof(T) * --_sz);
		return *this;
	}

	Array& insert(int atIndex, const T *arr, int howMany) {
		if(atIndex > _sz) atIndex = _sz;
		_ptr = (T*)::realloc(_ptr, sizeof(T) * (_sz + howMany));
		if(atIndex < _sz)
			::memmove(_ptr + atIndex + howMany, _ptr + atIndex, (_sz - atIndex) * sizeof(T));
		for(int i = 0; i < howMany; ++i)
			new(_ptr + atIndex + i) T(arr[i]); // call copy constructor
		_sz += howMany;
		return *this;
	}
	Array& insert(int atIndex, const Array<T> *other) { return this->insert(atIndex, other->_ptr, other->_sz); }
	Array& insert(int atIndex, const T& obj)          { return this->insert(atIndex, &obj, 1); }
	
	Array& append(const T *arr, int howMany) { return this->insert(_sz, arr, howMany); }
	Array& append(const Array<T> *other)     { return this->append(other->_ptr, other->_sz); }
	Array& append(const T& obj)              { return this->append(&obj, 1); }

	Array& move(int index, int newIndex) {
		if(index >= _sz || newIndex >= _sz) return *this;
		T *tmp = (T*)::_alloca(sizeof(T));
		::memcpy(tmp, _ptr + index, sizeof(T)); // store element to be moved
		newIndex > index ?
			::memmove(_ptr + index, _ptr + index + 1, sizeof(T) * (newIndex - index)) :
			::memmove(_ptr + newIndex + 1, _ptr + newIndex, sizeof(T) * (index - newIndex));
		::memcpy(_ptr + newIndex, tmp, sizeof(T));
		return *this;
	}

	Array& swap(Array *pOther) {
		T *tmpPtr = _ptr;      int tmpSz = _sz;
		_ptr = pOther->_ptr;   _sz = pOther->_sz;
		pOther->_ptr = tmpPtr; pOther->_sz = tmpSz;
		return *this;
	}

	Array& sort(int (__cdecl *CompareFunc)(const void*, const void*)) {
		::qsort(_ptr, _sz, sizeof(T), CompareFunc);
		return *this;
	}

private:
	T  *_ptr;
	int _sz;
};