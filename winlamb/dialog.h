/**
 * Part of WinLamb - Win32 API Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "base_inventory.h"
#include "base_threaded.h"
#include "base_wheel.h"
#include "font.h"

namespace wl {

struct setup_dialog {
	int dialogId;
	setup_dialog() : dialogId(0) { }
};


class dialog final {
private:
	base_wnd _wnd;
public:
	base_inventory inventory;
	base_threaded threaded;
private:
	setup_dialog& _setup;

public:
	dialog(setup_dialog& setup) :
		threaded(_wnd, inventory, TRUE), _setup(setup) { }

	const base_wnd& wnd() const { return this->_wnd; }

	bool basic_initial_checks() const {
		if (this->_wnd.hwnd()) {
			OutputDebugStringW(L"ERROR: tried to create dialog twice.\n");
			return false;
		}
		if (!this->_setup.dialogId) {
			OutputDebugStringW(L"ERROR: dialog not created, no resource ID given.\n");
			return false;
		}
		return true;
	}

	static INT_PTR CALLBACK dialog_proc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp) {
		dialog* pSelf = nullptr;
		INT_PTR ret = FALSE;

		if (msg == WM_INITDIALOG) {
			pSelf = reinterpret_cast<dialog*>(lp);
			SetWindowLongPtrW(hDlg, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pSelf));
			font::set_ui_on_children(hDlg); // if user creates controls manually, font must be set manually on them
			pSelf->_wnd = hDlg; // store HWND
		} else {
			pSelf = reinterpret_cast<dialog*>(GetWindowLongPtrW(hDlg, GWLP_USERDATA));
		}

		if (pSelf) {
			params p = {msg, wp, lp};
			base_inventory::funcT* pFunc = pSelf->inventory.find_func(p);
			if (pFunc) ret = (*pFunc)(p);
		}

		if (msg == WM_INITDIALOG) {
			base_wheel::apply_behavior(hDlg);
		} else if (msg == WM_NCDESTROY) { // cleanup
			SetWindowLongPtrW(hDlg, GWLP_USERDATA, 0);
			if (pSelf) {
				pSelf->_wnd = nullptr;
			}
		}

		return ret;
	}
};

}//namespace wl