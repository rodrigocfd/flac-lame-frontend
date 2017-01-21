/**
 * Part of WinLamb - Win32 API Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "internals/dialog.h"
#include "internals/i_control.h"
#include "internals/i_inventory.h"
#include "internals/i_threaded.h"
#include "internals/user_control.h"
#include "i_hwnd.h"

namespace wl {

namespace internals {
struct setup_dialog_control final : public setup_dialog { };
}//namespace internals


class dialog_control :
	public    i_hwnd,
	public    internals::i_inventory,
	public    internals::i_control<dialog_control>,
	protected internals::i_threaded
{
private:
	internals::dialog<internals::setup_dialog_control> _dialog;
	internals::user_control _control;
protected:
	internals::setup_dialog_control& setup;

public:
	dialog_control() :
		i_hwnd(_dialog.wnd()),
		i_inventory(_dialog.inventoryMsg),
		i_control(this),
		i_threaded(_dialog.threader),
		setup(_dialog.setup),
		_control(_dialog.wnd(), _dialog.inventoryMsg, TRUE) { }

	bool create(const i_hwnd* parent, int controlId, POINT position, SIZE size) {
		// Dialog styles to be set on the resource editor:
		// - Border: none
		// - Control: true
		// - Style: child
		// - Visible: true (otherwise will start invisible)
		// - Client Edge: true (if you want a border, will add WS_EX_CLIENTEDGE)

		if (!this->_dialog.basic_initial_checks()) return false;

		if (!CreateDialogParamW(
			parent->hinstance(), MAKEINTRESOURCE(setup.dialogId),
			parent->hwnd(), internals::dialog<internals::setup_dialog_control>::dialog_proc,
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