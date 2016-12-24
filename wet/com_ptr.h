/**
 * Part of WET - WinAPI Easy Toolkit
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wet
 */

#pragma once
#include <Windows.h>
#include <ObjBase.h>

namespace wet {

template<typename ptrT>
class com_ptr final {
private:
	ptrT* _ptr;
public:
	~com_ptr() { this->release(); }
	com_ptr() : _ptr(nullptr) { }
	com_ptr(const com_ptr&) = delete;
	com_ptr(com_ptr&& other) : com_ptr() { operator=(std::move(other)); }
	com_ptr& operator=(const com_ptr&) = delete;

	com_ptr& operator=(com_ptr&& other) {
		std::swap(this->_ptr, other._ptr);
		return *this;
	}

	void release() {
		if (this->_ptr) {
			this->_ptr->Release();
			this->_ptr = nullptr;
		}
	}

	bool co_create_instance(REFCLSID rclsid) {
		return this->_ptr ? false :
			SUCCEEDED(CoCreateInstance(rclsid, nullptr,
				CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&this->_ptr)));
	}

	bool co_create_instance(REFCLSID rclsid, REFIID riid) {
		return this->_ptr ? false :
			SUCCEEDED(CoCreateInstance(rclsid, nullptr,
				CLSCTX_INPROC_SERVER, riid, reinterpret_cast<void**>(&this->_ptr)));
	}

	template<typename COM_INTERFACE>
	bool query_interface(REFIID riid, COM_INTERFACE** comPtr) {
		return !this->_ptr ? false :
			SUCCEEDED(this->_ptr->QueryInterface(riid, reinterpret_cast<void**>(comPtr)));
	}

	bool   empty() const   { return this->_ptr == nullptr; }
	ptrT&  operator*()     { return *this->_ptr; }
	ptrT*  operator->()    { return this->_ptr; }
	ptrT** operator&()     { return &this->_ptr; }
	operator ptrT*() const { return this->_ptr; }
};

}//namespace wet