/**
 * Part of WinUtil - Windows Utilities
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winutil
 */

#include "checkbox.h"
using namespace winutil;

checkbox& checkbox::operator=(HWND hWnd)
{
	_hWnd = hWnd;
	return *this;
}

checkbox& checkbox::create(HWND hParent, int id, const wchar_t* caption, POINT pos, SIZE size)
{
	return operator=( CreateWindowEx(0, L"Button", caption,
		WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
		pos.x, pos.y, size.cx, size.cy,
		hParent, reinterpret_cast<HMENU>(static_cast<UINT_PTR>(id)),
		reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hParent, GWLP_HINSTANCE)),
		nullptr) );
}

checkbox& checkbox::enable(bool doEnable)
{
	EnableWindow(_hWnd, doEnable);
	return *this;
}

checkbox& checkbox::focus()
{
	SetFocus(_hWnd);
	return *this;
}

void checkbox::set_check_and_trigger(bool checked)
{
	set_check(checked);
	if (checked) {
		SendMessage(GetParent(_hWnd), WM_COMMAND, // emulate user click
			MAKEWPARAM(GetDlgCtrlID(_hWnd), 0),
			reinterpret_cast<LPARAM>(_hWnd) );
	}
}