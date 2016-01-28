
#pragma once
#include <functional>
#include <string>
#include <vector>
#include "FileIni.h"
#include "Menu.h"

class MenuRecent final {
public:
	typedef std::function<void(const std::wstring&, size_t)> OnClickType;

private:
	Menu                      _menu;
	WORD                      _menuItemId, _firstCmdId;
	FileIni                  *_pIni;
	std::wstring              _section;
	std::vector<std::wstring> _entries;
	size_t                    _maxEntries;
	OnClickType               _onClick;
public:
	MenuRecent();
	MenuRecent& create(HWND hWnd, Menu subMenu, int menuItemId, FileIni& ini, const wchar_t *iniSection, size_t maxEntries = 9);
	MenuRecent& onClick(OnClickType callback);
	MenuRecent& addEntry(const std::wstring& entry);
	MenuRecent& removeEntry(size_t index);
	MenuRecent& load();
	MenuRecent& flush();
private:
	void _rebuildMenu();
	static LRESULT CALLBACK _proc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR idSubclass, DWORD_PTR refData);
};