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

	Combo& operator=(HWND hwnd)          { ((Window*)this)->operator=(hwnd); return *this; }
	Combo& operator=(const Window& wnd)  { return operator=(wnd.hWnd()); }
	Combo& operator=(const Combo& other) { return operator=(other.hWnd()); }

	int    itemCount()               { return (int)sendMessage(CB_GETCOUNT, 0, 0); }
	Combo& itemSetSelected(int i)    { sendMessage(CB_SETCURSEL, i, 0); return *this; }
	int    itemGetSelected()         { return (int)sendMessage(CB_GETCURSEL, 0, 0); }
	
	Combo& itemAdd(std::initializer_list<const wchar_t*> arrStr) {
		for(int i = 0; i < (int)arrStr.size(); ++i)
			sendMessage(CB_ADDSTRING, 0, (LPARAM)*(arrStr.begin() + i));
		return *this;
	}

	wchar_t* itemGetText(int i, wchar_t *pBuf, int szBuf) {
		int len = (int)sendMessage(CB_GETLBTEXTLEN, i, 0) + 1;
		if(szBuf < len) *pBuf = 0; // buffer is too small
		else sendMessage(CB_GETLBTEXT, i, (LPARAM)pBuf);
		return pBuf;
	}
	String* itemGetText(int i, String *pBuf) {
		pBuf->reserve((int)sendMessage(CB_GETLBTEXTLEN, i, 0));
		sendMessage(CB_GETLBTEXT, i, (LPARAM)pBuf->ptrAt(0));
		return pBuf;
	}
	String itemGetText(int i) {
		String ret;
		itemGetText(i, &ret);
		return ret;
	}

	Combo& itemRemoveAll() { sendMessage(CB_RESETCONTENT, 0, 0); return *this; }
};

//__________________________________________________________________________________________________
// Regular listbox.
//

class ListBox : public Window {
public:
	ListBox()                     { }
	ListBox(HWND hwnd)            { operator=(hwnd); }
	ListBox(const Window& wnd)    { operator=(wnd); }
	ListBox(const ListBox& other) { operator=(other); }
	
	ListBox& operator=(HWND hwnd)            { ((Window*)this)->operator=(hwnd); return *this; }
	ListBox& operator=(const Window& wnd)    { return operator=(wnd.hWnd()); }
	ListBox& operator=(const ListBox& other) { return operator=(other.hWnd()); }
	
	ListBox& itemAdd(std::initializer_list<const wchar_t*> arrStr) {
		for(int i = 0; i < (int)arrStr.size(); ++i)
			sendMessage(LB_ADDSTRING, 0, (LPARAM)*(arrStr.begin() + i));
		return *this;
	}

	int itemCount() { return (int)sendMessage(LB_GETCOUNT, 0, 0); }
	int itemCountSelected() {
		int cou = (int)sendMessage(LB_GETSELCOUNT, 0, 0);
		if(cou == LB_ERR) // we have a single-selection listbox, zero or one items can be selected
			return sendMessage(LB_GETCURSEL, 0, 0) == LB_ERR ? 0 : 1;
		return cou;
	}
	int itemGetSelected(Array<int> *indexesBuf=0) {
		if(indexesBuf) {
			indexesBuf->realloc(itemCountSelected());
			if(sendMessage(LB_GETSELITEMS, (WPARAM)indexesBuf->size(), (LPARAM)&(*indexesBuf)[0]) == LB_ERR)
				if(indexesBuf->size() > 0) // a single-selection listbox
					(*indexesBuf)[0] = (int)sendMessage(LB_GETCURSEL, 0, 0);
			return indexesBuf->size() > 0 ? (*indexesBuf)[0] : -1;
		}
		return (int)sendMessage(LB_GETCURSEL, 0, 0); // will work for single-selection listbox only
	}

	wchar_t* itemGetText(int i, wchar_t *pBuf, int szBuf) {
		int len = (int)sendMessage(LB_GETTEXTLEN, i, 0) + 1;
		if(szBuf < len) *pBuf = 0; // buffer is too small
		else sendMessage(LB_GETTEXT, i, (LPARAM)pBuf);
		return pBuf;
	}
	String* itemGetText(int i, String *pBuf) {
		pBuf->reserve((int)sendMessage(LB_GETTEXTLEN, i, 0));
		sendMessage(LB_GETTEXT, i, (LPARAM)pBuf->ptrAt(0));
		return pBuf;
	}
	String itemGetText(int i) {
		String ret;
		itemGetText(i, &ret);
		return ret;
	}

	ListBox& itemRemoveAll() { sendMessage(LB_RESETCONTENT, 0, 0); return *this; }
};

//__________________________________________________________________________________________________
// Radio button.
//
class Radio : public Window {
public:
	enum class EmulateClick { EMULATE, NOEMULATE };

	Radio()                   { }
	Radio(HWND hwnd)          { operator=(hwnd); }
	Radio(const Window& wnd)  { operator=(wnd); }
	Radio(const Radio& other) { operator=(other); }

	Radio& operator=(HWND hwnd)          { ((Window*)this)->operator=(hwnd); return *this; }
	Radio& operator=(const Window& wnd)  { return operator=(wnd.hWnd()); }
	Radio& operator=(const Radio& other) { return operator=(other.hWnd()); }

	bool isChecked() { return sendMessage(BM_GETCHECK, 0, 0) == BST_CHECKED; }

	void setCheck(bool checked, EmulateClick emulateClick) {
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

	CheckBox& operator=(HWND hwnd)             { ((Window*)this)->operator=(hwnd); return *this; }
	CheckBox& operator=(const Window& wnd)     { return operator=(wnd.hWnd()); }
	CheckBox& operator=(const CheckBox& other) { return operator=(other.hWnd()); }
};

//__________________________________________________________________________________________________
// Progress bar.
//
class ProgressBar : public Window {
public:
	ProgressBar()                         { }
	ProgressBar(HWND hwnd)                { operator=(hwnd); }
	ProgressBar(const Window& wnd)        { operator=(wnd); }
	ProgressBar(const ProgressBar& other) { operator=(other); }

	ProgressBar& operator=(HWND hwnd)                { ((Window*)this)->operator=(hwnd); return *this; }
	ProgressBar& operator=(const Window& wnd)        { return operator=(wnd.hWnd()); }
	ProgressBar& operator=(const ProgressBar& other) { return operator=(other.hWnd()); }

	ProgressBar& setRange(int min, int max) { sendMessage(PBM_SETRANGE, 0, MAKELPARAM(min, max)); return *this; }
	ProgressBar& setPos(int pos)            { sendMessage(PBM_SETPOS, pos, 0); return *this; }
	ProgressBar& setPos(double pos)         { return setPos(int(pos + 0.5)); }
	int          getPos()                   { return (int)sendMessage(PBM_GETPOS, 0, 0); }
	
	ProgressBar& animateMarquee(bool animate) {
		::SetWindowLongPtr(this->hWnd(), GWL_STYLE, ::GetWindowLongPtr(this->hWnd(), GWL_STYLE) | PBS_MARQUEE); // set this on resource editor won't work
		sendMessage(PBM_SETMARQUEE, (WPARAM)animate, 0);
		return *this;
	}
};