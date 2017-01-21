/**
 * Part of WinLamb - Win32 API Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "internals/i_control.h"
#include "internals/i_text.h"
#include "internals/native_control.h"
#include "i_hwnd.h"

namespace wl {

class label final :
	public i_hwnd,
	public internals::i_control<label>,
	public internals::i_text<label>
{
private:
	internals::native_control _control;

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