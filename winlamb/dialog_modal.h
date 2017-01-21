/**
 * Part of WinLamb - Win32 API Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "internals/dialog.h"
#include "internals/i_inventory.h"
#include "internals/i_text.h"
#include "internals/i_threaded.h"
#include "i_hwnd.h"

namespace wl {

namespace internals {
struct setup_dialog_modal final : public setup_dialog { };
}//namespace internals


class dialog_modal :
	public    i_hwnd,
	protected internals::i_inventory,
	protected internals::i_text<dialog_modal>,
	protected internals::i_threaded
{
private:
	internals::dialog<internals::setup_dialog_modal> _dialog;
protected:
	internals::setup_dialog_modal& setup;

public:
	dialog_modal() :
		i_hwnd(_dialog.wnd()),
		i_inventory(_dialog.inventoryMsg),
		i_text(this),
		i_threaded(_dialog.threader),
		setup(_dialog.setup)
	{
		this->on_message(WM_CLOSE, [&](const params&)->INT_PTR {
			EndDialog(this->hwnd(), IDOK);
			return TRUE;
		});
	}

	int show(const i_hwnd* parent) {
		if (!this->_dialog.basic_initial_checks()) return -1;
		return static_cast<int>(DialogBoxParamW(
			parent->hinstance(), MAKEINTRESOURCE(this->setup.dialogId),
			parent->hwnd(), internals::dialog<internals::setup_dialog_modal>::dialog_proc,
			reinterpret_cast<LPARAM>(&this->_dialog)) );
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