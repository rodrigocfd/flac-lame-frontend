/**
 * Part of WinLamb - Win32 API Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include <Windows.h>

namespace wl {
namespace internals {

class loop final {
protected:
	loop() = default;

public:
	static int msg_loop(HWND hWnd, HACCEL hAccel = nullptr) {
		MSG  msg = { 0 };
		BOOL ret = 0;
		while ((ret = GetMessageW(&msg, nullptr, 0, 0)) != 0) {
			if (ret == -1) return -1;
			if ((hAccel && TranslateAcceleratorW(hWnd, hAccel, &msg)) ||
				IsDialogMessageW(hWnd, &msg) ) continue;
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
		return static_cast<int>(msg.wParam); // this can be used as program return value
	}
};

}//namespace internals
}//namespace wl