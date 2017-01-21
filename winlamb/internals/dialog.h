/**
 * Part of WinLamb - Win32 API Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "inventory.h"
#include "threaded.h"
#include "wheel_hover.h"
#include "../font.h"

namespace wl {
namespace internals {

struct setup_dialog {
	int dialogId;
	setup_dialog() : dialogId(0) { }
};


template<typename setupT>
class dialog final {
private:
	base_wnd _wnd;
public:
	inventory inventoryMsg;
	threaded  threader;
	setupT    setup;

	dialog() : threader(_wnd, inventoryMsg, TRUE) { }

	const base_wnd& wnd() const { return this->_wnd; }

	bool basic_initial_checks() const {
		if (this->_wnd.hwnd()) {
			OutputDebugStringW(L"ERROR: tried to create dialog twice.\n");
			return false;
		}
		if (!this->setup.dialogId) {
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
			inventory::funcT* pFunc = pSelf->inventoryMsg.find_func(p);
			if (pFunc) ret = (*pFunc)(p);
		}

		if (msg == WM_INITDIALOG) {
			wheel_hover::apply_behavior(hDlg);
		} else if (msg == WM_NCDESTROY) { // cleanup
			SetWindowLongPtrW(hDlg, GWLP_USERDATA, 0);
			if (pSelf) {
				pSelf->_wnd = nullptr;
			}
		}

		return ret;
	}
};

}//namespace internals
}//namespace wl