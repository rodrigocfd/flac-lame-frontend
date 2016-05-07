/**
 * Part of WinUtil - Windows Utilities
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winutil
 */

#include <vector>
#include "menu_recent.h"
#include <CommCtrl.h>
#include "file.h"
#include "file_ini.h"
#include "path.h"
#include "str.h"
using namespace winutil;
using std::vector;
using std::wstring;

menu_recent::menu_recent()
	: _iniFile(nullptr), _maxEntries(9), _firstCmdId(0)
{	
}

menu_recent& menu_recent::init(HWND hWnd, size_t subMenuIndex, WORD subMenuItemCmdId, file_ini& iniFile, const wchar_t *iniSection, size_t maxEntries)
{
	if (_firstCmdId) return *this; // avoid running twice

	// The subMenuItemCmdId is a regular menu entry with a caption like "Recent files".
	// A submenu with the recent files will be created right at it.

	_iniFile = &iniFile;
	_iniSection = iniSection;
	_maxEntries = maxEntries;

	static WORD baseCmdId = 0xE110; // taken from MFC, supposedly hard to be picked up by user
	_firstCmdId = baseCmdId; // will be used when matching IDs within _sub_proc
	baseCmdId += static_cast<WORD>(_maxEntries); // so the next instance of menu_recent will have different IDs

	menu subMenu = GetMenu(hWnd);
	subMenu = subMenu.get_submenu(subMenuIndex);

	_injectedMenu = subMenu.add_submenu(subMenu.get_caption(subMenuItemCmdId), subMenuItemCmdId); // inject submenu
	subMenu.delete_item_by_id(subMenuItemCmdId); // remove ordinary entry
	rebuild_menu();

	// Subclass parent so we handle the WM_COMMAND messages ourselves; pass pointer to object.
	static UINT_PTR uniqueSubId = 1;
	SetWindowSubclass(hWnd, _proc, uniqueSubId++, reinterpret_cast<DWORD_PTR>(this));

	return *this;
}

wstring menu_recent::get_entry(size_t index) const
{
	bool good = _iniFile &&
		!_iniSection.empty() &&
		_iniFile->has_section(_iniSection) &&
		index < _maxEntries;

	if (good) {
		wstring key = str::format(L"file%u", index + 1);
		if (_iniFile->has_key(_iniSection, key)) {
			return _iniFile->val(_iniSection, key);
		}
	}

	return L"";
}

menu_recent& menu_recent::add_entry(const wchar_t *newPath)
{
	vector<wstring> entries = _read_entries();

	for (auto itE = entries.begin(); itE != entries.end(); ++itE) {
		if (path::is_same(*itE, newPath)) {
			entries.erase(itE); // remove if already present
			break;
		}
	}

	if (entries.size() == _maxEntries) {
		entries.pop_back(); // remove last if max reached
	}

	entries.insert(entries.begin(), newPath); // insert as first
	_write_entries(entries);
	_rebuild_menu_entries(&entries);
	return *this;
}

menu_recent& menu_recent::remove_entry(size_t index)
{
	vector<wstring> entries = _read_entries();

	if (index <= entries.size() - 1) {
		entries.erase(entries.begin() + index);
		_write_entries(entries);
		_rebuild_menu_entries(&entries);
	}
	return *this;
}

vector<wstring> menu_recent::_read_entries() const
{
	vector<wstring> ret;
	
	if (_iniFile && !_iniSection.empty() && _iniFile->has_section(_iniSection)) {
		ret.reserve(_maxEntries);
		wstring key;

		for (size_t i = 0; i < _maxEntries; ++i) {
			key = str::format(L"file%u", i + 1);
			if (_iniFile->has_key(_iniSection, key)) {
				ret.emplace_back(_iniFile->val(_iniSection, key));
			}
		}
	}

	return ret;
}

void menu_recent::_write_entries(const vector<wstring> entries)
{
	if (!_iniSection.empty()) {
		_iniFile->clear_section(_iniSection);

		for (size_t i = 0; i < entries.size(); ++i) {
			_iniFile->add(_iniSection,
				str::format(L"file%u", i + 1),
				entries[i]);
		}
	}
}

void menu_recent::_rebuild_menu_entries(const vector<wstring> *entries)
{
	if (_injectedMenu.hmenu()) {
		_injectedMenu.delete_all_items();

		vector<wstring> entriesBuf;
		if (!entries) {
			entriesBuf = _read_entries();
			entries = &entriesBuf; // if not passed, load from INI structure
		}

		if (entries->empty()) {
			_injectedMenu.add_item(_firstCmdId, L"(none)"); // arbitrary text
			_injectedMenu.enable_item(_firstCmdId, false);
		} else {
			for (size_t i = 0; i < entries->size(); ++i) {
				const wchar_t *fmtStr = (i <= 8) ? L"&%u %s" : L"%u %s";
				_injectedMenu.add_item(static_cast<WORD>(_firstCmdId + i), // unique command ID
					str::format(fmtStr, i + 1, (*entries)[i].c_str()) );
			}
		}
	}
}

LRESULT CALLBACK menu_recent::_proc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR idSubclass, DWORD_PTR refData)
{
	switch (msg) {
	case WM_COMMAND: {
		menu_recent *pSelf = reinterpret_cast<menu_recent*>(refData);
		bool isOurCmd = (LOWORD(wp) >= pSelf->_firstCmdId) &&
			(LOWORD(wp) <= pSelf->_firstCmdId + pSelf->_maxEntries - 1);
		if (isOurCmd && pSelf->_onClick) {
			size_t index = LOWORD(wp) - pSelf->_firstCmdId;
			pSelf->_onClick(index); // invoke user callback, pass zero-based index of selected entry
			return 0; // halt, WM_COMMAND is done here
		}
		break;
	}
	case WM_NCDESTROY:
		RemoveWindowSubclass(hWnd, _proc, idSubclass); // http://blogs.msdn.com/b/oldnewthing/archive/2003/11/11/55653.aspx
	}
	return DefSubclassProc(hWnd, msg, wp, lp); // parent window can process WM_COMMAND too
}