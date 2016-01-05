/**
 * Part of WOLF - WinAPI Object Lambda Framework
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#include "Menu.h"
using namespace wolf;
using std::function;
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
	this->_hMenu = hMenu;
	return *this;
}

Menu& Menu::operator=(Menu&& m)
{
	std::swap(this->_hMenu, m._hMenu);
	return *this;
}

HMENU Menu::hMenu() const
{
	return this->_hMenu;
}

void Menu::destroy()
{
	if (this->_hMenu) {
		DestroyMenu(this->_hMenu);
		this->_hMenu = nullptr;
	}
}

int Menu::size() const
{
	return GetMenuItemCount(this->_hMenu);
}

Menu Menu::getSubmenu(int pos) const
{
	return Menu(GetSubMenu(this->_hMenu, pos));
}

WORD Menu::getCommandId(int index) const
{
	return GetMenuItemID(this->_hMenu, index);
}

Menu& Menu::addSeparator()
{
	this->_createOnce();
	InsertMenu(this->_hMenu, -1, MF_BYPOSITION | MF_SEPARATOR, 0, nullptr);
	return *this;
}

Menu& Menu::addItem(WORD commandId, const wchar_t *caption)
{
	this->_createOnce();
	InsertMenu(this->_hMenu, -1, MF_BYPOSITION | MF_STRING, commandId, caption);
	return *this;
}

Menu& Menu::addItem(WORD commandId, const wstring& caption)
{
	return this->addItem(commandId, caption.c_str());
}

Menu& Menu::enableItem(WORD commandId, bool doEnable)
{
	EnableMenuItem(this->_hMenu, commandId,
		MF_BYCOMMAND | ((doEnable) ? MF_ENABLED : MF_GRAYED));
	return *this;
}

Menu& Menu::enableItem(initializer_list<WORD> commandIds, bool doEnable)
{
	for (const WORD& cmd : commandIds) {
		this->enableItem(cmd, doEnable);
	}
	return *this;
}

Menu Menu::addSubmenu(const wchar_t *caption)
{
	this->_createOnce();
	Menu sub;
	sub._hMenu = CreatePopupMenu();
	AppendMenu(this->_hMenu, MF_STRING | MF_POPUP,
		reinterpret_cast<UINT_PTR>(sub._hMenu), caption);
	return sub; // return new submenu, so it can be edited
}

Menu Menu::addSubmenu(const wstring& caption)
{
	return this->addSubmenu(caption.c_str());
}

void Menu::_createOnce()
{
	// Do nothing here.
}


void MenuInitHandler::onInitMenuPopup(function<void()> callback)
{
	this->_callback = std::move(callback);
}


void MenuMain::_createOnce()
{
	if (!this->_hMenu) {
		this->_hMenu = CreateMenu();
	}
}


void MenuContext::_createOnce()
{
	if (!this->_hMenu) {
		this->_hMenu = CreatePopupMenu();
	}
}

void MenuContext::showAtPoint(HWND hParent, POINT pt, HWND hWndCoordsRelativeTo)
{
	// Shows a popup context menu, anchored at the given coordinates.
	// The passed coordinates can be relative to any window.

	POINT ptParent = pt; // receives coordinates relative to hParent
	ClientToScreen(hWndCoordsRelativeTo ? hWndCoordsRelativeTo : hParent, &ptParent); // to screen coordinates
	SetForegroundWindow(hParent);
	TrackPopupMenu(this->_hMenu, 0, ptParent.x, ptParent.y, 0, hParent, nullptr); // owned by dialog, so messages go to it
	PostMessage(hParent, WM_NULL, 0, 0); // http://msdn.microsoft.com/en-us/library/ms648002%28VS.85%29.aspx
}