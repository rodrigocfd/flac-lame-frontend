/**
 * Part of WinUtil - Windows Utilities
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winutil
 */

#include "label.h"
using namespace winutil;
using std::wstring;

label& label::operator=(HWND hwnd)
{
	_hWnd = hwnd;
	return *this;
}

label& label::create(HWND hParent, int id, POINT pos, SIZE size)
{
	return operator=( CreateWindowEx(0, L"Static", nullptr,
		WS_CHILD | WS_VISIBLE,
		pos.x, pos.y, size.cx, size.cy,
		hParent, reinterpret_cast<HMENU>(static_cast<UINT_PTR>(id)),
		reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hParent, GWLP_HINSTANCE)),
		nullptr) );
}

label& label::set_text(const wchar_t *t)
{
	SetWindowText(_hWnd, t);
	return *this;
}

wstring label::get_text() const
{
	int txtLen = GetWindowTextLength(_hWnd);
	wstring buf(txtLen + 1, L'\0');
	GetWindowText(_hWnd, &buf[0], txtLen + 1);
	buf.resize(txtLen);
	return buf;
}

label& label::enable(bool doEnable)
{
	EnableWindow(_hWnd, doEnable);
	return *this;
}