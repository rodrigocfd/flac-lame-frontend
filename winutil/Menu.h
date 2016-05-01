
#pragma once
#include <string>
#include <Windows.h>

class Menu {
protected:
	HMENU _hMenu;
public:
	Menu()              : _hMenu(nullptr) { }
	Menu(HMENU hMenu)   : _hMenu(hMenu) { }
	Menu(const Menu& m) : _hMenu(m._hMenu) { }
	Menu(Menu&& m)      : Menu() { operator=(std::move(m)); }
	Menu& operator=(HMENU hMenu);
	Menu& operator=(const Menu& m);
	Menu& operator=(Menu&& m);

	HMENU        hMenu() const { return _hMenu; }
	Menu&        loadResource(int resourceId, HINSTANCE hInst = nullptr);
	Menu&        loadResource(int resourceId, size_t subMenuIndex, HINSTANCE hInst);
	void         destroy();
	Menu         getSubmenu(size_t pos) const   { return Menu(GetSubMenu(_hMenu, static_cast<int>(pos))); }
	WORD         getCommandId(size_t pos) const { return GetMenuItemID(_hMenu, static_cast<int>(pos)); }
	std::wstring getCaption(WORD commandId) const;
	size_t       getItemCount() const           { return static_cast<size_t>(GetMenuItemCount(_hMenu)); }
	Menu&        addSeparator();
	Menu&        addItem(WORD commandId, const wchar_t *caption);
	Menu&        addItem(WORD commandId, const std::wstring& caption) { return addItem(commandId, caption.c_str()); }
	Menu&        enableItem(WORD commandId, bool doEnable);
	Menu&        enableItem(std::initializer_list<WORD> commandIds, bool doEnable);
	Menu&        setDefaultItem(WORD commandId);
	Menu&        deleteItemByPos(size_t pos);
	Menu&        deleteItemById(WORD commandId);
	Menu&        deleteAllItems();
	Menu         addSubmenu(const wchar_t *caption);
	Menu         addSubmenu(const std::wstring& caption) { return addSubmenu(caption.c_str()); }
	Menu&        showAtPoint(HWND hParent, POINT pt, HWND hWndCoordsRelativeTo);
};