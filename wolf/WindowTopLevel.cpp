/**
 * Part of WOLF - WinAPI Object Lambda Framework
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#include "WindowTopLevel.h"
#include "Font.h"
using namespace wolf;

WindowTopLevel::SetupTopLevel::SetupTopLevel()
	: resize(false), maximize(false), minimize(true), dropFiles(false), size({300, 200})
{
}


WindowTopLevel::~WindowTopLevel()
{
}

bool WindowTopLevel::_loadIfTemplate(HINSTANCE hInst, SetupTopLevel& setup)
{
	if (!this->WindowParent::_loadIfTemplate(hInst, setup)) {
		return false;
	}

	setup.title     = this->WindowParent::_dialogTemplate.title; // overwrite values with dialog's
	setup.size      = this->WindowParent::_dialogTemplate.size;
	setup.maximize  = (this->WindowParent::_dialogTemplate.style & WS_MAXIMIZEBOX) != 0;
	setup.minimize  = (this->WindowParent::_dialogTemplate.style & WS_MINIMIZEBOX) != 0;
	setup.resize    = (this->WindowParent::_dialogTemplate.style & WS_SIZEBOX) != 0 &&
		(this->WindowParent::_dialogTemplate.style & WS_BORDER) != 0;
	setup.dropFiles = (this->WindowParent::_dialogTemplate.exStyle & WS_EX_ACCEPTFILES) != 0;
	return true;
}

int WindowTopLevel::_loop(SetupTopLevel& setup)
{
	DWORD err = 0;
	if (!setup.accTable.create(&err)) {
		WindowMsgHandler::_errorShout(err, L"WindowTopLevel::_loop", L"CreateAcceleratorTable");
		return -1;
	}

	MSG  msg = { 0 };
	BOOL ret = 0;
	while (IsWindow(this->Window::hWnd()) && (ret = GetMessage(&msg, nullptr, 0, 0)) != 0) {
		if (ret == -1) {
			WindowMsgHandler::_errorShout(GetLastError(), L"WindowTopLevel::_loop", L"GetMessage");
			return -1;
		}
		if ( (setup.accTable.hAccel() &&
				setup.accTable.translate(this->Window::hWnd(), msg)) ||
			IsDialogMessage(this->Window::hWnd(), &msg) )
		{
			continue;
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return static_cast<int>(msg.wParam); // this can be used as program return value
}

DWORD WindowTopLevel::_calcStyle(const SetupTopLevel& setup)
{
	return WS_CAPTION | WS_SYSMENU | WS_CLIPCHILDREN |
		(setup.resize ? WS_SIZEBOX : WS_BORDER) |
		(setup.minimize ? WS_MINIMIZEBOX : 0) |
		(setup.maximize ? WS_MAXIMIZEBOX : 0);
}

DWORD WindowTopLevel::_calcStyleEx(const SetupTopLevel& setup)
{
	return setup.dropFiles ? WS_EX_ACCEPTFILES : 0;
}

bool WindowTopLevel::_compensateBorders(DWORD style, bool hasMenu, SetupTopLevel& setup)
{
	RECT rc = { 0, 0, setup.size.cx, setup.size.cy };
	if (!AdjustWindowRect(&rc, style, hasMenu)) { // compensate different theme window borders
		WindowMsgHandler::_errorShout(GetLastError(), L"WindowTopLevel::_compensateBorders", L"AdjustWindowRect");
		return false;
	}
	setup.size.cx = rc.right - rc.left;
	setup.size.cy = rc.bottom - rc.top;
	return true;
}