/**
 * Part of WinLamb - Win32 API Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "base_wnd.h"

/**
 * base_wnd <-- base_native_control
 */

namespace wl {
namespace base {

	class native_control : virtual public wnd {
	protected:
		native_control() = default;

		void assign(const wnd* parent, int controlId) {
			this->wnd::_hWnd = GetDlgItem(parent->hwnd(), controlId);
		}

		bool create(const wnd* parent, int controlId, const wchar_t* caption,
			POINT pos, SIZE size, const wchar_t* className,
			DWORD styles = (WS_CHILD | WS_VISIBLE), DWORD exStyles = 0)
		{
			if (this->hwnd()) {
				OutputDebugStringW(L"ERROR: native control already created.\n");
				return false;
			}

			this->wnd::_hWnd = CreateWindowExW(exStyles, className, caption, styles,
				pos.x, pos.y, size.cx, size.cy, parent->hwnd(),
				reinterpret_cast<HMENU>(static_cast<UINT_PTR>(controlId)),
				reinterpret_cast<HINSTANCE>(GetWindowLongPtrW(parent->hwnd(), GWLP_HINSTANCE)),
				nullptr);

			return this->hwnd() != nullptr;
		}

	public:
		int control_id() const {
			return GetDlgCtrlID(this->hwnd());
		}
	};

}//namespace base
}//namespace wl