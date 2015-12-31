/**
 * Part of WOLF - WinAPI Object Lambda Framework
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#pragma once
#include <string>
#include <Windows.h>

namespace wolf {

class Menu final {
protected:
	HMENU _hMenu;
public:
	Menu();
	Menu(HMENU hMenu);
	Menu(Menu&& m);
	Menu& operator=(HMENU hMenu);
	Menu& operator=(Menu&& m);
	HMENU hMenu() const;
	void  destroy();
	Menu& createMain();
	Menu& createPopup();
	Menu& loadResource(int menuId);
	int   size() const;
	Menu  getSubmenu(int pos) const;
	WORD  getCmdId(int index) const;
	Menu& addSeparator();
	Menu& addItem(const wchar_t *caption, WORD cmdId);
	Menu& addItem(const std::wstring& caption, WORD cmdId);
	Menu& enableItem(std::initializer_list<WORD> cmdIds, bool doEnable);
	Menu  addSubmenu(const wchar_t *caption);
	Menu  addSubmenu(const std::wstring& caption);
	void  showAtPoint(HWND hParent, POINT pt, HWND hWndCoordsRelativeTo=nullptr);
};

}//namespace wolf