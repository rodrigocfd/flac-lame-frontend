//
// Wrappers to some often-used controls.
// Part of TOOLOW - Thin Object Oriented Layer Over Win32.
//

#pragma once
#include "Window.h"
#include <CommCtrl.h>

//__________________________________________________________________________________________________
// Regular combobox.
//
class Combo : public Window {
public:
	Combo()                   { }
	Combo(HWND hwnd)          { operator=(hwnd); }
	Combo(const Window& wnd)  { operator=(wnd); }
	Combo(const Combo& other) { operator=(other); }

	Combo& operator=(HWND hwnd)          { *((Window*)this) = hwnd; return *this; }
	Combo& operator=(const Window& wnd)  { return operator=(wnd.hWnd()); }
	Combo& operator=(const Combo& other) { return operator=(other.hWnd()); }

	int    itemCount()               { return (int)sendMessage(CB_GETCOUNT, 0, 0); }
	Combo& itemSetSelected(int i)    { sendMessage(CB_SETCURSEL, i, 0); return *this; }
	int    itemGetSelected()         { return (int)sendMessage(CB_GETCURSEL, 0, 0); }
	Combo& itemAdd(const wchar_t *s) { sendMessage(CB_ADDSTRING, 0, (LPARAM)s); return *this; }

	Combo& itemAdd(int howMany, const wchar_t **pStrs) {
		for(int i = 0; i < howMany; ++i) // automation for an array of strings
			itemAdd(pStrs[i]);
		return *this;
	}

	wchar_t* itemGetText(int i, wchar_t *pBuf, int szBuf) {
		int len = (int)sendMessage(CB_GETLBTEXTLEN, i, 0) + 1;
		if(szBuf < len) {
			*pBuf = 0;
			::OutputDebugString(L"ERROR on Combo::itemGetText(): buffer is too small.\n");
		}
		else sendMessage(CB_GETLBTEXT, i, (LPARAM)pBuf);
		return pBuf;
	}

	String* itemGetText(int i, String *pBuf) {
		pBuf->reserve((int)sendMessage(CB_GETLBTEXTLEN, i, 0));
		sendMessage(CB_GETLBTEXT, i, (LPARAM)pBuf->ptrAt(0));
		return pBuf;
	}
};

//__________________________________________________________________________________________________
// Radio button.
//
class Radio : public Window {
public:
	struct EmulateClick { enum Value { EMULATE, NOEMULATE }; };

	Radio()                   { }
	Radio(HWND hwnd)          { operator=(hwnd); }
	Radio(const Window& wnd)  { operator=(wnd); }
	Radio(const Radio& other) { operator=(other); }

	Radio& operator=(HWND hwnd)          { *((Window*)this) = hwnd; return *this; }
	Radio& operator=(const Window& wnd)  { return operator=(wnd.hWnd()); }
	Radio& operator=(const Radio& other) { return operator=(other.hWnd()); }

	bool isChecked() { return sendMessage(BM_GETCHECK, 0, 0) == BST_CHECKED; }

	void setCheck(bool checked, EmulateClick::Value emulateClick) {
		sendMessage(BM_SETCHECK, checked ? BST_CHECKED : BST_UNCHECKED, 0);
		if(emulateClick == EmulateClick::EMULATE)
			getParent().sendMessage(WM_COMMAND, MAKEWPARAM(::GetDlgCtrlID(hWnd()), 0), (LPARAM)hWnd());
	}
};

//__________________________________________________________________________________________________
// Checkbox; same behavior of a radio button.
//
class CheckBox : public Radio {
public:
	CheckBox()                      { }
	CheckBox(HWND hwnd)             { operator=(hwnd); }
	CheckBox(const Window& wnd)     { operator=(wnd); }
	CheckBox(const CheckBox& other) { operator=(other); }

	CheckBox& operator=(HWND hwnd)             { *((Window*)this) = hwnd; return *this; }
	CheckBox& operator=(const Window& wnd)     { return operator=(wnd.hWnd()); }
	CheckBox& operator=(const CheckBox& other) { return operator=(other.hWnd()); }
};

//__________________________________________________________________________________________________
// Progress bar.
//
class ProgressBar : public Window {
public:
	ProgressBar()                   { }
	ProgressBar(HWND hwnd)          { operator=(hwnd); }
	ProgressBar(const Window& wnd)  { operator=(wnd); }
	ProgressBar(const ProgressBar& other) { operator=(other); }

	ProgressBar& operator=(HWND hwnd)                { *((Window*)this) = hwnd; return *this; }
	ProgressBar& operator=(const Window& wnd)        { return operator=(wnd.hWnd()); }
	ProgressBar& operator=(const ProgressBar& other) { return operator=(other.hWnd()); }

	ProgressBar& setRange(int min, int max) { sendMessage(PBM_SETRANGE, 0, MAKELPARAM(min, max)); return *this; }
	ProgressBar& setPos(int pos)            { sendMessage(PBM_SETPOS, pos, 0); return *this; }
	int          getPos()                   { return (int)sendMessage(PBM_GETPOS, 0, 0); }
	
	ProgressBar& animateMarquee(bool animate) {
		::SetWindowLongPtr(this->hWnd(), GWL_STYLE, ::GetWindowLongPtr(this->hWnd(), GWL_STYLE) | PBS_MARQUEE); // set this on resource editor won't work
		sendMessage(PBM_SETMARQUEE, (WPARAM)animate, 0);
		return *this;
	}
};