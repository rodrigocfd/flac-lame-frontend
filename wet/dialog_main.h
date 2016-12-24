/**
 * Part of WET - WinAPI Easy Toolkit
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wet
 */

#pragma once
#include "dialog.h"
#include "base_wnd_main.h"
#include "base_wnd_toplevel.h"

/**
 *             +-- base_wnd_loop <-- base_wnd_thread <-- dialog --+
 *             |                                                  |
 * base_wnd <--+---------------- base_wnd_main <------------------+-- dialog_main
 *             |                                                  |
 *             +-------------- base_wnd_toplevel <----------------+
 */

namespace wet {

struct setup_dialog_main final : public setup_dialog {
	int iconId;
	int accelTableId;
	setup_dialog_main() : iconId(0), accelTableId(0) { }
};


class dialog_main :
	public dialog<setup_dialog_main>,
	public base_wnd_main,
	public base_wnd_toplevel
{
protected:
	dialog_main() = default;

public:
	virtual ~dialog_main() = default;
	dialog_main& operator=(const dialog_main&) = delete;

	virtual int run(HINSTANCE hInst, int cmdShow) override {
		InitCommonControls();
		if (!this->dialog::_basic_check()) return -1;

		HWND hwndRet = CreateDialogParamW(hInst, MAKEINTRESOURCE(this->dialog::setup.dialogId),
			nullptr, dialog::_proc,
			reinterpret_cast<LPARAM>(static_cast<dialog*>(this)) ); // _hwnd member is set on first message processing
		if (!hwndRet) {
			DBG(L"ERROR: main dialog not created, CreateDialogParam failed.\n");
			return -1;
		}

		HACCEL hAccel = nullptr;
		if (this->dialog::setup.accelTableId) {
			hAccel = LoadAccelerators(hInst, MAKEINTRESOURCE(this->dialog::setup.accelTableId));
		}

		this->_set_icon(hInst);
		ShowWindow(this->base_wnd::hwnd(), cmdShow);
		return this->base_wnd_loop::_msg_loop(hAccel); // this can be used as program return value
	}

protected:
	INT_PTR def_proc(params p) const {
		switch (p.message) {
		case WM_CLOSE:
			DestroyWindow(this->base_wnd::hwnd());
			return TRUE;
		case WM_NCDESTROY:
			PostQuitMessage(0);
			return TRUE;
		}
		return dialog::def_proc(p);
	}

private:
	void _set_icon(HINSTANCE hInst) const {
		if (this->dialog::setup.iconId) {
			SendMessageW(this->base_wnd::hwnd(), WM_SETICON, ICON_SMALL,
				reinterpret_cast<LPARAM>(reinterpret_cast<HICON>(LoadImage(hInst,
					MAKEINTRESOURCE(this->dialog::setup.iconId),
					IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR))));
			SendMessageW(this->base_wnd::hwnd(), WM_SETICON, ICON_BIG,
				reinterpret_cast<LPARAM>(reinterpret_cast<HICON>(LoadImage(hInst,
					MAKEINTRESOURCE(this->dialog::setup.iconId),
					IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR))));
		}
	}

	base_wnd_loop::_msg_loop;
	dialog::_basic_check;
	dialog::_proc;
	dialog::def_proc;
};

}//namespace wet