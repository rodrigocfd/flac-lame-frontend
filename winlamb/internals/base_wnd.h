/**
 * Part of WinLamb - Win32 API Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include <Windows.h>

namespace wl {
namespace internals {

class base_wnd {
private:
	HWND _hWnd;
public:
	base_wnd()                  : _hWnd(nullptr) { }
	base_wnd(HWND h)            : _hWnd(h) { }
	base_wnd(const base_wnd& w) : _hWnd(w._hWnd) { }

	base_wnd& operator=(HWND h)            { this->_hWnd = h; return *this; }
	base_wnd& operator=(const base_wnd& w) { this->_hWnd = w._hWnd; return *this; }

	HWND hwnd() const { return this->_hWnd; }
};

}//namespace internals
}//namespace wl