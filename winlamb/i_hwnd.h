/**
 * Part of WinLamb - Win32 API Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "base_wnd.h"

namespace wl {

class i_hwnd {
private:
	const base_wnd& _wnd;
protected:
	i_hwnd(const base_wnd& w) : _wnd(w) { }

public:
	HWND      hwnd() const      { return this->_wnd.hwnd(); }
	HINSTANCE hinstance() const { return this->_wnd.hinstance(); }
};

}//namespace wl