/**
 * Part of WinUtil - Windows Utilities
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winutil
 */

#pragma once
#include <string>
#include <Windows.h>

namespace winutil {

class combobox final {
private:
	HWND _hWnd;
public:
	combobox()          : _hWnd(nullptr) { }
	combobox(HWND hwnd) : _hWnd(hwnd) { }
	combobox& operator=(HWND hWnd);

	HWND         hwnd() const                   { return _hWnd; }
	combobox&    create(HWND hParent, int id, POINT pos, int width, bool sorted);
	int          get_id() const                 { return GetDlgCtrlID(_hWnd); }
	combobox&    enable(bool doEnable);
	combobox&    focus();
	size_t       item_count() const             { return SendMessage(_hWnd, CB_GETCOUNT, 0, 0); }
	combobox&    item_remove_all();
	combobox&    item_set_selected(size_t i);
	int          item_get_selected() const      { return static_cast<int>(SendMessage(_hWnd, CB_GETCURSEL, 0, 0)); }
	combobox&    item_add(const std::wstring& entry);
	combobox&    item_add(std::initializer_list<const wchar_t*> entries);
	combobox&    item_add(const wchar_t* entries, wchar_t delimiter = L'|');
	std::wstring item_get_text(size_t i) const;
	std::wstring item_get_selected_text() const { return item_get_text(item_get_selected()); }
};

}//namespace winutil