/**
 * Part of WinLamb - Windows Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "window_basic.h"

 /**
  * window_basic_main
  *  window_basic
  *   window_thread<window_basic_traits>
  *    window_proc<window_basic_traits>
  *     window
  */

namespace winlamb {

class window_basic_main : public window_basic {
public:
	virtual ~window_basic_main() = default;
	window_basic_main& operator=(const window_basic_main&) = delete;

	window_basic_main()
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
	window_basic::create;
};

}//namespace winlamb
