/**
 * Part of WinLamb - Win32 API Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "base_native_control.h"
#include "i_control.h"
#include "i_hwnd.h"
#include "i_text.h"

namespace wl {

class label final :
	public i_hwnd,
	public i_control<label>,
	public i_text<label>
{
private:
	base_native_control _control;

public:
	label() : i_hwnd(_control.wnd()), i_control(this), i_text(this) { }

	label& be(const i_hwnd* ctrl)                  { this->_control.be(ctrl); return *this; }
	label& be(const i_hwnd* parent, int controlId) { this->_control.be(parent, controlId); return *this; }

	label& create(const i_hwnd* parent, int controlId, const wchar_t* caption, POINT pos, SIZE size) {
		this->_control.create(parent, controlId, caption, pos, size, L"Static");
		return *this;
	}
};

}//namespace wl