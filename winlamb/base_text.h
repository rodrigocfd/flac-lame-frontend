/**
 * Part of WinLamb - Win32 API Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include <string>
#include "base_wnd.h"

/**
 * base_wnd <-- base_text
 */

namespace wl {
namespace base {

	template<typename wndT>
	class text : virtual public wnd {
	protected:
		text() = default;

	public:
		wndT& set_text(const wchar_t* text) {
			SetWindowTextW(this->hwnd(), text);
			return *static_cast<wndT*>(this);
		}

		wndT& set_text(const std::wstring& text) {
			return this->set_text(text.c_str());
		}

		std::wstring get_text() const {
			std::wstring buf;
			int len = GetWindowTextLengthW(this->hwnd());
			if (len) {
				buf.resize(len + 1, L'\0');
				GetWindowTextW(this->hwnd(), &buf[0], len + 1);
				buf.resize(len);
			}
			return buf;
		}
	};

}//namespace base
}//namespace wl