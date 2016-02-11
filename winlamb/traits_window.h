/**
 * Part of WinLamb - Windows Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include <Windows.h>

namespace winlamb {

struct traits_window {
	typedef LRESULT ret_type;

	static void* get_instance_pointer(HWND hWnd, UINT msg, LPARAM lp)
	{
		void *p = nullptr;
		if (msg == WM_NCCREATE) {
			p = reinterpret_cast<void*>((reinterpret_cast<CREATESTRUCT*>(lp))->lpCreateParams);
			SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(p));
		} else {
			p = reinterpret_cast<void*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		}
		return p;
	}

	static LRESULT default_proc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
	{
		return DefWindowProc(hWnd, msg, wp, lp);
	}
};

}//namespace winlamb
