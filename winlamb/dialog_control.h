/**
 * Part of WinLamb - Win32 API Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "dialog.h"
#include "base_user_control.h"
#include "i_control.h"
#include "i_hwnd.h"
#include "i_inventory.h"

namespace wl {

struct setup_dialog_control final : public setup_dialog { };


class dialog_control :
	public i_hwnd,
	public i_inventory,
	public i_control<dialog_control>
{
protected:
	setup_dialog_control setup;
private:
	dialog _dialog;
	base_user_control _control;

public:
	dialog_control() :
		i_hwnd(_dialog.wnd()), i_inventory(_dialog.inventory), i_control(this), _dialog(setup),
		_control(_dialog.wnd(), _dialog.inventory, TRUE) { }

	bool create(const i_hwnd* parent, int controlId, POINT position, SIZE size) {
		// Dialog styles to be set on the resource editor:
		// - Border: none
		// - Control: true
		// - Style: child
		// - Visible: true (otherwise will start invisible)
		// - Client Edge: true (if you want a border, will add WS_EX_CLIENTEDGE)

		if (!this->_dialog.basic_initial_checks()) return false;

		if (!CreateDialogParamW(
			parent->hinstance(), MAKEINTRESOURCE(this->setup.dialogId),
			parent->hwnd(), dialog::dialog_proc,
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