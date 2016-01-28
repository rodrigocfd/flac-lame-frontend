
#pragma once
#include <string>
#include <Windows.h>

class Menu {
protected:
	HMENU _hMenu;
public:
	Menu();
	Menu(HMENU hMenu);
	Menu(const Menu& m);
	Menu(Menu&& m);
	Menu& operator=(HMENU hMenu);
	Menu& operator=(const Menu& m);
	Menu& operator=(Menu&& m);

	HMENU        hMenu() const;
	Menu&        loadResource(int resourceId, HINSTANCE hInst = nullptr);
	Menu&        loadResource(int resourceId, size_t subMenuIndex, HINSTANCE hInst);
	void         destroy();
	int          size() const;
	Menu         getSubmenu(size_t pos) const;
	WORD         getCommandId(size_t pos) const;
	std::wstring getCaption(WORD commandId) const;
	size_t       getItemCount() const;
	Menu&        addSeparator();
	Menu&        addItem(WORD commandId, const wchar_t *caption);
	Menu&        addItem(WORD commandId, const std::wstring& caption);
	Menu&        enableItem(WORD commandId, bool doEnable);
	Menu&        enableItem(std::initializer_list<WORD> commandIds, bool doEnable);
	Menu&        deleteItemByPos(size_t pos);
	Menu&        deleteItemById(WORD commandId);
	Menu&        deleteAllItems();
	Menu         addSubmenu(const wchar_t *caption);
	Menu         addSubmenu(const std::wstring& caption);
	Menu&        showAtPoint(HWND hParent, POINT pt, HWND hWndCoordsRelativeTo);
};