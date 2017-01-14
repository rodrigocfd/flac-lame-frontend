/**
 * Part of WinLamb - Win32 API Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "base_wnd.h"
#include "i_hwnd.h"

namespace wl {

class base_native_control final {
private:
	base_wnd _wnd;

public:
	const base_wnd& wnd() const { return this->_wnd; }

	void be(const i_hwnd* ctrl) {
		this->_wnd = ctrl->hwnd();
	}

	void be(const i_hwnd* parent, int controlId) {
		this->_wnd = GetDlgItem(parent->hwnd(), controlId);
	}

	bool create(const i_hwnd* parent, int controlId, const wchar_t* caption,
		POINT pos, SIZE size, const wchar_t* className,
		DWORD styles = (WS_CHILD | WS_VISIBLE), DWORD exStyles = 0)
	{
		if (this->_wnd.hwnd()) {
			OutputDebugStringW(L"ERROR: native control already created.\n");
			return false;
		}

		this->_wnd = CreateWindowExW(exStyles, className, caption, styles,
			pos.x, pos.y, size.cx, size.cy, parent->hwnd(),
			reinterpret_cast<HMENU>(static_cast<UINT_PTR>(controlId)),
			parent->hinstance(), nullptr);

		return this->_wnd.hwnd() != nullptr;
	}
};

}//namespace wl