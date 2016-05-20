/**
 * Part of WinUtil - Windows Utilities
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winutil
 */

#pragma once
#include <Windows.h>
#include <ObjBase.h>

namespace winutil {

template<typename ptrT> class com_ptr {
private:
	ptrT* _ptr;
public:
	~com_ptr() { this->release(); }
	com_ptr() : _ptr(nullptr) { }
	com_ptr(const com_ptr&) = delete;
	com_ptr(com_ptr&& other) : com_ptr() { operator=(std::move(other)); }
	
	com_ptr& operator=(const com_ptr&) = delete;

	com_ptr& operator=(com_ptr&& other) {
		std::swap(_ptr, other._ptr);
		return *this;
	}

	void release() {
		if (_ptr) {
			_ptr->Release();
			_ptr = nullptr;
		}
	}

	bool co_create_instance(REFCLSID rclsid) {
		return _ptr ? false :
			SUCCEEDED(CoCreateInstance(rclsid, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&_ptr)));
	}

	bool co_create_instance(REFCLSID rclsid, REFIID riid) {
		return _ptr ? false :
			SUCCEEDED(CoCreateInstance(rclsid, nullptr, CLSCTX_INPROC_SERVER, riid, reinterpret_cast<void**>(&_ptr)));
	}

	template<typename COM_INTERFACE> bool query_interface(REFIID riid, COM_INTERFACE** comPtr) {
		return !_ptr ? false :
			SUCCEEDED(_ptr->QueryInterface(riid, reinterpret_cast<void**>(comPtr)));
	}

	bool   empty() const   { return _ptr == nullptr; }
	ptrT&  operator*()     { return *_ptr; }
	ptrT*  operator->()    { return _ptr; }
	ptrT** operator&()     { return &_ptr; }
	operator ptrT*() const { return _ptr; }
};

}//namespace winutil