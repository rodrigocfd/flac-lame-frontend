/**
 * Part of WinLamb - Win32 API Lambda Library - More
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb-more
 */

#pragma once
#include <Windows.h>

namespace wl {
namespace base {

	template<typename wndT>
	class styles {
	private:
		wndT& _wnd;

	protected:
		explicit styles(wndT& target) : _wnd(target) { }
		explicit styles(wndT* target) : styles(*target) { }

		wndT& target() const { return this->_wnd; }

	public:
		wndT& set_style(bool addStyle, DWORD styleFlags) {
			return this->_change_style_flags(false, addStyle, styleFlags);
		}

		wndT& set_style_ex(bool addStyle, DWORD styleFlags) {
			return this->_change_style_flags(true, addStyle, styleFlags);
		}

		bool has_style(DWORD styleFlags) const {
			return (GetWindowLongPtrW(this->_wnd.hwnd(), GWL_STYLE) & styleFlags) != 0;
		}

		bool has_style_ex(DWORD styleFlags) const {
			return (GetWindowLongPtrW(this->_wnd.hwnd(), GWL_EXSTYLE) & styleFlags) != 0;
		}

	private:
		wndT& _change_style_flags(bool isEx, bool addStyle, DWORD styleFlags) {
			LONG_PTR curFlags = GetWindowLongPtrW(this->_wnd.hwnd(), isEx ? GWL_EXSTYLE : GWL_STYLE);
			if (addStyle) {
				curFlags |= static_cast<LONG_PTR>(styleFlags);
			} else {
				curFlags &= ~static_cast<LONG_PTR>(styleFlags);
			}
			SetWindowLongPtrW(this->_wnd.hwnd(), isEx ? GWL_EXSTYLE : GWL_STYLE, curFlags);
			return this->_wnd;
		}
	};

}//namespace base
}//namespace wl