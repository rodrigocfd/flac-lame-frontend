/**
 * Part of WinUtil - Windows Utilities
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winutil
 */

#include "textbox.h"
#include <CommCtrl.h>
#include "str.h"
#include "sys.h"
using namespace winutil;
using std::function;
using std::vector;
using std::wstring;

const int IDSUBCLASS = 1;

textbox& textbox::operator=(HWND hwnd)
{
	if (_hWnd != hwnd) {
		if (_hWnd) RemoveWindowSubclass(_hWnd, _proc, IDSUBCLASS);
		_hWnd = hwnd;
		SetWindowSubclass(_hWnd, _proc, IDSUBCLASS, reinterpret_cast<DWORD_PTR>(this));
	}
	return *this;
}

textbox& textbox::operator=(textbox&& other)
{
	_onKeyUp = std::move(other._onKeyUp);
	operator=(other._hWnd);
	other._hWnd = nullptr;
	return *this;
}

textbox& textbox::set_text(const wchar_t *t)
{
	SetWindowText(_hWnd, t);
	return *this;
}

wstring textbox::get_text() const
{
	int txtLen = GetWindowTextLength(_hWnd);
	wstring buf(txtLen + 1, L'\0');
	GetWindowText(_hWnd, &buf[0], txtLen + 1);
	buf.resize(txtLen);
	return buf;
}

vector<wstring> textbox::get_text_lines() const
{
	return str::explode(get_text(), L"\r\n");
}

textbox& textbox::set_selection(selection selec)
{
	SendMessage(_hWnd, EM_SETSEL, selec.start, selec.start + selec.len);
	return *this;
}

textbox::selection textbox::get_selection() const
{
	int p0 = 0, p1 = 0;
	SendMessage(_hWnd, EM_GETSEL,
		reinterpret_cast<WPARAM>(&p0), reinterpret_cast<LPARAM>(&p1));
	return { p0, p1 - p0 }; // start, length
}

textbox& textbox::replace_selection(const wchar_t *t)
{
	SendMessage(_hWnd, EM_REPLACESEL, TRUE, reinterpret_cast<LPARAM>(t));
	return *this;
}

textbox& textbox::enable(bool doEnable)
{
	EnableWindow(_hWnd, doEnable);
	return *this;
}

textbox& textbox::focus()
{
	SetFocus(_hWnd);
	return *this;
}

textbox& textbox::on_keyup(func_keyup_type callback)
{
	_onKeyUp = std::move(callback);
	return *this;
}

textbox& textbox::_create(HWND hParent, int id, POINT pos, SIZE size, DWORD extraStyles)
{
	// For children, WS_BORDER gives old, flat drawing; always use WS_EX_CLIENTEDGE.
	return operator=( CreateWindowEx(WS_EX_CLIENTEDGE, L"Edit", nullptr,
		WS_CHILD | WS_VISIBLE | extraStyles,
		pos.x, pos.y, size.cx, size.cy,
		hParent, reinterpret_cast<HMENU>(static_cast<UINT_PTR>(id)),
		reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hParent, GWLP_HINSTANCE)),
		nullptr) );
}

LRESULT CALLBACK textbox::_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR idSubclass, DWORD_PTR refData)
{
	textbox *pSelf = reinterpret_cast<textbox*>(refData);

	switch(msg) {
	case WM_KEYDOWN:
		switch(LOWORD(wp)) {
		case VK_ESCAPE: // ESC http://www.williamwilling.com/blog/?p=28
			SendMessage(GetAncestor(hwnd, GA_PARENT), WM_COMMAND,
				IDCANCEL, reinterpret_cast<LPARAM>(hwnd));
			return 0;
		}
		break;
	case WM_GETDLGCODE:
		if(lp && wp == 'A' && sys::has_ctrl()) { // Ctrl+A to select all text
			reinterpret_cast<MSG*>(lp)->wParam = 0; // prevent propagation, therefore beep
			SendMessage(hwnd, EM_SETSEL, 0, -1);
			return DLGC_WANTCHARS;
		}
		break;
	case WM_KEYUP:
		if(pSelf->_onKeyUp) {
			pSelf->_onKeyUp(static_cast<BYTE>(wp));
		}
		break;
	case WM_NCDESTROY:
		RemoveWindowSubclass(hwnd, _proc, idSubclass);
	}
	return DefSubclassProc(hwnd, msg, wp, lp);
}