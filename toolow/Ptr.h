//
// Smart pointer with reference counting, for ordinary and COM pointers.
// Night of Friday, September 13, 2013.
// Part of TOOLOW - Thin Object Oriented Layer Over Win32.
//

#pragma once
#include <Windows.h>
#include <ObjBase.h>

template<typename T> class Ptr {
public:
	Ptr()                 : _ptr(NULL), _counter(0) { }
	Ptr(T *ptr)           : _ptr(NULL), _counter(0) { operator=(ptr); }
	Ptr(const Ptr& other) : _ptr(NULL), _counter(0) { operator=(other); }
	~Ptr() {
		if(_counter) {
			if(!--(*_counter)) {
				delete _ptr; _ptr = NULL;
				delete _counter; _counter = 0;
			}
		}
	}
	Ptr& operator=(T *ptr) {
		this->~Ptr();
		if(ptr) {
			_ptr = ptr; // take ownership
			_counter = new int(1); // start counter
		}
		return *this;
	}
	Ptr& operator=(const Ptr& other) {
		if(this != &other) {
			this->~Ptr();
			_ptr = other._ptr;
			_counter = other._counter;
			if(_counter) ++(*_counter);
		}
		return *this;
	}
	bool isNull() const  { return _ptr == NULL; }
	T& operator*()       { return *_ptr; }
	T* operator->()      { return _ptr; }
	operator T*() const  { return _ptr; }
private:
	T   *_ptr;
	int *_counter;
};


template<typename T> class ComPtr {
public:
	ComPtr()                    : _ptr(NULL) { }
	ComPtr(const ComPtr& other) : _ptr(NULL) { operator=(other); }
	~ComPtr()                   { this->release(); }
	ComPtr& operator=(const ComPtr& other) {
		if(this != &other) {
			this->~ComPtr();
			_ptr = other._ptr;
			if(_ptr) _ptr->AddRef();
		}
		return *this;
	}
	void release() {
		if(_ptr) {
			_ptr->Release();
			_ptr = NULL;
		}
	}
	bool coCreateInstance(REFCLSID rclsid) {
		return _ptr ? false :
			SUCCEEDED(::CoCreateInstance(rclsid, 0, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&_ptr)));
	}
	bool coCreateInstance(REFCLSID rclsid, REFIID riid) {
		return _ptr ? false :
			SUCCEEDED(::CoCreateInstance(rclsid, 0, CLSCTX_INPROC_SERVER, riid, (void**)&_ptr));
	}
	template<typename COM_INTERFACE> bool queryInterface(REFIID riid, COM_INTERFACE **comPtr) {
		return !_ptr ? false :
			SUCCEEDED(_ptr->QueryInterface(riid, (void**)comPtr));
	}
	bool isNull() const { return _ptr == NULL; }
	T&   operator*()    { return *_ptr; }
	T*   operator->()   { return _ptr; }
	T**  operator&()    { return &_ptr; }
	operator T*() const { return _ptr; }
private:
	T *_ptr;
};