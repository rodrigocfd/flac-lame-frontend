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
#include "internals/loop.h"
#include "internals/run.h"
#include "i_hwnd.h"

namespace wl {

namespace internals {
struct setup_dialog_main final : public setup_dialog {
	int iconId;
	int accelTableId;
	setup_dialog_main() : iconId(0), accelTableId(0) { }
};
}//namespace internals


class dialog_main :
	public    i_hwnd,
	protected internals::i_inventory,
	protected internals::i_text<dialog_main>,
	protected internals::i_threaded
{
private:
	internals::dialog<internals::setup_dialog_main> _dialog;
protected:
	internals::setup_dialog_main& setup;

public:
	dialog_main() :
		i_hwnd(_dialog.wnd()),
		i_inventory(_dialog.inventoryMsg),
		i_text(this),
		i_threaded(_dialog.threader),
		setup(_dialog.setup)
	{
		this->on_message(WM_CLOSE, [&](const params&)->INT_PTR {
			DestroyWindow(this->hwnd());
			return TRUE;
		});
		this->on_message(WM_NCDESTROY, [](const params&)->INT_PTR {
			PostQuitMessage(0);
			return TRUE;
		});
	}

	int run(HINSTANCE hInst, int cmdShow) {
		InitCommonControls();
		if (!this->_dialog.basic_initial_checks()) return -1;

		HWND hwndRet = CreateDialogParamW(hInst, MAKEINTRESOURCE(this->setup.dialogId),
			nullptr, internals::dialog<internals::setup_dialog_main>::dialog_proc,
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
		return internals::loop::msg_loop(this->hwnd(), hAccel); // this can be used as program return value
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