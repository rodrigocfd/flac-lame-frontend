/**
 * Part of WET - WinAPI Easy Toolkit
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wet
 */

#pragma once
#include "dialog.h"
#include "base_wnd_toplevel.h"

/**
 *             +-- base_wnd_loop <-- base_wnd_thread <-- dialog --+
 * base_wnd <--+                                                  +-- dialog_modal
 *             +-------------- base_wnd_toplevel <----------------+
 */

namespace wet {

class dialog_modal :
	public dialog<>,
	public base_wnd_toplevel
{
protected:
	dialog_modal() = default;

public:
	virtual ~dialog_modal() = default;
	dialog_modal& operator=(const dialog_modal&) = delete;

	int show(HWND hParent) {
		if (!this->dialog::_basic_check()) return -1;

		return static_cast<int>(DialogBoxParamW(
			reinterpret_cast<HINSTANCE>(GetWindowLongPtrW(hParent, GWLP_HINSTANCE)),
			MAKEINTRESOURCE(this->dialog::setup.dialogId), hParent, dialog::_proc,
			reinterpret_cast<LPARAM>(static_cast<dialog*>(this)) )); // _hwnd member is set on first message processing
	}

	int show(const base_wnd* parent) {
		return this->show(parent->hwnd());
	}

protected:
	INT_PTR def_proc(params p) const {
		switch (p.message) {
		case WM_CLOSE:
			EndDialog(this->base_wnd::hwnd(), IDOK);
			return TRUE;
		}
		return dialog::def_proc(p);
	}

	void center_on_parent() const {
		RECT rc = { 0 }, rcParent = { 0 };
		GetWindowRect(this->base_wnd::hwnd(), &rc);
		GetWindowRect(this->base_wnd::parent().hwnd(), &rcParent); // both relative to screen
		SetWindowPos(this->base_wnd::hwnd(), nullptr,
			rcParent.left + (rcParent.right - rcParent.left)/2 - (rc.right - rc.left)/2,
			rcParent.top + (rcParent.bottom - rcParent.top)/2 - (rc.bottom - rc.top)/2,
			0, 0, SWP_NOZORDER | SWP_NOSIZE);
	}

private:
	base_wnd_loop::_msg_loop;
	dialog::_basic_check;
	dialog::_proc;
	dialog::def_proc;
};

}//namespace wet