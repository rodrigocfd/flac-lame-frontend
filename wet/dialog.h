/**
 * Part of WET - WinAPI Easy Toolkit
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wet
 */

#pragma once
#include "base_wnd_thread.h"
#include "font.h"
#include "wheel_hover.h"

/**
 * base_wnd <-- base_wnd_loop <-- base_wnd_thread <-- dialog
 */

namespace wet {

struct setup_dialog {
	int dialogId;
	setup_dialog() : dialogId(0) { }
};


template<typename setupT = setup_dialog>
class dialog : public base_wnd_thread {
public:
	virtual ~dialog() = default;

protected:
	dialog() = default;
	setupT setup;
	virtual INT_PTR proc(params p) = 0;

	INT_PTR def_proc(params p) const {
		switch (p.message) {
		case base_wnd_thread::WM_THREAD_MESSAGE:
			this->base_wnd_thread::_process_thread(p);
			return TRUE;
		}
		return FALSE;
	}

	bool _basic_check() const {
		if (this->base_wnd::hwnd()) {
			DBG(L"ERROR: tried to create dialog twice.\n");
			return false;
		}
		if (!this->setup.dialogId) {
			DBG(L"ERROR: dialog not created, no resource ID given.\n");
			return false;
		}
		return true;
	}

	static INT_PTR CALLBACK _proc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
		dialog* pSelf = nullptr;
		INT_PTR ret = FALSE;

		if (msg == WM_INITDIALOG) {
			pSelf = reinterpret_cast<dialog*>(lp);
			SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pSelf));
			font::set_ui_on_children(hWnd); // if user creates controls manually, font must be set manually on them
			pSelf->base_wnd::operator=(hWnd); // store HWND
		} else {
			pSelf = reinterpret_cast<dialog*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
		}

		if (pSelf) {
			ret = pSelf->proc({ msg, wp, lp });
		}

		if (msg == WM_INITDIALOG) {
			wheel_hover::apply_behavior(pSelf->base_wnd::hwnd());
		} else if (msg == WM_NCDESTROY) { // cleanup
			SetWindowLongPtrW(hWnd, GWLP_USERDATA, 0);
			if (pSelf) {
				pSelf->base_wnd::operator=(nullptr);
			}
		}

		return ret;
	}

private:
	base_wnd::_text;
	base_wnd_thread::_process_thread;
};

}//namespace wet