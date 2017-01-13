/**
 * Part of WinLamb - Win32 API Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "dialog.h"
#include "base_loop.h"
#include "base_run.h"
#include "plus_on.h"
#include "plus_text.h"

namespace wl {

struct setup_dialog_main final : public setup_dialog {
	int iconId;
	int accelTableId;
	setup_dialog_main() : iconId(0), accelTableId(0) { }
};


class dialog_main : protected plus_on, protected plus_text<dialog_main> {
protected:
	setup_dialog_main setup;
private:
	dialog _dialog;

public:
	dialog_main() :
		plus_on(_dialog.inventory), plus_text(this), _dialog(setup)
	{
		this->on_message(WM_CLOSE, [&](const params& p)->INT_PTR {
			DestroyWindow(this->hwnd());
			return TRUE;
		});
		this->on_message(WM_NCDESTROY, [](const params& p)->INT_PTR {
			PostQuitMessage(0);
			return TRUE;
		});
	}

	HWND hwnd() const { return this->_dialog.wnd().hwnd(); }

	int run(HINSTANCE hInst, int cmdShow) {
		InitCommonControls();
		if (!this->_dialog.basic_initial_checks()) return -1;

		HWND hwndRet = CreateDialogParamW(hInst, MAKEINTRESOURCE(this->setup.dialogId),
			nullptr, dialog::dialog_proc,
			reinterpret_cast<LPARAM>(&this->_dialog));
		if (!hwndRet) {
			OutputDebugStringW(L"ERROR: main dialog not created, CreateDialogParam failed.\n");
			return -1;
		}

		HACCEL hAccel = nullptr;
		if (this->setup.accelTableId) {
			hAccel = LoadAccelerators(hInst, MAKEINTRESOURCE(this->setup.accelTableId));
		}

		this->_set_icon(hInst);
		ShowWindow(this->hwnd(), cmdShow);
		return base_loop::msg_loop(this->hwnd(), hAccel); // this can be used as program return value
	}

protected:
	void ui_thread(base_threaded::funcT func) const {
		this->_dialog.threaded.ui_thread(std::move(func));
	}

private:
	void _set_icon(HINSTANCE hInst) const {
		if (this->setup.iconId) {
			SendMessageW(this->hwnd(), WM_SETICON, ICON_SMALL,
				reinterpret_cast<LPARAM>(reinterpret_cast<HICON>(LoadImage(hInst,
					MAKEINTRESOURCE(this->setup.iconId),
					IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR))));
			SendMessageW(this->hwnd(), WM_SETICON, ICON_BIG,
				reinterpret_cast<LPARAM>(reinterpret_cast<HICON>(LoadImage(hInst,
					MAKEINTRESOURCE(this->setup.iconId),
					IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR))));
		}
	}
};

}//namespace wl