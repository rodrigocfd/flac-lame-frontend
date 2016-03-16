/**
 * Part of WinLamb - Windows Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "font.h"

namespace winlamb {

struct traits_dialog final {
	typedef INT_PTR ret_type;

	template<typename instT>
	static instT* get_instance_pointer(HWND hWnd, UINT msg, LPARAM lp)
	{
		instT *p = nullptr;
		if (msg == WM_INITDIALOG) {
			p = reinterpret_cast<instT*>(lp);
			SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(p));
			font::set_ui_on_children(hWnd); // if user creates controls manually, font must be set manually on them
		} else {
			p = reinterpret_cast<instT*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		}
		return p;
	}

	static INT_PTR default_proc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
	{
		return FALSE;
	}
};

}//namespace winlamb
