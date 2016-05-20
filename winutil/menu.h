/**
 * Part of WinUtil - Windows Utilities
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winutil
 */

#pragma once
#include <string>
#include <Windows.h>

namespace winutil {

class menu {
protected:
	HMENU _hMenu;
public:
	menu()              : _hMenu(nullptr) { }
	menu(HMENU hMenu)   : _hMenu(hMenu) { }
	menu(const menu& m) : _hMenu(m._hMenu) { }
	menu(menu&& m)      : menu() { operator=(std::move(m)); }
	menu& operator=(HMENU hMenu);
	menu& operator=(const menu& m);
	menu& operator=(menu&& m);

	HMENU        hmenu() const { return _hMenu; }
	menu&        load_resource(int resourceId, HINSTANCE hInst = nullptr);
	menu&        load_resource(int resourceId, size_t subMenuIndex, HINSTANCE hInst);
	void         destroy();
	menu         get_submenu(size_t pos) const    { return menu(GetSubMenu(_hMenu, static_cast<int>(pos))); }
	WORD         get_command_id(size_t pos) const { return GetMenuItemID(_hMenu, static_cast<int>(pos)); }
	std::wstring get_caption(WORD commandId) const;
	size_t       get_item_count() const           { return static_cast<size_t>(GetMenuItemCount(_hMenu)); }
	menu&        add_separator(WORD insertBeforeCmdId = 0);
	menu&        add_item(WORD commandId, const wchar_t* caption, WORD insertBeforeCmdId = 0);
	menu&        add_item(WORD commandId, const std::wstring& caption, WORD insertBeforeCmdId = 0) { return add_item(commandId, caption.c_str(), insertBeforeCmdId); }
	menu&        enable_item(WORD commandId, bool doEnable);
	menu&        enable_item(std::initializer_list<WORD> commandIds, bool doEnable);
	menu&        set_default_item(WORD commandId);
	menu&        delete_item_by_pos(size_t pos);
	menu&        delete_item_by_id(WORD commandId);
	menu&        delete_all_items();
	menu         add_submenu(const wchar_t* caption, WORD insertBeforeCmdId = 0);
	menu         add_submenu(const std::wstring& caption, WORD insertBeforeCmdId = 0) { return add_submenu(caption.c_str(), insertBeforeCmdId); }
	menu&        show_at_point(HWND hParent, POINT pt, HWND hWndCoordsRelativeTo);
};

}//namespace winutil