/**
 * Part of WOLF - WinAPI Object Lambda Framework
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#pragma once
#include "WindowMsgHandler.h"

namespace wolf {

class Menu {
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
	int   size() const;
	Menu  getSubmenu(int pos) const;
	WORD  getCommandId(int index) const;
	Menu& addSeparator();
	Menu& addItem(WORD commandId, const wchar_t *caption);
	Menu& addItem(WORD commandId, const std::wstring& caption);
	Menu& enableItem(WORD commandId, bool doEnable);
	Menu& enableItem(std::initializer_list<WORD> commandIds, bool doEnable);
	Menu  addSubmenu(const wchar_t *caption);
	Menu  addSubmenu(const std::wstring& caption);
protected:
	virtual void _createOnce();
	template<WNDPROC DefProcT> bool _addInitMenuPopup(WindowMsgHandler<DefProcT> *parent, std::function<void()> callback);
};


class MenuMain : public Menu {
public:
	MenuMain() = default;
	MenuMain& operator=(const MenuMain& m) = delete;
	MenuMain& operator=(MenuMain&& m) = delete;
	template<WNDPROC DefProcT> bool onInitMenuPopup(WindowMsgHandler<DefProcT> *parent, std::function<void()> callback);
private:
	void _createOnce() override;
	Menu::_addInitMenuPopup;
};


class MenuContext : public Menu {
public:
	MenuContext() = default;
	MenuContext& operator=(const MenuContext& m) = delete;
	MenuContext& operator=(MenuContext&& m) = delete;
	template<WNDPROC DefProcT> bool onInitMenuPopup(WindowMsgHandler<DefProcT> *parent, std::function<void()> callback);
	void showAtPoint(HWND hParent, POINT pt, HWND hWndCoordsRelativeTo=nullptr);
private:
	void _createOnce() override;
	Menu::_addInitMenuPopup;
};



template<WNDPROC DefProcT>
bool Menu::_addInitMenuPopup(WindowMsgHandler<DefProcT> *parent, std::function<void()> callback)
{
	return parent->onMessage(WM_INITMENUPOPUP, [callback, this](WPARAM wp, LPARAM lp)->LRESULT {
		if (this->_hMenu == reinterpret_cast<HMENU>(wp)) {
			callback();
		}
		return 0;
	});
}

template<WNDPROC DefProcT>
bool MenuMain::onInitMenuPopup(WindowMsgHandler<DefProcT> *parent, std::function<void()> callback)
{
	return this->Menu::_addInitMenuPopup(parent, std::move(callback));
}

template<WNDPROC DefProcT>
bool MenuContext::onInitMenuPopup(WindowMsgHandler<DefProcT> *parent, std::function<void()> callback)
{
	return this->Menu::_addInitMenuPopup(parent, std::move(callback));
}

}//namespace wolf