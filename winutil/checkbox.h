/**
 * Part of WinUtil - Windows Utilities
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winutil
 */

#pragma once
#include <Windows.h>

namespace winutil {

class checkbox final {
private:
	HWND _hWnd;
public:
	checkbox()          : _hWnd(nullptr) { }
	checkbox(HWND hwnd) : _hWnd(hwnd) { }
	checkbox& operator=(HWND hwnd);

	HWND      hwnd() const            { return _hWnd; }
	checkbox& enable(bool doEnable);
	checkbox& create(HWND hParent, int id, const wchar_t *caption, POINT pos, SIZE size);
	checkbox& focus();
	bool      is_checked()            { return SendMessage(_hWnd, BM_GETCHECK, 0, 0) == BST_CHECKED; }
	void      set_check(bool checked) { SendMessage(_hWnd, BM_SETCHECK, checked ? BST_CHECKED : BST_UNCHECKED, 0); }
	void      set_check_and_trigger(bool checked);
};

}//namespace winutil