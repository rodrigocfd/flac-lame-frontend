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
	virtual ~Menu() = default;
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
};


class MenuInitHandler : public Menu {
private:
	std::function<void()> _callback;
public:
	virtual ~MenuInitHandler() = default;
	template<WNDPROC DefProcT> explicit MenuInitHandler(WindowMsgHandler<DefProcT> *parent);
	void onInitMenuPopup(std::function<void()> callback);
};


class MenuMain final : public MenuInitHandler {
public:
	template<WNDPROC DefProcT> explicit MenuMain(WindowMsgHandler<DefProcT> *parent);
	MenuMain& operator=(const MenuMain& m) = delete;
	MenuMain& operator=(MenuMain&& m) = delete;
private:
	void _createOnce() override;
};


class MenuContext final : public MenuInitHandler {
public:
	template<WNDPROC DefProcT> explicit MenuContext(WindowMsgHandler<DefProcT> *parent);
	MenuContext& operator=(const MenuContext& m) = delete;
	MenuContext& operator=(MenuContext&& m) = delete;
	void showAtPoint(HWND hParent, POINT pt, HWND hWndCoordsRelativeTo = nullptr);
private:
	void _createOnce() override;
};



template<WNDPROC DefProcT>
MenuInitHandler::MenuInitHandler(WindowMsgHandler<DefProcT> *parent)
{
	parent->onMessage(WM_INITMENUPOPUP, [this](WPARAM wp, LPARAM lp)->LRESULT {
		if (this->_hMenu == reinterpret_cast<HMENU>(wp) && this->_callback) {
			this->_callback();
		}
		return 0;
	});
}

template<WNDPROC DefProcT>
MenuMain::MenuMain(WindowMsgHandler<DefProcT> *parent)
	: MenuInitHandler(parent)
{
}

template<WNDPROC DefProcT>
MenuContext::MenuContext(WindowMsgHandler<DefProcT> *parent)
	: MenuInitHandler(parent)
{
}

}//namespace wolf