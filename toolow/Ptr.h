//
// Smart pointer implementation.
// Part of TOOLOW - Thin Object Oriented Layer Over Win32.
// @author Rodrigo Cesar de Freitas Dias
// @see https://github.com/rodrigocfd/toolow
//

#pragma once

template<typename T> class Ptr final {
private:
	T   *_ptr;
	int *_counter;
public:
	Ptr()                 : _ptr(nullptr), _counter(nullptr) { }
	Ptr(T *ptr)           : _ptr(nullptr), _counter(nullptr) { operator=(ptr); }
	Ptr(const Ptr& other) : _ptr(nullptr), _counter(nullptr) { operator=(other); }

	~Ptr() {
		if (_counter && !--(*_counter)) {
			delete _ptr;     _ptr = nullptr;
			delete _counter; _counter = nullptr;
		}
	}

	Ptr& operator=(T *ptr) {
		this->~Ptr();
		if (ptr) {
			_ptr = ptr; // take ownership
			_counter = new int(1); // start counter
		}
		return *this;
	}

	Ptr& operator=(const Ptr& other) {
		if (this != &other) {
			this->~Ptr();
			_ptr = other._ptr;
			_counter = other._counter;
			if (_counter) ++(*_counter);
		}
		return *this;
	}

	bool isNull() const         { return _ptr == nullptr; }
	T& operator*()              { return *_ptr; }
	const T* operator->() const { return _ptr; }
	T* operator->()             { return _ptr; }
	operator T*() const         { return _ptr; }
};


template<typename T> class ComPtr final {
private:
	T *_ptr;
public:
	ComPtr()                    : _ptr(nullptr) { }
	ComPtr(const ComPtr& other) : _ptr(nullptr) { operator=(other); }
	~ComPtr()                   { this->release(); }
	
	ComPtr& operator=(const ComPtr& other) {
		if (this != &other) {
			this->~ComPtr();
			_ptr = other._ptr;
			if (_ptr) _ptr->AddRef();
		}
		return *this;
	}
	
	void release() {
		if (_ptr) {
			_ptr->Release();
			_ptr = nullptr;
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

	bool isNull() const { return _ptr == nullptr; }
	T&   operator*()    { return *_ptr; }
	T*   operator->()   { return _ptr; }
	T**  operator&()    { return &_ptr; }
	operator T*() const { return _ptr; }
};