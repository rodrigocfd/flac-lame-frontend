/**
 * Part of WinLamb - Windows Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include <Windows.h>
#include <CommCtrl.h>

namespace winlamb {

class wnd {
protected:
	HWND _hWnd;
public:
	virtual ~wnd() = default;

	wnd()             : _hWnd(nullptr) { }
	wnd(HWND h)       : _hWnd(h) { }
	wnd(const wnd& w) : _hWnd(w._hWnd) { }

	wnd& operator=(HWND h)       { _hWnd = h; return *this; }
	wnd& operator=(const wnd& w) { _hWnd = w._hWnd; return *this; }

	HWND hwnd() const { return _hWnd; }
};

}//namespace winlamb