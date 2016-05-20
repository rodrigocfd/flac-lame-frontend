/**
 * Part of WinUtil - Windows Utilities
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winutil
 */

#pragma once
#include <string>
#include <Windows.h>

namespace winutil {

class label final {
private:
	HWND _hWnd;
public:
	label()          : _hWnd(nullptr) { }
	label(HWND hwnd) : _hWnd(hwnd) { }
	label& operator=(HWND hwnd);

	HWND         hwnd() const                    { return _hWnd; }
	label&       create(HWND hParent, int id, POINT pos, SIZE size);
	int          get_id() const                  { return GetDlgCtrlID(_hWnd); }
	label&       set_text(const wchar_t* t);
	label&       set_text(const std::wstring& t) { return set_text(t.c_str()); }
	std::wstring get_text() const;
	label&       enable(bool doEnable);
};

}//namespace winutil