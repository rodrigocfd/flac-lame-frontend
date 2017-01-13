/**
 * Part of WinLamb - Win32 API Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "base_native_control.h"
#include "plus_control.h"
#include "plus_text.h"

namespace wl {

class label final : public plus_control<label>, public plus_text<label> {
private:
	base_native_control _control;

public:
	label() : plus_control(this), plus_text(this) { }

	HWND   hwnd() const                    { return this->_control.hwnd(); }
	label& be(HWND hWnd)                   { this->_control.be(hWnd); return *this; }
	label& be(HWND hParent, int controlId) { this->_control.be(hParent, controlId); return *this; }

	label& create(HWND hParent, int controlId, const wchar_t* caption, POINT pos, SIZE size) {
		this->_control.create(hParent, controlId, caption, pos, size, L"Static");
		return *this;
	}
};

}//namespace wl