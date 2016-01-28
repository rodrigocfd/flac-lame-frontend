/**
 * Part of WinLamb - Windows Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "window_dialog.h"

 /**
  * window_dialog_main
  *  window_dialog
  *   window_thread<window_dialog_traits>
  *    window_proc<window_dialog_traits>
  *     window
  */

namespace winlamb {

class window_dialog_main : public window_dialog {
public:
	struct setup_type {
		int dialogId;
		int iconId;
		int accelTableId;
		setup_type() : dialogId(0), iconId(0), accelTableId(0) { }
	};

	setup_type setup;
	virtual ~window_dialog_main() = default;
	window_dialog_main& operator=(const window_dialog_main&) = delete;

	window_dialog_main()
	{
		on_message(WM_CLOSE, [this](WPARAM wp, LPARAM lp)->INT_PTR {
			DestroyWindow(hwnd());
			return TRUE;
		});
		on_message(WM_NCDESTROY, [](WPARAM wp, LPARAM lp)->INT_PTR {
			PostQuitMessage(0);
			return TRUE;
		});
	}

	int run(HINSTANCE hInst, int cmdShow)
	{
		InitCommonControls();
		
		if (!CreateDialogParam(hInst, MAKEINTRESOURCE(setup.dialogId), nullptr,
			window_proc::_proc, reinterpret_cast<LPARAM>(this))) return -1; // _hwnd member is set on first message processing

		if (setup.iconId) {
			SendMessage(hwnd(), WM_SETICON, ICON_SMALL,
				reinterpret_cast<LPARAM>(reinterpret_cast<HICON>(LoadImage(hInst,
				MAKEINTRESOURCE(setup.iconId), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR))));
			SendMessage(hwnd(), WM_SETICON, ICON_BIG,
				reinterpret_cast<LPARAM>(reinterpret_cast<HICON>(LoadImage(hInst,
				MAKEINTRESOURCE(setup.iconId), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR))));
		}

		HACCEL hAccel = nullptr;
		if (setup.accelTableId) {
			hAccel = LoadAccelerators(hInst, MAKEINTRESOURCE(setup.accelTableId));
		}

		ShowWindow(hwnd(), cmdShow);

		MSG  msg = { 0 };
		BOOL ret = 0;
		while ((ret = GetMessage(&msg, nullptr, 0, 0)) != 0) {
			if (ret == -1) return -1;
			if ( (hAccel && TranslateAccelerator(hwnd(), hAccel, &msg)) ||
				IsDialogMessage(hwnd(), &msg) ) continue;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		return static_cast<int>(msg.wParam); // this can be used as program return value
	}

private:
	window_proc::_proc;
};

}//namespace winlamb
