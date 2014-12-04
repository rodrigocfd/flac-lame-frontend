//
// Automation to ordinary menus.
// Part of TOOLOW - Thin Object Oriented Layer Over Win32.
// @author Rodrigo Cesar de Freitas Dias
// @see https://github.com/rodrigocfd/toolow
//

#include "String.h"

class Menu {
private:
	HMENU _hMenu;
public:
	Menu()            : _hMenu(nullptr) { }
	Menu(HMENU hMenu) : _hMenu(hMenu) { }
	
	HMENU hMenu() const             { return _hMenu; }
	int   size() const              { return ::GetMenuItemCount(_hMenu); }
	void  destroy()                 { if (_hMenu) { ::DestroyMenu(_hMenu); _hMenu = nullptr; } }
	Menu  getSubmenu(int pos) const { return Menu(::GetSubMenu(_hMenu, pos)); }
	WORD  getCmdId(int pos) const   { return ::GetMenuItemID(_hMenu, pos); }

	Menu& createMain(HWND owner) {
		destroy();
		_hMenu = ::CreateMenu(); // to be used as a main window menu
		appendItem(L"_DUMMY_", WM_APP-2); // avoids further call to DrawMenuBar(), which would demand HWND again
		::SetMenu(owner, _hMenu);
		return *this;
	}
	Menu& createPopup() {
		destroy();
		_hMenu = ::CreatePopupMenu(); // to be used as a popup menu
		return *this;
	}

	Menu& appendSeparator() {
		_checkDummyEntry();
		::InsertMenu(_hMenu, -1, MF_BYPOSITION | MF_SEPARATOR, 0, nullptr);
		return *this;
	}
	Menu& appendItem(const wchar_t *caption, WORD cmdId) {
		_checkDummyEntry();
		::InsertMenu(_hMenu, -1, MF_BYPOSITION | MF_STRING, cmdId, caption);
		return *this;
	}
	Menu& enableItem(WORD cmdId, bool doEnable) {
		::EnableMenuItem(_hMenu, cmdId, MF_BYCOMMAND | ((doEnable) ? MF_ENABLED : MF_GRAYED));
		return *this;
	}

	Menu appendSubmenu(const wchar_t *caption) {
		_checkDummyEntry();
		Menu sub; sub.createPopup();
		::AppendMenu(_hMenu, MF_STRING | MF_POPUP, (UINT_PTR)sub.hMenu(), caption);
		return sub; // return new submenu, so it can be edited
	}
private:
	void _checkDummyEntry() {
		if (size() == 1 && ::GetMenuItemID(_hMenu, 0) == WM_APP-2)
			::DeleteMenu(_hMenu, 0, MF_BYPOSITION); // delete dummy, if any
	}
};