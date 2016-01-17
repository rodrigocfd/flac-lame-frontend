
#include "Menu.h"
using std::initializer_list;
using std::wstring;

Menu::Menu()
	: _hMenu(nullptr)
{
}

Menu::Menu(HMENU hMenu)
	: _hMenu(hMenu)
{
}

Menu::Menu(Menu&& m)
	: _hMenu(m._hMenu)
{
	m._hMenu = nullptr;
}

Menu& Menu::operator=(HMENU hMenu)
{
	_hMenu = hMenu;
	return *this;
}

Menu& Menu::operator=(Menu&& m)
{
	std::swap(_hMenu, m._hMenu);
	return *this;
}

HMENU Menu::hMenu() const
{
	return _hMenu;
}

Menu& Menu::loadResource(int resourceId, HINSTANCE hInst)
{
	destroy();
	if (!hInst) hInst = GetModuleHandle(nullptr);
	_hMenu = LoadMenu(hInst, MAKEINTRESOURCE(resourceId));
	return *this;
}

Menu& Menu::loadResource(int resourceId, int subMenuIndex, HINSTANCE hInst)
{
	loadResource(resourceId, hInst);
	_hMenu = GetSubMenu(_hMenu, subMenuIndex);
	return *this;
}

void Menu::destroy()
{
	if (_hMenu) {
		DestroyMenu(_hMenu);
		_hMenu = nullptr;
	}
}

int Menu::size() const
{
	return GetMenuItemCount(_hMenu);
}

Menu Menu::getSubmenu(int pos) const
{
	return Menu(GetSubMenu(_hMenu, pos));
}

WORD Menu::getCommandId(int index) const
{
	return GetMenuItemID(_hMenu, index);
}

Menu& Menu::addSeparator()
{
	InsertMenu(_hMenu, -1, MF_BYPOSITION | MF_SEPARATOR, 0, nullptr);
	return *this;
}

Menu& Menu::addItem(WORD commandId, const wchar_t *caption)
{
	InsertMenu(_hMenu, -1, MF_BYPOSITION | MF_STRING, commandId, caption);
	return *this;
}

Menu& Menu::addItem(WORD commandId, const wstring& caption)
{
	return addItem(commandId, caption.c_str());
}

Menu& Menu::enableItem(WORD commandId, bool doEnable)
{
	EnableMenuItem(_hMenu, commandId,
		MF_BYCOMMAND | ((doEnable) ? MF_ENABLED : MF_GRAYED));
	return *this;
}

Menu& Menu::enableItem(initializer_list<WORD> commandIds, bool doEnable)
{
	for (const WORD& cmd : commandIds) {
		enableItem(cmd, doEnable);
	}
	return *this;
}

Menu Menu::addSubmenu(const wchar_t *caption)
{
	Menu sub;
	sub._hMenu = CreatePopupMenu();
	AppendMenu(_hMenu, MF_STRING | MF_POPUP,
		reinterpret_cast<UINT_PTR>(sub._hMenu), caption);
	return sub; // return new submenu, so it can be edited
}

Menu Menu::addSubmenu(const wstring& caption)
{
	return addSubmenu(caption.c_str());
}

Menu& Menu::showAtPoint(HWND hParent, POINT pt, HWND hWndCoordsRelativeTo)
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