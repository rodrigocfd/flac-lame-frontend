//
// Redirects mousewheel messages to controls hovered by it, instead of focused by it.
// Evening of Saturday, December 29, 2012.
// Part of TOOLOW - Thin Object Oriented Layer Over Win32.
//

#pragma once
#include <Windows.h>

class WheelHover {
public:
	WheelHover() : _hParent(0) { }

	WheelHover& create(HWND hParent);
	WheelHover& add(HWND hCtrl);
	WheelHover& add(HWND hParent, int ctrlId);
	WheelHover& addByHwnd(int howMany, ...);
	WheelHover& addById(HWND hParent, int howMany, ...);

private:
	HWND _hParent;

	static LRESULT CALLBACK _SubProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR idSubclass, DWORD_PTR refData);
};