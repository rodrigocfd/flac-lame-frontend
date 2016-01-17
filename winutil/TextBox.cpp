
#include "TextBox.h"
#include <CommCtrl.h>
#include "Str.h"
#include "Sys.h"
using std::function;
using std::pair;
using std::vector;
using std::wstring;

TextBox::TextBox()
	: _hWnd(nullptr)
{
}

TextBox::TextBox(HWND hwnd)
	: _hWnd(nullptr)
{
	operator=(hwnd);
}

TextBox& TextBox::operator=(HWND hwnd)
{
	const int IDSUBCLASS = 1;

	if (_hWnd) RemoveWindowSubclass(_hWnd, _proc, IDSUBCLASS);
	_hWnd = hwnd;
	SetWindowSubclass(_hWnd, _proc, IDSUBCLASS, reinterpret_cast<DWORD_PTR>(this));
	return *this;
}

HWND TextBox::hWnd() const
{
	return _hWnd;
}

TextBox& TextBox::create(HWND hParent, int id, POINT pos, LONG width)
{
	return _create(hParent, id, pos, {width,21}, ES_AUTOHSCROLL);
}

TextBox& TextBox::createPassword(HWND hParent, int id, POINT pos, LONG width)
{
	return _create(hParent, id, pos, {width,21}, ES_AUTOHSCROLL | ES_PASSWORD);
}

TextBox& TextBox::createMultiLine(HWND hParent, int id, POINT pos, SIZE size)
{
	return _create(hParent, id, pos, size, ES_MULTILINE | ES_WANTRETURN);
}

TextBox& TextBox::setText(const wchar_t *t)
{
	SetWindowText(_hWnd, t);
	return *this;
}

TextBox& TextBox::setText(const wstring& t)
{
	return setText(t.c_str());
}

wstring TextBox::getText() const
{
	int txtLen = GetWindowTextLength(_hWnd);
	wstring buf(txtLen + 1, L'\0');
	GetWindowText(_hWnd, &buf[0], txtLen + 1);
	buf.resize(txtLen);
	return buf;
}

vector<wstring> TextBox::getTextLines() const
{
	return Str::explode(getText(), L"\r\n");
}

TextBox& TextBox::setSelection(int start, int length)
{
	SendMessage(_hWnd, EM_SETSEL, start, start + length);
	return *this;
}

pair<int, int> TextBox::getSelection() const
{
	int p0 = 0, p1 = 0;
	SendMessage(_hWnd, EM_GETSEL,
		reinterpret_cast<WPARAM>(&p0), reinterpret_cast<LPARAM>(&p1));
	return std::make_pair(p0, p1 - p0); // start, length
}

TextBox& TextBox::replaceSelection(const wchar_t *t)
{
	SendMessage(_hWnd, EM_REPLACESEL, TRUE, reinterpret_cast<LPARAM>(t));
	return *this;
}

TextBox& TextBox::replaceSelection(const std::wstring& t)
{
	return replaceSelection(t.c_str());
}

TextBox& TextBox::enable(bool doEnable)
{
	EnableWindow(_hWnd, doEnable);
	return *this;
}

TextBox& TextBox::onKeyUp(KeyUpFunc callback)
{
	_onKeyUp = std::move(callback);
	return *this;
}

TextBox& TextBox::_create(HWND hParent, int id, POINT pos, SIZE size, DWORD extraStyles)
{
	// For children, WS_BORDER gives old, flat drawing; always use WS_EX_CLIENTEDGE.
	return operator=( CreateWindowEx(WS_EX_CLIENTEDGE, L"Edit", nullptr,
		WS_CHILD | WS_VISIBLE | extraStyles,
		pos.x, pos.y, size.cx, size.cy,
		hParent, reinterpret_cast<HMENU>(static_cast<UINT_PTR>(id)),
		reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hParent, GWLP_HINSTANCE)),
		nullptr) );
}

LRESULT CALLBACK TextBox::_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp,
	UINT_PTR idSubclass, DWORD_PTR refData)
{
	TextBox *pSelf = reinterpret_cast<TextBox*>(refData);

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
		if(lp && wp == 'A' && Sys::hasCtrl()) { // Ctrl+A to select all text
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