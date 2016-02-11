
#include "MenuRecent.h"
#include "Str.h"
#include <CommCtrl.h>
using std::function;
using std::map;
using std::wstring;

MenuRecent::MenuRecent()
	: _menuItemId(0), _firstCmdId(0), _pIni(nullptr), _maxEntries(9)
{
}

MenuRecent& MenuRecent::create(HWND hWnd, Menu subMenu, int menuItemId, FileIni& ini, const wchar_t *iniSection, size_t maxEntries)
{
	// The menuItemId is a regular menu entry with a caption like "Recent files".
	// A submenu with the recent files will be created right at it.

	if (_menu.hMenu()) return *this;

	_pIni       = &ini;
	_section    = iniSection;
	_menuItemId = menuItemId;
	_maxEntries = maxEntries;

	static WORD baseCmdId = 0xE110; // taken from MFC, supposedly hard to be picked up by user
	_firstCmdId = baseCmdId; // first entry of this instance
	baseCmdId += static_cast<WORD>(_maxEntries); // so next instance will have different IDs

	wstring entryCaption = subMenu.getCaption(_menuItemId);
	_menu = CreatePopupMenu();
	InsertMenu(subMenu.hMenu(), _menuItemId, MF_POPUP | MF_BYCOMMAND,
		reinterpret_cast<UINT_PTR>(_menu.hMenu()), entryCaption.c_str()); // insert submenu before menu entry
	DeleteMenu(subMenu.hMenu(), _menuItemId, MF_BYCOMMAND); // remove menu entry; effectively replace it

	_rebuildMenu();
	
	// Subclass parent so we handle the WM_COMMAND messages ourselves; pass pointer to object.
	static UINT_PTR uniqueSubId = 1;
	SetWindowSubclass(hWnd, _proc, uniqueSubId++, reinterpret_cast<DWORD_PTR>(this));
	return *this;
}

MenuRecent& MenuRecent::onClick(OnClickType callback)
{
	_onClick = std::move(callback);
	return *this;
}

MenuRecent& MenuRecent::addEntry(const wstring& entry)
{
	wstring entryU = Str::upper(entry);

	for (size_t i = 0; i < _entries.size(); ++i) {
		if (entryU == Str::upper(_entries[i])) {
			_entries.erase(_entries.begin() + i); // remove if existing
			break;
		}
	}

	if (_entries.size() == _maxEntries) {
		_entries.erase(_entries.begin() + _entries.size() - 1); // remove last if max reached
	}

	_entries.insert(_entries.begin(), entry); // insert at top
	_rebuildMenu();
	return *this;
}

MenuRecent& MenuRecent::removeEntry(size_t index)
{
	_entries.erase(_entries.begin() + index);
	_rebuildMenu();
	return *this;
}

MenuRecent& MenuRecent::load()
{
	_entries.clear();
	const auto sectionIt = _pIni->data.find(_section); // assumes FileIni has been loaded already
	if (sectionIt != _pIni->data.end()) {
		for (const auto& entry : sectionIt->second) {
			if (Str::beginsWith(entry.first, L"file") && !entry.second.empty()) {
				_entries.emplace_back(entry.second);
			}
		}
		_rebuildMenu();
	}
	return *this;
}

MenuRecent& MenuRecent::flush()
{
	auto sectionIt = _pIni->data.find(_section); // assumes FileIni has been loaded already
	if (sectionIt == _pIni->data.end()) {
		auto ret = _pIni->data.emplace(_section, map<wstring, wstring>()); // add if absent yet
		sectionIt = ret.first;
	}
	sectionIt->second.clear();
	for (size_t i = 0; i < _entries.size(); ++i) {
		sectionIt->second.emplace(Str::format(L"file%u", i + 1), _entries[i]);
	}
	return *this;
}

void MenuRecent::_rebuildMenu()
{
	_menu.deleteAllItems();
	if (_entries.empty()) {
		_menu.addItem(_firstCmdId, L"(none)"); // arbitrary text
		_menu.enableItem(_firstCmdId, false);
	} else {
		for (size_t i = 0; i < _entries.size(); ++i) {
			_menu.addItem(static_cast<WORD>(_firstCmdId + i),
				Str::format(L"&%u %s", i + 1, _entries[i].c_str()) );
		}
	}
}

LRESULT CALLBACK MenuRecent::_proc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR idSubclass, DWORD_PTR refData)
{
	switch (msg) {
	case WM_COMMAND: {
		MenuRecent *pSelf = reinterpret_cast<MenuRecent*>(refData);
		bool isOurCmd = (LOWORD(wp) >= pSelf->_firstCmdId) &&
			(LOWORD(wp) <= pSelf->_firstCmdId + pSelf->_maxEntries - 1);
		if (isOurCmd && pSelf->_onClick) {
			size_t index = LOWORD(wp) - pSelf->_firstCmdId;
			pSelf->_onClick(pSelf->_entries[index], index); // invoke user callback, pass path and position
			return 0; // halt, WM_COMMAND is done here
		}
		break;
	}
	case WM_NCDESTROY:
		RemoveWindowSubclass(hWnd, _proc, idSubclass); // http://blogs.msdn.com/b/oldnewthing/archive/2003/11/11/55653.aspx
	}
	return DefSubclassProc(hWnd, msg, wp, lp); // parent window can process WM_COMMAND too
}