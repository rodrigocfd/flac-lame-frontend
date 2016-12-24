/**
 * Part of WET - WinAPI Easy Toolkit
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wet
 */

#pragma once
#include <Windows.h>

namespace wet {

class menu {
protected:
	HMENU _hMenu;

public:
	menu()              : _hMenu(nullptr) { }
	menu(HMENU hMenu)   : _hMenu(hMenu) { }
	menu(const menu& m) : _hMenu(m._hMenu) { }
	menu(menu&& m)      : menu() { operator=(std::move(m)); }
	
	menu& operator=(HMENU hMenu) {
		this->_hMenu = hMenu;
		return *this;
	}

	menu& operator=(const menu& m) {
		this->_hMenu = m._hMenu;
		return *this;
	}

	menu& operator=(menu&& m) {
		this->_hMenu = m._hMenu;
		m._hMenu = nullptr;
		return *this;
	}
	
	HMENU hmenu() const {
		return this->_hMenu;
	}

	void destroy() {
		if (this->_hMenu) {
			DestroyMenu(this->_hMenu);
			this->_hMenu = nullptr;
		}
	}

	menu& load_resource(int resourceId, HINSTANCE hInst = nullptr) {
		this->destroy();
		if (!hInst) hInst = GetModuleHandleW(nullptr);
		this->_hMenu = LoadMenuW(hInst, MAKEINTRESOURCE(resourceId));
		return *this;
	}
	
	menu& load_resource(int resourceId, size_t subMenuIndex, HINSTANCE hInst) {
		this->load_resource(resourceId, hInst);
		this->_hMenu = GetSubMenu(this->_hMenu, static_cast<int>(subMenuIndex));
		return *this;
	}

	menu get_submenu(size_t pos) const {
		return menu(GetSubMenu(this->_hMenu, static_cast<int>(pos)));
	}

	WORD get_command_id(size_t pos) const {
		return GetMenuItemID(this->_hMenu, static_cast<int>(pos));
	}

	std::wstring get_caption(WORD commandId) const {
		wchar_t captionBuf[64] = { L'\0' }; // arbitrary buffer length
		MENUITEMINFO mii = { 0 };

		mii.cbSize     = sizeof(mii);
		mii.cch        = ARRAYSIZE(captionBuf);
		mii.dwTypeData = captionBuf;
		mii.fMask      = MIIM_STRING;

		GetMenuItemInfoW(this->_hMenu, commandId, FALSE, &mii);
		return captionBuf;
	}

	size_t get_item_count() const {
		return static_cast<size_t>(GetMenuItemCount(this->_hMenu));
	}

	menu& add_separator(WORD insertBeforeCmdId = 0) {
		if (insertBeforeCmdId) { // insert before the specified command ID
			InsertMenuW(this->_hMenu, insertBeforeCmdId, MF_BYCOMMAND | MF_SEPARATOR, 0, nullptr);
		} else { // just append
			InsertMenuW(this->_hMenu, -1, MF_BYPOSITION | MF_SEPARATOR, 0, nullptr);
		}
		return *this;
	}

	menu& add_item(WORD commandId, const wchar_t* caption, WORD insertBeforeCmdId = 0) {
		if (insertBeforeCmdId) { // insert before the specified command ID
			InsertMenuW(this->_hMenu, insertBeforeCmdId, MF_BYCOMMAND | MF_STRING, commandId, caption);
		} else { // just append
			InsertMenuW(this->_hMenu, -1, MF_BYPOSITION | MF_STRING, commandId, caption);
		}
		return *this;
	}

	menu& add_item(WORD commandId, const std::wstring& caption, WORD insertBeforeCmdId = 0) {
		return this->add_item(commandId, caption.c_str(), insertBeforeCmdId);
	}

	menu& enable_item(WORD commandId, bool doEnable) {
		EnableMenuItem(this->_hMenu, commandId,
			MF_BYCOMMAND | ((doEnable) ? MF_ENABLED : MF_GRAYED));
		return *this;
	}

	menu& enable_item(std::initializer_list<WORD> commandIds, bool doEnable) {
		for (const WORD& cmdId : commandIds) {
			this->enable_item(cmdId, doEnable);
		}
		return *this;
	}

	menu& set_default_item(WORD commandId) {
		SetMenuDefaultItem(this->_hMenu, commandId, MF_BYCOMMAND);
		return *this;
	}

	menu& delete_item_by_pos(size_t pos) {
		DeleteMenu(this->_hMenu, static_cast<UINT>(pos), MF_BYPOSITION);
		return *this;
	}

	menu& delete_item_by_id(WORD commandId) {
		DeleteMenu(this->_hMenu, commandId, MF_BYCOMMAND);
		return *this;
	}

	menu& delete_all_items() {
		for (size_t i = this->get_item_count(); i-- > 0; ) {
			this->delete_item_by_pos(i);
		}
		return *this;
	}

	menu add_submenu(const wchar_t* caption, WORD insertBeforeCmdId = 0) const {
		menu sub;
		sub._hMenu = CreatePopupMenu();

		if (insertBeforeCmdId) { // insert before the specified command ID
			InsertMenuW(this->_hMenu, insertBeforeCmdId, MF_POPUP | MF_BYCOMMAND,
				reinterpret_cast<UINT_PTR>(sub._hMenu), caption);
		} else { // just append
			AppendMenuW(this->_hMenu, MF_STRING | MF_POPUP,
				reinterpret_cast<UINT_PTR>(sub._hMenu), caption);
		}		
		return sub; // return new submenu, so it can be edited
	}

	menu add_submenu(const std::wstring& caption, WORD insertBeforeCmdId = 0) const {
		return this->add_submenu(caption.c_str(), insertBeforeCmdId);
	}

	menu& show_at_point(HWND hParent, POINT pt, HWND hWndCoordsRelativeTo) {
		// Shows a popup context menu, anchored at the given coordinates.
		// The passed coordinates can be relative to any window.
		POINT ptParent = pt; // receives coordinates relative to hParent
		ClientToScreen(hWndCoordsRelativeTo ? hWndCoordsRelativeTo : hParent, &ptParent); // to screen coordinates
		SetForegroundWindow(hParent);
		TrackPopupMenu(this->_hMenu, 0, ptParent.x, ptParent.y, 0, hParent, nullptr); // owned by dialog, so messages go to it
		PostMessageW(hParent, WM_NULL, 0, 0); // http://msdn.microsoft.com/en-us/library/ms648002%28VS.85%29.aspx
		return *this;
	}
};

}//namespace wet