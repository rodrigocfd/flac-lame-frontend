//
// Basic array automation.
// Part of TOOLOW - Thin Object Oriented Layer Over Win32.
// @author Rodrigo Cesar de Freitas Dias
// @see https://github.com/rodrigocfd/toolow
//

#pragma once
#include <functional>
#include <new>
#include <stdlib.h>
#include <string.h>
using std::function;
using std::initializer_list; // because these should be keywords
#define MOVE std::move

template<typename T> class Array final {
private:
	T  *_ptr;
	int _sz, _stor;
public:
	Array()                        : _ptr(nullptr), _sz(0), _stor(0) { }
	Array(const Array& other)      : _ptr(nullptr), _sz(0), _stor(0) { operator=(other); }
	Array(Array&& other)           : _ptr(nullptr), _sz(0), _stor(0) { operator=(MOVE(other)); }
	Array(initializer_list<T> arr) : _ptr(nullptr), _sz(0), _stor(0) { operator=(arr); }
	explicit Array(int length)     : _ptr(nullptr), _sz(0), _stor(0) { this->resize(length); }
	~Array()                       { this->resize(0).compact(); }

	Array& reserve(int length) {
		if (length > _stor) { // only grows
			_ptr = (T*)::realloc(_ptr, sizeof(T) * length); // increase storage room
			_stor = length;
		}
		return *this;
	}
	Array& resize(int length) {
		if (length >= 0) {
			for (int i = length; i < _sz; ++i) _ptr[i].~T(); // when size < _sz, call destructors
			this->reserve(length);
			for (int i = _sz; i < length; ++i) new(_ptr + i) T; // when size > _sz, call constructors
			_sz = length;
		}
		return *this;
	}
	Array& compact() {
		_stor = _sz;
		if (_stor) {
			_ptr = (T*)::realloc(_ptr, sizeof(T) * _stor); // decrease storage room
		} else {
			::free(_ptr);
			_ptr = nullptr;
		}
		return *this;
	}

	Array& operator=(const Array& other) {
		this->reserve(other._sz);
		for (int i = 0; i < other._sz; ++i)
			this->append(other._ptr[i]);
		return *this;
	}
	Array& operator=(Array&& other) {
		this->resize(0).compact();
		_ptr = other._ptr;    _sz = other._sz; _stor = other._stor; // steal pointer
		other._ptr = nullptr; other._sz = 0;   other._stor = 0;
		return *this;
	}
	Array& operator=(initializer_list<T> vals) {
		this->reserve((int)vals.size());
		for (T& val : vals) this->append(val);
		return *this;
	}

	int      size() const                { return _sz; }
	const T& operator[](int index) const { return _ptr[index]; }
	T&       operator[](int index)       { return _ptr[index]; }
	const T& last() const                { return _ptr[_sz - 1]; }
	T&       last()                      { return _ptr[_sz - 1]; }

	Array& remove(int index) {
		if (index >= 0 && index < _sz) {
			_ptr[index].~T();
			if (index < _sz - 1)
				::memmove(_ptr + index, _ptr + index + 1, (_sz - index - 1) * sizeof(T));
			--_sz;
		}
		return *this;
	}

	Array& insert(int atIndex, const T *arr, int howMany) {
		if (atIndex > _sz) atIndex = _sz; // avoid index out of bounds
		this->reserve(_sz + howMany);
		if (atIndex < _sz)
			::memmove(_ptr + atIndex + howMany, _ptr + atIndex, (_sz - atIndex) * sizeof(T));
		for (int i = 0; i < howMany; ++i)
			new(_ptr + atIndex + i) T(arr[i]); // call copy constructor
		_sz += howMany;
		return *this;
	}
	Array& insert(int atIndex, initializer_list<T> vals) { return this->insert(atIndex, vals.begin(), (int)vals.size()); }
	Array& insert(int atIndex, const Array<T>& other)    { return this->insert(atIndex, other._ptr, other._sz); }
	Array& insert(int atIndex, const T& obj)             { return this->insert(atIndex, &obj, 1); }

	Array& append(const T *arr, int howMany) { return this->insert(_sz, arr, howMany); }
	Array& append(initializer_list<T> vals)  { return this->append(vals.begin(), (int)vals.size()); }
	Array& append(const Array<T>& other)     { return this->append(other._ptr, other._sz); }
	Array& append(const T& obj)              { return this->append(&obj, 1); }

	Array& reorder(int index, int newIndex) {
		if (index >= _sz || newIndex >= _sz) return *this;
		T *tmp = (T*)::_alloca(sizeof(T));
		::memcpy(tmp, _ptr + index, sizeof(T)); // store element to be moved
		newIndex > index ?
			::memmove(_ptr + index, _ptr + index + 1, sizeof(T) * (newIndex - index)) :
			::memmove(_ptr + newIndex + 1, _ptr + newIndex, sizeof(T) * (index - newIndex));
		::memcpy(_ptr + newIndex, tmp, sizeof(T));
		return *this;
	}

	Array& swap(Array& other) {
		T *tmpPtr = _ptr;    int tmpSz = _sz;   int tmpStor = _stor;
		_ptr = other._ptr;   _sz = other._sz;   _stor = other._stor;
		other._ptr = tmpPtr; other._sz = tmpSz; other._stor = tmpStor;
		return *this;
	}

	template<typename Ty> Array<Ty> transform(function<Ty(int i, const T& elem)> callback) {
		// Example usage:
		// Array<int> nums;
		// Array<float> trans = nums.transform<float>([](int i, const int& elem)->float { return (float)elem; });
		Array<Ty> ret(_sz); // prealloc
		for (int i = 0; i < _sz; ++i)
			ret[i] = callback(i, _ptr[i]); // invokes operator= on elements
		return ret;
	}

	Array filter(function<bool(int i, const T& elem)> callback) {
		// Example usage:
		// Array<float> nums;
		// Array<float> filtered = nums.filter([](int i, const float& elem)->bool { return elem < 25; });
		Array ret;
		for (int i = 0; i < _sz; ++i)
			if (callback(i, _ptr[i]))
				ret.append(_ptr[i]);
		return ret;
	}

	Array& sort(function<int(const T& a, const T& b)> callback) {
		// Example usage:
		// Array<float> nums;
		// nums.sort([](const float& a, const float& b)->int { return (int)(a - b); }); // lowest to highest
		::qsort_s(_ptr, _sz, sizeof(T), [](void *compareFunc, const void *a, const void *b)->int {
			typedef function<int(const T& a, const T& b)> Fun;
			Fun *callback = (Fun*)compareFunc;
			return (*callback)(*((const T*)a), *((const T*)b));
		}, &callback);
		return *this;
	}

	// Allows the use of C++11 for-range loops.
	class Iter { // http://www.cprogramming.com/c++11/c++11-ranged-for-loop.html
	private:
		int _pos;
		Array *_pArr;
	public:
		Iter(Array *pArr, int pos) : _pos(pos), _pArr(pArr) { }
		bool operator!=(const Iter& other) const { return _pos != other._pos; }
		T& operator*() { return (*_pArr)[_pos]; }
		Iter& operator++() { ++_pos; return *this; }
	};
	class IterConst {
	private:
		int _pos;
		const Array *_pArr;
	public:
		IterConst(const Array *pArr, int pos) : _pos(pos), _pArr(pArr) { }
		bool operator!=(const IterConst& other) const { return _pos != other._pos; }
		const T& operator*() const { return (*_pArr)[_pos]; }
		const IterConst& operator++() { ++_pos; return *this; }
	};
	Iter      begin()       { return Iter(this, 0); }
    Iter      end()         { return Iter(this, this->size()); }
	IterConst begin() const { return IterConst(this, 0); }
    IterConst end() const   { return IterConst(this, this->size()); }
};