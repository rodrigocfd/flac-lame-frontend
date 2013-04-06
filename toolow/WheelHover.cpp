
#include "WheelHover.h"
#include <Commctrl.h>
#pragma comment(lib, "ComCtl32.lib")

WheelHover& WheelHover::create(HWND hParent)
{
	_hParent = hParent;
	return *this;
}

WheelHover& WheelHover::add(HWND hCtrl)
{
	static int uniqueSubId = 1;
	SetWindowSubclass(hCtrl, _SubProc, uniqueSubId++, (DWORD_PTR)_hParent); // yes, subclass every control
	return *this;
}

WheelHover& WheelHover::add(HWND hParent, int ctrlId)
{
	return this->add(GetDlgItem(hParent, ctrlId));
}

WheelHover& WheelHover::addByHwnd(int howMany, ...)
{
	va_list marker;
	va_start(marker, howMany);

	for(int i = 0; i < howMany; ++i)
		this->add(va_arg(marker, HWND)); // user should pass HWND of each child to be added
	
	va_end(marker);
	return *this;
}

WheelHover& WheelHover::addById(HWND hParent, int howMany, ...)
{
	va_list marker;
	va_start(marker, howMany);

	for(int i = 0; i < howMany; ++i)
		this->add(GetDlgItem(hParent, va_arg(marker, int))); // user should pass item ID of each child
	
	va_end(marker);
	return *this;
}

LRESULT CALLBACK WheelHover::_SubProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR idSubclass, DWORD_PTR refData)
{
	switch(msg)
	{
	case WM_MOUSEWHEEL:
		if(!(LOWORD(wp) & 0x0800)) { // bitflag not set, this is the first and unprocessed WM_MOUSEWHEEL passage
			HWND hParent = (HWND)refData;
			POINT pt = { LOWORD(lp), HIWORD(lp) };
			ScreenToClient(hParent, &pt); // to client coordinates relative to parent
			SendMessage(ChildWindowFromPoint(hParent, pt), // window below cursor
				WM_MOUSEWHEEL,
				MAKEWPARAM(LOWORD(wp) | 0x0800, HIWORD(wp)), // set 0x0800 bitflag and kick to window below cursor
				lp);
			return 0; // halt processing
		} else { // bitflag is set, WM_MOUSEWHEEL has been kicked here and can be safely processed
			wp &= ~0x0800; // unset bitflag
			break; // finally dispatch to default processing
		}
	case WM_NCDESTROY:
		RemoveWindowSubclass(hwnd, _SubProc, idSubclass); // http://blogs.msdn.com/b/oldnewthing/archive/2003/11/11/55653.aspx
	}
	return DefSubclassProc(hwnd, msg, wp, lp);
}