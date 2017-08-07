/**
 * Part of WinLamb - Win32 API Lambda Library - More
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb-more
 */

#pragma once
#include <string>
#include "../winlamb/base_wnd.h"

/**
 * base_wnd <-- textable
 */

namespace wl {

// Adds get/set text methods to a window.
template<typename wndT>
class textable : virtual public base::wnd {
protected:
	textable() = default;

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

}//namespace wl