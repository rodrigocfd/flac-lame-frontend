/**
 * Part of WET - WinAPI Easy Toolkit
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wet
 */

#pragma once
#include "base_widget_text.h"

/**
 * base_wnd <-- base_widget <-- base_widget_text <-- checkbox
 */

namespace wet {

class checkbox final : public base_widget_text<checkbox> {
public:
	checkbox()          = default;
	checkbox(HWND hWnd) : base_widget_text(hWnd) { }

	checkbox& operator=(HWND hWnd) {
		return this->base_widget::operator=(hWnd);
	}

	checkbox& create(const base_wnd *parent, int controlId, const wchar_t* caption, POINT pos, SIZE size) {
		return this->base_widget::create(parent, controlId, caption, pos, size,
			L"Button", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX);
	}

	bool is_checked() const {
		return SendMessageW(this->base_wnd::hwnd(), BM_GETCHECK, 0, 0) == BST_CHECKED;
	}
	
	checkbox& set_check(bool checked) {
		SendMessageW(this->base_wnd::hwnd(), BM_SETCHECK,
			checked ? BST_CHECKED : BST_UNCHECKED, 0);
		return *this;
	}

	checkbox& set_check_and_trigger(bool checked) {
		this->set_check(checked);
		SendMessageW(this->base_wnd::parent().hwnd(), WM_COMMAND,
			MAKEWPARAM(this->base_wnd::id(), 0),
			reinterpret_cast<LPARAM>(this->base_wnd::hwnd()) ); // emulate user click
		return *this;
	}

private:
	base_widget::create;
};

}//namespace wet