/**
 * Part of WinUtil - Windows Utilities
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winutil
 */

#pragma once
#include <functional>
#include <vector>
#include <Windows.h>
#include "file_ini.h"
#include "menu.h"

namespace winutil {

class menu_recent final {
private:
	file_ini    *_iniFile;
	std::wstring _iniSection;
	size_t       _maxEntries;
	WORD         _firstCmdId;
	menu         _injectedMenu;
	std::function<void(size_t)> _onClick;

public:
	menu_recent();
	menu_recent& init(HWND hWnd, size_t subMenuIndex, WORD subMenuItemCmdId, winutil::file_ini& iniFile, const wchar_t *iniSection, size_t maxEntries = 9);
	std::wstring get_entry(size_t index) const;
	menu_recent& add_entry(const wchar_t *newPath);
	menu_recent& add_entry(const std::wstring& newPath)         { return add_entry(newPath.c_str()); }
	menu_recent& remove_entry(size_t index);
	menu_recent& rebuild_menu()                                 { _rebuild_menu_entries(nullptr); return *this; }
	menu_recent& on_click(std::function<void(size_t)> callback) { _onClick = std::move(callback); return *this; }

private:
	std::vector<std::wstring> _read_entries() const;
	void _write_entries(const std::vector<std::wstring> entries);
	void _rebuild_menu_entries(const std::vector<std::wstring> *entries);
	static LRESULT CALLBACK _proc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR idSubclass, DWORD_PTR refData);
};

}//namespace winutil