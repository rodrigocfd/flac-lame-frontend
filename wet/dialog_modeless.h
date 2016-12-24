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
* base_wnd <--+                                                  +-- dialog_modeless
*             +-------------- base_wnd_toplevel <----------------+
 */

namespace wet {

class dialog_modeless :
	public dialog<>,
	public base_wnd_toplevel
{
protected:
	dialog_modeless() = default;

	INT_PTR def_proc(params p) const {
		switch (p.message) {
		case WM_CLOSE:
			DestroyWindow(this->base_wnd::hwnd());
			return TRUE;
		}
		return dialog::def_proc(p);
	}

public:
	virtual ~dialog_modeless() = default;
	dialog_modeless& operator=(const dialog_modeless&) = delete;

	void show(HWND hParent) {
		// To have dialog messages processed, pass HWND to parent's modeless_add().
		if (!this->dialog::_basic_check()) return;
		HINSTANCE hInst = reinterpret_cast<HINSTANCE>(GetWindowLongPtrW(hParent, GWLP_HINSTANCE));

		if (!CreateDialogParamW(hInst, MAKEINTRESOURCE(this->dialog::setup.dialogId),
			hParent, dialog::_proc,
			reinterpret_cast<LPARAM>(static_cast<dialog*>(this))) ) // _hWnd member is set on first message processing
		{
			DBG(L"ERROR: modeless dialog not created, CreateDialogParam failed.\n");
			return;
		}

		ShowWindow(this->base_wnd::hwnd(), SW_SHOW);
	}

	void show(const base_wnd* parent) {
		return this->show(parent->hwnd());
	}

private:
	base_wnd_loop::_msg_loop;
	dialog::_basic_check;
	dialog::_proc;
	dialog::def_proc;
};

}//namespace wet