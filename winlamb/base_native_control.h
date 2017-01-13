/**
 * Part of WinLamb - Win32 API Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "base_wnd.h"

namespace wl {

class base_native_control final {
private:
	base_wnd _wnd;

public:
	HWND      hwnd() const      { return this->_wnd.hwnd(); }
	HINSTANCE hinstance() const { return this->_wnd.hinstance(); }

	void be(HWND hWnd) {
		this->_wnd = hWnd;
	}

	void be(HWND hParent, int controlId) {
		this->be(GetDlgItem(hParent, controlId));
	}

	bool create(HWND hParent, int controlId, const wchar_t* caption,
		POINT pos, SIZE size, const wchar_t* className,
		DWORD styles = (WS_CHILD | WS_VISIBLE), DWORD exStyles = 0)
	{
		if (this->hwnd()) {
			OutputDebugStringW(L"ERROR: native control already created.\n");
			return false;
		}

		this->_wnd = CreateWindowExW(exStyles, className, caption, styles,
			pos.x, pos.y, size.cx, size.cy, hParent,
			reinterpret_cast<HMENU>(static_cast<UINT_PTR>(controlId)),
			reinterpret_cast<HINSTANCE>(GetWindowLongPtrW(hParent, GWLP_HINSTANCE)),
			nullptr);

		return this->hwnd() != nullptr;
	}
};

}//namespace wl