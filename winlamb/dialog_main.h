/**
 * Part of WinLamb - Win32 API Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "base_dialog.h"
#include "base_loop.h"
#include "base_text.h"
#include "base_run.h"

/**
 *                              +--------------------- msgs_[any] <------------------------+
 *             +-- base_msgs <--+                                                          |
 *             |                +-- base_threaded <--+                                     |
 *             |                                     +-- base_dialog <--+                  +-- [user]
 *             +------------ base_wheel <------------+                  |                  |
 * base_wnd <--+                                                        |                  |
 *             +--------------------- base_loop <-----------------------+-- dialog_main <--+
 *             |                                                        |
 *             +--------------------- base_text <-----------------------+
 */

namespace wl {

// Inherit from this class to have a dialog as the main window of your application.
class dialog_main :
	public    base::dialog,
	protected base::loop,
	protected base::text<dialog_main>
{
public:
	struct setup_vars final : public base::dialog::setup_vars {
		int iconId;
		int accelTableId;
		setup_vars() : iconId(0), accelTableId(0) { }
	};

protected:
	setup_vars setup;

	dialog_main(size_t msgsReserve = 0) : dialog(msgsReserve + 2) {
		this->on_message(WM_CLOSE, [&](const params&)->INT_PTR {
			DestroyWindow(this->hwnd());
			return TRUE;
		});
		this->on_message(WM_NCDESTROY, [](const params&)->INT_PTR {
			PostQuitMessage(0);
			return TRUE;
		});
	}

public:
	int run(HINSTANCE hInst, int cmdShow) {
		InitCommonControls();
		if (!this->dialog::_basic_initial_checks(this->setup)) return -1;

		HWND hwndRet = CreateDialogParamW(hInst, MAKEINTRESOURCE(this->setup.dialogId),
			nullptr, base::dialog::_dialog_proc, reinterpret_cast<LPARAM>(this));
		if (!hwndRet) {
			OutputDebugStringW(L"ERROR: main dialog not created, CreateDialogParam failed.\n");
			return -1;
		}

		HACCEL hAccel = nullptr;
		if (this->setup.accelTableId) {
			hAccel = LoadAcceleratorsW(hInst, MAKEINTRESOURCE(this->setup.accelTableId));
		}

		this->_set_icon(hInst);
		ShowWindow(this->hwnd(), cmdShow);
		return this->loop::_msg_loop(hAccel); // this can be used as program return value
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