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

class checkbox final : public plus_control<checkbox>, public plus_text<checkbox> {
private:
	base_native_control _control;

public:
	checkbox() : plus_control(this), plus_text(this) { }

	HWND      hwnd() const                    { return this->_control.hwnd(); }
	checkbox& be(HWND hWnd)                   { this->_control.be(hWnd); return *this; }
	checkbox& be(HWND hParent, int controlId) { this->_control.be(hParent, controlId); return *this; }

	checkbox& create(HWND hParent, int controlId, const wchar_t* caption, POINT pos, SIZE size) {
		this->_control.create(hParent, controlId, caption, pos, size,
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