/**
 * Part of WinLamb - Windows Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "window.h"

 /**
  * window_main
  *  window
  *   threaded<traits_window>
  *    proc<traits_window>
  *     handle
  */

namespace winlamb {

struct setup_window_main final : public setup_window {
	HACCEL accelTable;
	setup_window_main() : accelTable(nullptr) { }
};


class window_main : public window<setup_window_main> {
public:
	virtual ~window_main() = default;
	window_main& operator=(const window_main&) = delete;

	window_main()
	{
		setup.wcx.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
		setup.wcx.style = CS_DBLCLKS;
		setup.style = WS_CAPTION | WS_SYSMENU | WS_CLIPCHILDREN | WS_BORDER;

		on_message(WM_NCDESTROY, [](WPARAM wp, LPARAM lp)->LRESULT {
			PostQuitMessage(0);
			return 0;
		});
	}

	int run(HINSTANCE hInst, int cmdShow)
	{
		InitCommonControls();
		if (!create(nullptr, hInst)) return -1;

		ShowWindow(hwnd(), cmdShow);
		UpdateWindow(hwnd());

		MSG  msg = { 0 };
		BOOL ret = 0;
		while ((ret = GetMessage(&msg, nullptr, 0, 0)) != 0) {
			if (ret == -1) return -1;
			if ( (setup.accelTable && TranslateAccelerator(hwnd(), setup.accelTable, &msg)) ||
				IsDialogMessage(hwnd(), &msg) ) continue;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		return static_cast<int>(msg.wParam); // this can be used as program return value
	}

private:
	window::create;
};

}//namespace winlamb