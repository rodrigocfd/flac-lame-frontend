/**
 * Part of WinLamb - Win32 API Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "dialog.h"
#include "plus_text.h"
#include "plus_on.h"

namespace wl {

struct setup_dialog_modal final : public setup_dialog { };


class dialog_modal : protected plus_on, protected plus_text<dialog_modal> {
protected:
	setup_dialog_modal setup;
private:
	dialog _dialog;

public:
	dialog_modal() :
		plus_on(_dialog.inventory), plus_text(this), _dialog(setup)
	{
		this->on_message(WM_CLOSE, [&](const params& p)->INT_PTR {
			EndDialog(this->hwnd(), IDOK);
			return TRUE;
		});
	}

	HWND hwnd() const { return this->_dialog.wnd().hwnd(); }

	int show(HWND hParent) {
		if (!this->_dialog.basic_initial_checks()) return -1;
		return static_cast<int>(DialogBoxParamW(
			reinterpret_cast<HINSTANCE>(GetWindowLongPtrW(hParent, GWLP_HINSTANCE)),
			MAKEINTRESOURCE(this->setup.dialogId), hParent, dialog::dialog_proc,
			reinterpret_cast<LPARAM>(&this->_dialog)) );
	}

protected:
	void ui_thread(base_threaded::funcT func) const {
		this->_dialog.threaded.ui_thread(std::move(func));
	}

	void center_on_parent() const {
		RECT rc = { 0 }, rcParent = { 0 };
		GetWindowRect(this->hwnd(), &rc);
		GetWindowRect(GetParent(this->hwnd()), &rcParent); // both relative to screen
		SetWindowPos(this->hwnd(), nullptr,
			rcParent.left + (rcParent.right - rcParent.left)/2 - (rc.right - rc.left)/2,
			rcParent.top + (rcParent.bottom - rcParent.top)/2 - (rc.bottom - rc.top)/2,
			0, 0, SWP_NOZORDER | SWP_NOSIZE);
	}
};

}//namespace wl