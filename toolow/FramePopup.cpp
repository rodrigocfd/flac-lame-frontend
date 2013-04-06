
#include "Frame.h"
#include "Font.h"

FramePopup::~FramePopup()
{
}

LRESULT FramePopup::msgHandler(UINT msg, WPARAM wp, LPARAM lp)
{
	static Font hSysFont; // to be shared among all regular container windows

	switch(msg)
	{
	case WM_CREATE:
		if(!hSysFont.hFont()) {
			Font::Info nfof;
			Font::GetDefaultDialogFontInfo(&nfof);
			hSysFont.create(&nfof);
		}
		hSysFont.applyOnChildren(this->hWnd());

		SetFocus(GetNextDlgTabItem(this->hWnd(), 0, FALSE)); // focus 1st child according to tab order
		this->sendMessage(WM_INITDIALOG, 0, 0); // can be used to process stuff after the WM_CREATE default processing (font & focus)
		break;

	case WM_ACTIVATE:
		if(!HIWORD(wp)) { // it not in minimized state
			if(LOWORD(wp) == WA_INACTIVE)
				this->_hWndCurFocus = GetFocus(); // save currently focused window
			else
				SetFocus(this->_hWndCurFocus); // restore focus back
			return 0;
		}
		break;
	}
	return Frame::msgHandler(msg, wp, lp); // forward to parent class message handler
}

Window FramePopup::createButton(const wchar_t *caption, int id, int x, int y, int cx, bool def)
{
	return CreateWindowEx(0, L"BUTTON", caption,
		WS_CHILD | WS_VISIBLE | WS_TABSTOP | (def ? BS_DEFPUSHBUTTON : 0), x, y, cx, 23,
		this->hWnd(), (HMENU)id, this->getInstance(), 0);
}

Window FramePopup::createLabel(const wchar_t *caption, int x, int y, int cx, int id)
{
	return CreateWindowEx(0, L"STATIC", caption,
		WS_CHILD | WS_VISIBLE, x, y, cx, 17,
		this->hWnd(), (HMENU)id, this->getInstance(), 0);
}

Window FramePopup::createCheck(const wchar_t *caption, int id, int x, int y, int cx)
{
	return CreateWindowEx(0, L"BUTTON", caption,
		WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX, x, y, cx, 21,
		this->hWnd(), (HMENU)id, this->getInstance(), 0);
}

Window FramePopup::createEdit(int id, int x, int y, int cx, UINT extraStyles)
{
	return CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", 0,
		WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL | extraStyles, x, y, cx, 21,
		this->hWnd(), (HMENU)id, this->getInstance(), 0);
}

Window FramePopup::createCombo(int id, int x, int y, int cx)
{
	return CreateWindowEx(0, L"COMBOBOX", 0,
		WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | CBS_SORT, x, y, cx, 0,
		this->hWnd(), (HMENU)id, this->getInstance(), 0);
}