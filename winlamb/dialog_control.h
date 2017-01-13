/**
 * Part of WinLamb - Win32 API Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "dialog.h"
#include "base_user_control.h"
#include "plus_on.h"

namespace wl {

struct setup_dialog_control final : public setup_dialog { };


class dialog_control : public plus_on {
protected:
	setup_dialog_control setup;
private:
	dialog _dialog;
	base_user_control _control;

public:
	dialog_control() :
		plus_on(_dialog.inventory), _dialog(setup),
		_control(_dialog.wnd(), _dialog.inventory, TRUE) { }

	HWND hwnd() const { return this->_dialog.wnd().hwnd(); }

	bool create(HWND hParent, int controlId, POINT position, SIZE size) {
		// Dialog styles to be set on the resource editor:
		// - Border: none
		// - Control: true
		// - Style: child
		// - Visible: true (otherwise will start invisible)
		// - Client Edge: true (if you want a border, will add WS_EX_CLIENTEDGE)

		if (!this->_dialog.basic_initial_checks()) return false;

		if (!CreateDialogParamW(
			reinterpret_cast<HINSTANCE>(GetWindowLongPtrW(hParent, GWLP_HINSTANCE)),
			MAKEINTRESOURCE(this->setup.dialogId),
			hParent, dialog::dialog_proc,
			reinterpret_cast<LPARAM>(&this->_dialog) ))
		{
			OutputDebugStringW(L"ERROR: control dialog not created, CreateDialogParam failed.\n");
			return false;
		}

		this->_check_bad_styles();
		SetWindowLongPtrW(this->hwnd(), GWLP_ID, controlId);
		SetWindowPos(this->hwnd(), nullptr,
			position.x, position.y,
			size.cx, size.cy, SWP_NOZORDER);
		return true;
	}

protected:
	void ui_thread(base_threaded::funcT func) const {
		this->_dialog.threaded.ui_thread(std::move(func));
	}

private:
	void _check_bad_styles() {
		DWORD style = static_cast<DWORD>(GetWindowLongPtrW(this->hwnd(), GWL_STYLE));
		if (!(style & DS_CONTROL)) {
			OutputDebugStringW(L"ERROR: control template doesn't have DS_CONTROL style.\n");
		}
		if (!(style & WS_CHILD)) {
			OutputDebugStringW(L"ERROR: control template doesn't have WS_CHILD style.\n");
		}
	}
};

}//namespace wl