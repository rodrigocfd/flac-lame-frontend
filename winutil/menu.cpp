/**
 * Part of WinUtil - Windows Utilities
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winutil
 */

#include "menu.h"
using namespace winutil;
using std::initializer_list;
using std::wstring;

menu& menu::operator=(HMENU hMenu)
{
	_hMenu = hMenu;
	return *this;
}

menu& menu::operator=(const menu& m)
{
	_hMenu = m._hMenu;
	return *this;
}

menu& menu::operator=(menu&& m)
{
	_hMenu = m._hMenu;
	m._hMenu = nullptr;
	return *this;
}

menu& menu::load_resource(int resourceId, HINSTANCE hInst)
{
	destroy();
	if (!hInst) hInst = GetModuleHandle(nullptr);
	_hMenu = LoadMenu(hInst, MAKEINTRESOURCE(resourceId));
	return *this;
}

menu& menu::load_resource(int resourceId, size_t subMenuIndex, HINSTANCE hInst)
{
	load_resource(resourceId, hInst);
	_hMenu = GetSubMenu(_hMenu, static_cast<int>(subMenuIndex));
	return *this;
}

void menu::destroy()
{
	if (_hMenu) {
		DestroyMenu(_hMenu);
		_hMenu = nullptr;
	}
}

wstring menu::get_caption(WORD commandId) const
{
	wchar_t captionBuf[64]; // arbitrary buffer length
	MENUITEMINFO mii = { 0 };

	mii.cbSize     = sizeof(mii);
	mii.cch        = ARRAYSIZE(captionBuf);
	mii.dwTypeData = captionBuf;
	mii.fMask      = MIIM_STRING;

	GetMenuItemInfo(_hMenu, commandId, FALSE, &mii);
	return captionBuf;
}

menu& menu::add_separator(WORD insertBeforeCmdId)
{
	if (insertBeforeCmdId) { // insert before the specified command ID
		InsertMenu(_hMenu, insertBeforeCmdId, MF_BYCOMMAND | MF_SEPARATOR, 0, nullptr);
	} else { // just append
		InsertMenu(_hMenu, -1, MF_BYPOSITION | MF_SEPARATOR, 0, nullptr);
	}
	return *this;
}

menu& menu::add_item(WORD commandId, const wchar_t* caption, WORD insertBeforeCmdId)
{
	if (insertBeforeCmdId) { // insert before the specified command ID
		InsertMenu(_hMenu, insertBeforeCmdId, MF_BYCOMMAND | MF_STRING, commandId, caption);
	} else { // just append
		InsertMenu(_hMenu, -1, MF_BYPOSITION | MF_STRING, commandId, caption);
	}
	return *this;
}

menu& menu::enable_item(WORD commandId, bool doEnable)
{
	EnableMenuItem(_hMenu, commandId,
		MF_BYCOMMAND | ((doEnable) ? MF_ENABLED : MF_GRAYED));
	return *this;
}

menu& menu::delete_item_by_pos(size_t pos)
{
	DeleteMenu(_hMenu, static_cast<UINT>(pos), MF_BYPOSITION);
	return *this;
}

menu& menu::delete_item_by_id(WORD commandId)
{
	DeleteMenu(_hMenu, commandId, MF_BYCOMMAND);
	return *this;
}

menu& menu::delete_all_items()
{
	for (size_t i = get_item_count(); i-- > 0; ) {
		delete_item_by_pos(i);
	}
	return *this;
}

menu& menu::enable_item(initializer_list<WORD> commandIds, bool doEnable)
{
	for (const WORD& cmd : commandIds) {
		enable_item(cmd, doEnable);
	}
	return *this;
}

menu& menu::set_default_item(WORD commandId)
{
	SetMenuDefaultItem(_hMenu, commandId, MF_BYCOMMAND);
	return *this;
}

menu menu::add_submenu(const wchar_t* caption, WORD insertBeforeCmdId)
{
	menu sub;
	sub._hMenu = CreatePopupMenu();

	if (insertBeforeCmdId) { // insert before the specified command ID
		InsertMenu(_hMenu, insertBeforeCmdId, MF_POPUP | MF_BYCOMMAND,
			reinterpret_cast<UINT_PTR>(sub._hMenu), caption);
	} else { // just append
		AppendMenu(_hMenu, MF_STRING | MF_POPUP,
			reinterpret_cast<UINT_PTR>(sub._hMenu), caption);
	}
	
	return sub; // return new submenu, so it can be edited
}

menu& menu::show_at_point(HWND hParent, POINT pt, HWND hWndCoordsRelativeTo)
{
	// Shows a popup context menu, anchored at the given coordinates.
	// The passed coordinates can be relative to any window.

	POINT ptParent = pt; // receives coordinates relative to hParent
	ClientToScreen(hWndCoordsRelativeTo ? hWndCoordsRelativeTo : hParent, &ptParent); // to screen coordinates
	SetForegroundWindow(hParent);
	TrackPopupMenu(_hMenu, 0, ptParent.x, ptParent.y, 0, hParent, nullptr); // owned by dialog, so messages go to it
	PostMessage(hParent, WM_NULL, 0, 0); // http://msdn.microsoft.com/en-us/library/ms648002%28VS.85%29.aspx
	return *this;
}