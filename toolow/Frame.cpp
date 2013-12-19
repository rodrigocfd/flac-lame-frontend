
#include "Frame.h"

Frame::~Frame()
{
}

ATOM Frame::Register(const wchar_t *className, Frame::Cursor cursor, int iconId, SysColor bg)
{
	HINSTANCE hInst = GetModuleHandle(0);
	WNDCLASSEX wc = { 0 };

	wc.cbSize        = sizeof(wc);
	wc.lpfnWndProc   = Frame::_WindowProc;
	wc.hInstance     = hInst;
	wc.lpszClassName = className;
	wc.hbrBackground = (HBRUSH)((int)bg + 1); // http://www.newobjects.com/pages/ndl/alp%5Caf-sysColor.htm
	wc.hCursor       = LoadCursor(0, MAKEINTRESOURCE(cursor));
	wc.style         = CS_DBLCLKS;
	
	if(iconId) {
		wc.hIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(iconId), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);
		wc.hIconSm = (HICON)LoadImage(hInst, MAKEINTRESOURCE(iconId), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	}

	return RegisterClassEx(&wc);
}

LRESULT Frame::msgHandler(UINT msg, WPARAM wp, LPARAM lp)
{
	return DefWindowProc(this->hWnd(), msg, wp, lp); // default-most message processing for a regular window
}

LRESULT CALLBACK Frame::_WindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	Frame *pSelf; // in run-time, will be a pointer to the derived-most class
	if(msg == WM_NCCREATE) {
		pSelf = (Frame*)((CREATESTRUCT*)lp)->lpCreateParams; // passed on CreateWindowEx()
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pSelf); // store pointer to object into HWND room
		*(Window*)pSelf = hwnd; // assign hWnd member
	}
	else pSelf = (Frame*)GetWindowLongPtr(hwnd, GWLP_USERDATA); // from HWND room, zero if not set yet
	return pSelf ? pSelf->msgHandler(msg, wp, lp) : DefWindowProc(hwnd, msg, wp, lp); // works, since msgHandler() is virtual
}