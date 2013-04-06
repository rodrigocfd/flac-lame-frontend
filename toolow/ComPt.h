//
// Automation to COM pointers.
// Night of Saturday, October 27, 2012.
// Part of TOOLOW - Thin Object Oriented Layer Over Win32.
//

#pragma once
#include <Windows.h>
#include <ObjBase.h>

template<typename T> class ComPt {
public:
	ComPt()  : pt(0) { }
	~ComPt() { this->release(); }

	void release() {
		if(this->pt) {
			this->pt->Release();
			this->pt = 0;
		}
	}

	bool coCreateInstance(REFCLSID rclsid) {
		return this->pt ? false :
			SUCCEEDED(::CoCreateInstance(rclsid, 0, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&this->pt)));
	}

	template<typename COM_INTERFACE>
	bool queryInterface(REFIID riid, ComPt<COM_INTERFACE> *comObj) {
		return !this->pt ? false :
			SUCCEEDED(this->pt->QueryInterface(riid, (void**)&comObj->pt));
	}

	T *pt;
};