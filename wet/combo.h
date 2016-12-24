/**
 * Part of WET - WinAPI Easy Toolkit
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wet
 */

#pragma once
#include "base_widget.h"
#include "str.h"

/**
 * base_wnd <-- base_widget <-- combo
 */

namespace wet {

class combo final : public base_widget<combo> {
public:
	static const size_t npos = -1;

	combo()          = default;
	combo(HWND hWnd) : base_widget(hWnd) { }

	combo& operator=(HWND hWnd) {
		return this->base_widget::operator=(hWnd);
	}

	combo& create(const base_wnd* parent, int controlId, POINT pos, LONG width, bool sorted) {
		return this->base_widget::create(parent, controlId, nullptr,
			pos, {width,0}, L"combobox",
			WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | (sorted ? CBS_SORT : 0), 0);
	}

	size_t item_count() const {
		return SendMessageW(this->base_wnd::hwnd(), CB_GETCOUNT, 0, 0);
	}

	size_t item_get_selected() const {
		return static_cast<size_t>(SendMessageW(this->base_wnd::hwnd(), CB_GETCURSEL, 0, 0));
	}

	combo& item_purge() {
		SendMessageW(this->base_wnd::hwnd(), CB_RESETCONTENT, 0, 0);
		return *this;
	}

	combo& item_add(const wchar_t* entries, wchar_t delimiter = L'|') {
		wchar_t delim[2] = { delimiter, L'\0' };
		std::vector<std::wstring> vals = str::explode(entries, delim);
		for (const std::wstring& s : vals) {
			SendMessageW(this->base_wnd::hwnd(), CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(s.c_str()));
		}
		return *this;
	}

	combo& item_add(std::initializer_list<const wchar_t*> entries) {
		for (const wchar_t* s : entries) {
			SendMessageW(this->base_wnd::hwnd(), CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(s));
		}
		return *this;
	}

	std::wstring item_get_text(size_t index) const {
		size_t txtLen = SendMessageW(this->base_wnd::hwnd(), CB_GETLBTEXTLEN, index, 0);
		std::wstring buf(txtLen + 1, L'\0');
		SendMessageW(this->base_wnd::hwnd(), CB_GETLBTEXT, index, reinterpret_cast<LPARAM>(&buf[0]));
		buf.resize(txtLen);
		return buf;
	}

	std::wstring item_get_selected_text() const {
		return this->item_get_text(this->item_get_selected());
	}

	combo& item_set_selected(size_t index) {
		SendMessageW(this->base_wnd::hwnd(), CB_SETCURSEL, index, 0);
		return *this;
	}

private:
	base_wnd::_text;
	base_widget::create;
};

}//namespace wet