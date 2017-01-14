/**
 * Part of WinLamb - Win32 API Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "dialog.h"
#include "i_hwnd.h"
#include "i_inventory.h"
#include "i_text.h"

namespace wl {

struct setup_dialog_modal final : public setup_dialog { };


class dialog_modal :
	public    i_hwnd,
	protected i_inventory,
	protected i_text<dialog_modal>
{
protected:
	setup_dialog_modal setup;
private:
	dialog _dialog;

public:
	dialog_modal() :
		i_hwnd(_dialog.wnd()), i_inventory(_dialog.inventory), i_text(this), _dialog(setup)
	{
		this->on_message(WM_CLOSE, [&](const params& p)->INT_PTR {
			EndDialog(this->hwnd(), IDOK);
			return TRUE;
		});
	}

	int show(const i_hwnd* parent) {
		if (!this->_dialog.basic_initial_checks()) return -1;
		return static_cast<int>(DialogBoxParamW(
			parent->hinstance(), MAKEINTRESOURCE(this->setup.dialogId),
			parent->hwnd(), dialog::dialog_proc,
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