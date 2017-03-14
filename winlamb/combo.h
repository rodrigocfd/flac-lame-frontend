/**
 * Part of WinLamb - Win32 API Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "base_native_control.h"
#include "str.h"

/**
 * base_wnd <-- base_native_control <-- combo
 */

namespace wl {

// Wrapper to combo box (dropdown) control.
class combo final : public base::native_control {
public:
	combo& assign(const base::wnd* parent, int controlId) {
		this->native_control::assign(parent, controlId);
		return *this;
	}

	combo& create(const base::wnd* parent, int controlId, POINT pos, LONG width, bool sorted) {
		this->native_control::create(parent, controlId, nullptr,
			pos, {width,0}, L"combobox",
			WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | (sorted ? CBS_SORT : 0), 0);
		return *this;
	}

	combo& focus() {
		SetFocus(this->hwnd());
		return *this;
	}

	combo& enable(bool doEnable) {
		EnableWindow(this->hwnd(), doEnable);
		return *this;
	}

	size_t item_count() const {
		return SendMessageW(this->hwnd(), CB_GETCOUNT, 0, 0);
	}

	size_t item_get_selected() const {
		return static_cast<size_t>(SendMessageW(this->hwnd(), CB_GETCURSEL, 0, 0));
	}

	combo& item_purge() {
		SendMessageW(this->hwnd(), CB_RESETCONTENT, 0, 0);
		return *this;
	}

	combo& item_add(const wchar_t* entries, wchar_t delimiter = L'|') {
		wchar_t delim[2] = { delimiter, L'\0' };
		std::vector<std::wstring> vals = str::explode(entries, delim);
		for (const std::wstring& s : vals) {
			SendMessageW(this->hwnd(), CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(s.c_str()));
		}
		return *this;
	}

	combo& item_add(std::initializer_list<const wchar_t*> entries) {
		for (const wchar_t* s : entries) {
			SendMessageW(this->hwnd(), CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(s));
		}
		return *this;
	}

	std::wstring item_get_text(size_t index) const {
		std::wstring buf;
		size_t len = SendMessageW(this->hwnd(), CB_GETLBTEXTLEN, index, 0);
		if (len) {
			buf.resize(len, L'\0');
			SendMessageW(this->hwnd(), CB_GETLBTEXT, index, reinterpret_cast<LPARAM>(&buf[0]));
			buf.resize(len);
		}
		return buf;
	}

	std::wstring item_get_selected_text() const {
		return this->item_get_text(this->item_get_selected());
	}

	combo& item_set_selected(size_t index) {
		SendMessageW(this->hwnd(), CB_SETCURSEL, index, 0);
		return *this;
	}
};

}//namespace wl