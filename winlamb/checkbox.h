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

class checkbox final :
	public i_hwnd,
	public internals::i_control<checkbox>,
	public internals::i_text<checkbox>
{
private:
	internals::native_control _control;

public:
	checkbox() : i_hwnd(_control.wnd()), i_control(this), i_text(this) { }

	checkbox& be(const i_hwnd* ctrl)                  { this->_control.be(ctrl); return *this; }
	checkbox& be(const i_hwnd* parent, int controlId) { this->_control.be(parent, controlId); return *this; }

	checkbox& create(const i_hwnd* parent, int controlId, const wchar_t* caption, POINT pos, SIZE size) {
		this->_control.create(parent, controlId, caption, pos, size,
			L"Button", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX);
		return *this;
	}

	bool is_checked() const {
		return SendMessageW(this->hwnd(), BM_GETCHECK, 0, 0) == BST_CHECKED;
	}

	checkbox& set_check(bool checked) {
		SendMessageW(this->hwnd(), BM_SETCHECK,
			checked ? BST_CHECKED : BST_UNCHECKED, 0);
		return *this;
	}

	checkbox& set_check_and_trigger(bool checked) {
		this->set_check(checked);
		SendMessageW(GetParent(this->hwnd()), WM_COMMAND,
			MAKEWPARAM(GetDlgCtrlID(this->hwnd()), 0),
			reinterpret_cast<LPARAM>(this->hwnd()) ); // emulate user click
		return *this;
	}
};

}//namespace wl