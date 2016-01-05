/**
 * Part of WOLF - WinAPI Object Lambda Framework
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#pragma once
#include "WindowTopLevel.h"
#include "Menu.h"

namespace wolf {

class WindowMain : public WindowTopLevel  {
private:
	struct SetupMain final : public WindowTopLevel::SetupTopLevel {
		MenuMain menu;
		int      iconId;
		SetupMain(WindowMain *wndMain);
	};

public:
	SetupMain setup;
	virtual ~WindowMain() = 0;
	WindowMain();
	int run(HINSTANCE hInst, int cmdShow);
private:
	ATOM _registerClass(HINSTANCE hInst);
	WindowProc::_registerClass;
	WindowParent::_dialogTemplate;
	WindowTopLevel::SetupTopLevel;
	WindowTopLevel::_loadIfTemplate;
	WindowTopLevel::_loop;
	WindowTopLevel::_calcStyle;
	WindowTopLevel::_calcStyleEx;
	WindowTopLevel::_compensateBorders;
};

}//namespace wolf



#define RUN(MainClass) \
int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE h0, wchar_t *cmdLine, int cmdShow) \
{ \
	int ret = 0; \
	{\
		MainClass wnd; \
		ret = wnd.run(hInst, cmdShow); \
	} \
	_ASSERT(!_CrtDumpMemoryLeaks()); \
	return ret; \
}