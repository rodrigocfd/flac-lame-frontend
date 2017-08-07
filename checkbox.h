/**
 * Part of WinLamb - Win32 API Lambda Library - More
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb-more
 */

#pragma once
#include "../winlamb/native_control.h"
#include "base_styles.h"
#include "textable.h"

/**
 *             +-- native_control <--+
 * base_wnd <--+                     +-- checkbox
 *             +----- textable <-----+
 */

namespace wl {

// Wrapper to checkbox control.
class checkbox final :
	public native_control,
	public textable<checkbox>
{
public:
	class styler : public base::styles<checkbox> {
	public:
		styler(checkbox* pCheckbox) : styles(pCheckbox) { }
	};

	styler style;

	checkbox() : style(this) { }

	checkbox& assign(HWND hParent, int controlId) {
		this->native_control::assign(hParent, controlId);
		return *this;
	}

	checkbox& assign(const base::wnd* parent, int controlId) {
		return this->assign(parent->hwnd(), controlId);
	}

	checkbox& create(HWND hParent, int controlId,
		const wchar_t* caption, POINT pos, SIZE size)
	{
		this->native_control::create(hParent, controlId, caption, pos, size,
			L"Button", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX);
		return *this;
	}

	checkbox& create(const base::wnd* parent, int controlId,
		const wchar_t* caption, POINT pos, SIZE size)
	{
		return this->create(parent->hwnd(), controlId, caption, pos, size);
	}

	checkbox& focus() {
		SetFocus(this->hwnd());
		return *this;
	}

	checkbox& enable(bool doEnable) {
		EnableWindow(this->hwnd(), doEnable);
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